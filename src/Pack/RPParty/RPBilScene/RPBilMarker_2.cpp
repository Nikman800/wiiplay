// Billiards next-ball marker (RPBilNextBallMarker, singleton RPBilNextBallMarker::sInstance): two
// bad_next_ball models over the lowest ball still on the table, one per camera view.
// Unit 0x802CBE88-0x802CC57C. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilMarker.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPBilScene/RPBilBall.h>
#include <Pack/RPParty/RPBilScene/RPBilBallMgr.h>
#include <Pack/RPParty/RPBilScene/RPBilCamera.h>
#include <Pack/RPParty/RPBilScene/RPBilTable.h>
#include <Pack/RPParty/RPPartyCommon/RPPhysSphere.h>

#include <egg/math.h>

#include <nw4r/g3d.h>

RPBilNextBallMarker* RPBilNextBallMarker::sInstance;

namespace {

// Element-wise copy (all loads before the stores)
inline void SetVec(EGG::Vector3f& rDst, const nw4r::math::VEC3& rSrc) {
    rDst.set(rSrc.x, rSrc.y, rSrc.z);
}

} // namespace

inline RPBilNextBallMarker::RPBilNextBallMarker() {
    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}

void RPBilNextBallMarker::CreateInstance() {
    sInstance = new RPBilNextBallMarker();
}

void RPBilNextBallMarker::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilNextBallMarker::~RPBilNextBallMarker() {}

void RPBilNextBallMarker::Load() {
    for (int i = 0; i < MODEL_NUM; i++) {
        mpModels[i] = RPGrpModel::Construct(5, "bad_next_ball", 4, 0, 0);
        mpModels[i]->SetDrawScene(1);

        mpModels[i]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
        mpModels[i]->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Chr, 0, 5,
                                                      "bad_next_ball");
        mpModels[i]->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
        mpModels[i]->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 1.0f);

        mpModels[i]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
        mpModels[i]->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Clr, 0, 5,
                                                      "bad_next_ball");
        mpModels[i]->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
        mpModels[i]->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 1.0f);
    }
}

void RPBilNextBallMarker::Reset() {
    mVisible = false;
}

void RPBilNextBallMarker::Exit() {}

void RPBilNextBallMarker::Calculate() {
    if (mVisible) {
        RPBilBallMgr* pBallMgr = RPBilBallMgr::sInstance;
        mBallPos = pBallMgr->GetBall(pBallMgr->GetLowestBall())->GetPos();
        CalcModelMtx();
    }
}

void RPBilNextBallMarker::CalcModelMtx() {
    RPGrpCamera* pCamera0 = RPBilCameraMgr::sInstance->GetCamera();
    RPGrpCamera* pCamera1 = RPBilCameraMgr::sInstance->GetAuxCamera();
    f32 height = 2.0f * mBallRadius + RPBilTable::GetHeight();

    for (int i = 0; i < MODEL_NUM; i++) {
        RPGrpCamera* pCamera = i == 0 ? pCamera0 : pCamera1;

        nw4r::math::MTX34 mtx;
        EGG::Vector3f dir;
        EGG::Vector3f up;
        EGG::Vector3f side;

        SetVec(dir, pCamera->GetPosition() - pCamera->GetTargetRef());
        f32 dist = dir.normalise();
        SetVec(up, pCamera->GetUp());
        side = up.cross(dir);

        mtx._00 = side.x;
        mtx._10 = side.y;
        mtx._20 = side.z;
        mtx._01 = up.x;
        mtx._11 = up.y;
        mtx._21 = up.z;
        mtx._02 = dir.x;
        mtx._12 = dir.y;
        mtx._22 = dir.z;

        EGG::Vector3f pos(mBallPos);
        pos += up * mBallRadius * 1.1f;
        pos.y = height;
        mtx._03 = pos.x;
        mtx._13 = pos.y;
        mtx._23 = pos.z;

        nw4r::g3d::ScnObj* pScnObj = mpModels[i]->GetScnObj();
        if (pScnObj != NULL) {
            pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
        }

        f32 scale = 0.8f + 0.7f * (dist / (RPBilCamera::GetBaseDistance() -
                                           RPBilCamera::GetZoomDistance()));
        if (scale < 0.8f) {
            scale = 0.8f;
        } else if (scale > 1.5f) {
            scale = 1.5f;
        }
        mpModels[i]->SetBaseScale(scale);

        mpModels[i]->UpdateFrame();
        mpModels[i]->Calc();
        mpModels[i]->Entry();
    }
}

bool RPBilNextBallMarker::Show() {
    RPBilBallMgr* pBallMgr = RPBilBallMgr::sInstance;
    s32 ball = pBallMgr->GetLowestBall();
    if (ball != -1) {
        RPBilBall* pBall = pBallMgr->GetBall(ball);
        mVisible = true;
        mBallPos = pBall->GetPos();
        mBallRadius = pBall->GetPhysSphere()->GetRadius();
        return true;
    }

    return false;
}

void RPBilNextBallMarker::UserDraw() {
    if (!mVisible) {
        return;
    }

    if (RPGrpRenderer::GetDrawStep() != 1) {
        return;
    }

    u8 view = RPGrpRenderer::GetCurrentViewNo();
    if (view == 1) {
        mpModels[0]->SetVisible(false);
        mpModels[1]->SetVisible(true);
    } else if (view == 4) {
        mpModels[0]->SetVisible(true);
        mpModels[1]->SetVisible(false);
    }
}
