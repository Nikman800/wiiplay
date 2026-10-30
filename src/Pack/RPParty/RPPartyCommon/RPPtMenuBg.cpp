// Party menu background (RPPtMenuBg): PT_menu_BG_model and the optional TV model.
// Unit 0x801DCF54-0x801DD8E8. Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPPtMenuBg.h>

#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPSystem/RPPartySystemData.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysSaveDataMgr.h>

#include <egg/gfxe/eggScreen.h>

extern "C" {
s32 Menu_slotToGameId(s32 slot);
s32 Menu_gameIdToSlot(s32 id);

}

f32 RPPtMenuBg::sTvRateIdle = 0.3f;
f32 RPPtMenuBg::sTvRateMove = 0.3f;

RPPtMenuBg* RPPtMenuBg::sInstance;

void RPPtMenuBg::CreateInstance(u8 viewNo, u8 sceneNo, bool withTv) {
    sInstance = new RPPtMenuBg(viewNo, sceneNo, withTv);
}

RPPtMenuBg::~RPPtMenuBg() {}

void RPPtMenuBg::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPPtMenuBg::RPPtMenuBg(u8 viewNo, u8 sceneNo, bool withTv)
    : mViewNo(viewNo), mSceneNo(sceneNo), mWithTv(withTv) {
    mpModel = NULL;

    mpCamera = new RPGrpCamera;
    mpCamera->SetCameraType(RPGrpCamera::ECameraType_LookAt);

    RPGrpScreen* pScreen = new RPGrpScreen;
    pScreen->SetCanvasMode(EGG::Screen::CANVAS_CC);
    pScreen->SetNearZ(20.0f);
    RPGrpRenderer::GetCurrent()->CreateView3D(mViewNo, mpCamera, pScreen);
    RPGrpRenderer::GetCurrent()->GetView(mViewNo)->SetDrawScene(mSceneNo);
}

void RPPtMenuBg::Load(EGG::Archive* pArchive) {
    EGG::Archive* pCommon = RPSysResourceManager::instance()->GetStaticCommonArchive();
    RPGrpModelResManager* pManager = RPGrpModelResManager::GetCurrent();

    RPGrpHandle bgFile = pManager->CreateData(RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pCommon, "G3D/PT_menu_BG_model.brres"), 0);
    mpBgModel = RPGrpModel::Construct(bgFile, "PT_menu_BG_model", 1, 0, 0);
    mpBgModel->SetDrawScene(mSceneNo);
    mpBgModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    mpBgModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
    mpBgModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 1);

    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_STD) {
        mpBgModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Chr, 0, bgFile,
                                                         "PT_menu_BG_4_3");
    } else {
        mpBgModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Chr, 0, bgFile,
                                                         "PT_menu_BG_16_9");
    }
    mpBgModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Clr, 0, bgFile,
                                                     "PT_menu_BG_model");
    mpBgModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_TexSrt, 0, bgFile,
                                                     "PT_menu_BG_model");

    if (mWithTv) {
        RPGrpHandle tvFile = pManager->CreateData(RPGrpModelResManager::Type_ResFile,
            RPSysResourceManager::GetFileFromArchive(pCommon, "G3D/PT_menu_TV_model.brres"),
            0);
        mpTvModel = RPGrpModel::Construct(tvFile, "PT_menu_TV_model", 1, 0, 0);
        mpTvModel->SetDrawScene(mSceneNo);
        mpTvModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
        mpTvModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
        mpTvModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Chr, 0, tvFile,
                                                         "PT_menu_TV_model");
        mpTvModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Clr, 0, tvFile,
                                                         "PT_menu_TV_model");
    }
}

void RPPtMenuBg::Reset(s32 mode) {
    mpCamera->SetPosition(nw4r::math::VEC3(0.0f, 0.0f, 65.0f));
    mpCamera->SetTarget(nw4r::math::VEC3(0.0f, 0.0f, 0.0f));
    mpCamera->SetUp(nw4r::math::VEC3(0.0f, 1.0f, 0.0f));
    mPos = EGG::Vector3f::zero;

    mpBgModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    mpBgModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
    mpBgModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);

    if (mWithTv) {
        mMode = mode;

        RPPartySystemData data = RPSysSaveDataMgr::instance()->getPartySystemData();
        f32 lockedFrame = 8.0f;
        for (s32 i = 0; i < 9; i++) {
            if (!data.isGameOpen(Menu_gameIdToSlot(i))) {
                lockedFrame = i - 1.0f;
                break;
            }
        }
        mpTvModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, lockedFrame);
        mpTvModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 0.0f);

        mTvFrame = 0.0f;
        if (mMode != 9) {
            mTvFrame = 40.0f * Menu_slotToGameId(mMode);
        }
        mpTvModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, mTvFrame);
        unk6C = sTvRateIdle;
        mpTvModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, unk6C);
    }
}

void RPPtMenuBg::Calculate() {
    mpCamera->CalcMatrix();

    EGG::Matrix34f mtx;
    mtx.makeIdentity();
    mtx._03 = mPos.x;
    mtx._13 = mPos.y;
    mtx._23 = mPos.z;
    mpBgModel->GetModelEx()->setLocalMtx(&mtx);
    mpBgModel->SetBaseScale(1.0f);
    mpBgModel->UpdateFrame();
    mpBgModel->Calc();
    mpBgModel->Entry();

    if (mWithTv) {
        CalcTvModel();
    }
}

void RPPtMenuBg::CalcTvModel() {
    f32 frame = mpTvModel->GetModelAnm()->GetFrame(RPGrpModelAnm::Anm_Chr, 0);

    if (mMode != 9) {
        f32 target = 40.0f * Menu_slotToGameId(mMode);
        f32 wrapped;
        if ((frame <= target && frame >= target - 1.01f * unk6C) ||
            (frame <= (wrapped = 360.0f + target) && frame >= wrapped - 1.01f * unk6C)) {
            unk6C = 0.0f;
            mpTvModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Chr, 0, target);
        } else {
            unk6C = sTvRateMove;
        }
    } else {
        unk6C = sTvRateIdle;
    }
    mpTvModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, unk6C);

    EGG::Matrix34f mtx;
    mtx.makeIdentity();
    mtx._03 = mPos.x;
    mtx._13 = mPos.y;
    mtx._23 = mPos.z;
    mpTvModel->GetModelEx()->setLocalMtx(&mtx);
    mpTvModel->SetBaseScale(1.0f);
    mpTvModel->UpdateFrame();
    mpTvModel->Calc();
    mpTvModel->Entry();
}

void RPPtMenuBg::SetMode(s32 mode) {
    mMode = mode;
}
