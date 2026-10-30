// Billiards table (RPBilTable, singleton RPBilTable::sInstance): table and mask models, table.kcl, pocket
// positions and directions, the aiming guide line and the table's shadow texture.
// Unit 0x802BAB30-0x802BB950. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilTable.h>

#include <Pack/RPParty/RPBilScene/RPBilScene.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathShadowTexture.h>
#include <Pack/RPGraphics/RPGrpShadowTexture.h>
#include <Pack/RPParty/RPPartyCommon/RPJointMtxBuffer.h>
#include <Pack/RPParty/RPPartyCommon/RPKcl.h>
#include <Pack/RPSystem.h>

#include <egg/gfxe.h>
#include <egg/math.h>

u8 RPBilTable::sGuideLineWidth = 0xC0;
f32 RPBilTable::sGuideLineY = -0.3f;
GXColor RPBilTable::sGuideLineColors = {0x00, 0x32, 0x32, 0x80};
GXColor Bil_guideLineColorSub = {0x32, 0x00, 0x64, 0x80};

EGG::Vector3f RPBilTable::sCenter(EGG::Vector3f::zero);

RPBilTable* RPBilTable::sInstance;

f32 RPBilTable::GetHeight() {
    return 0.0f;
}

const EGG::Vector3f& RPBilTable::GetCenter() {
    return sCenter;
}

f32 RPBilTable::GetCushionParam() {
    return 3.57f;
}

EGG::Vector3f RPBilTable::GetPocketDirection(s32 pocket) {
    nw4r::math::VEC3 dirs[6] = {
        nw4r::math::VEC3(-1.0f, 0.0f, -1.0f), nw4r::math::VEC3(0.0f, 0.0f, -1.0f),
        nw4r::math::VEC3(1.0f, 0.0f, -1.0f),  nw4r::math::VEC3(-1.0f, 0.0f, 1.0f),
        nw4r::math::VEC3(0.0f, 0.0f, 1.0f),   nw4r::math::VEC3(1.0f, 0.0f, 1.0f),
    };

    EGG::Vector3f dir(dirs[pocket]);
    dir.normalise();
    return dir;
}

namespace {

inline void SetAxes(EGG::Matrix34f& rMtx, const EGG::Vector3f& rX, const EGG::Vector3f& rY,
                    const EGG::Vector3f& rZ) {
    rMtx.setBase(0, rX);
    rMtx.setBase(1, rY);
    rMtx.setBase(2, rZ);
}

inline void DrawCube(const nw4r::math::MTX34& rMtx, GXColor color) {
    EGG::DrawGX::BeginDrawCube();
    EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_CUBE, rMtx, color);
}

} // namespace

inline RPBilTable::RPBilTable() {
    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
    mpKcl = new RPKcl();
    mpJointMtx = NULL;
}

void RPBilTable::CreateInstance() {
    sInstance = new RPBilTable();
}

void RPBilTable::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilTable::~RPBilTable() {}

void RPBilTable::Load() {
    const char* kclName = "table.kcl";
    mpKcl->SetResource(RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), kclName));

    mpModel = RPGrpModel::Construct(0, "bad_table", 4, 0, 0);
    mpModel->SetDrawScene(1);
    mpMaskModel = RPGrpModel::Construct(0, "bad_mask", 4, 0, 0);
    mpMaskModel->SetDrawScene(1);
    mpMaskModel->SetUnk3(0xF);

    mpJointMtx = new RPJointMtxBuffer(mpModel);

    const nw4r::math::MTX34& rRange = mpJointMtx->GetWorldMtx((u16)mpModel->GetJointIndex("ball0_range"));
    mBallRange.x = rRange._03;
    mBallRange.y = rRange._13;
    mBallRange.z = rRange._23;

    const nw4r::math::MTX34& rCorner = mpJointMtx->GetWorldMtx((u16)mpModel->GetJointIndex("pocket_corner"));
    mPocketCorner.x = rCorner._03;
    mPocketCorner.y = rCorner._13;
    mPocketCorner.z = rCorner._23;

    const nw4r::math::MTX34& rSide = mpJointMtx->GetWorldMtx((u16)mpModel->GetJointIndex("pocket_side"));
    mPocketSide.x = rSide._03;
    mPocketSide.y = rSide._13;
    mPocketSide.z = rSide._23;

    mpShadowTex = new RPGrpShadowTexture(0x100, 1, 0x80);
    mpShadowTex->InitList();
    mpShadowTex->SHORT_0x70 |= 2;
    mpShadowTex->Replace(mpModel, "dummy");
    mpShadowTex->BYTE_0x72 = 0xE1;
    mpShadowTex->FLOAT_0x74 = 1.1f;
    mpShadowTex->VEC_0x78 = nw4r::math::VEC3(0.0f, -1.0f, 0.0f);
    RPGrpModelManager::GetCurrent()
        ->GetModelScene(1)
        ->GetDrawPathManager()
        ->GetDrawPathShadowTexture()
        ->SetTexture(0, mpShadowTex);
    mpShadowTex->ResetList();
}

void RPBilTable::Reset() {
    unk80 = false;
    mGuideLength = 0.0f;
    mShowGuideLine = false;
}

void RPBilTable::Exit() {}

void RPBilTable::Calculate() {
    mpShadowTex->ResetList();
    CalcModelMtx();

    EGG::Matrix34f mtx;
    mtx.setBase(0, mAxisX);
    mtx.setBase(1, mAxisY);
    mtx.setBase(2, mAxisZ);
    mtx(0, 3) = mPos.x;
    mtx(1, 3) = mPos.y;
    mtx(2, 3) = mPos.z;
    mpMaskModel->GetModelEx()->setLocalMtx(&mtx);
    mpMaskModel->SetBaseScale(mScale);
    mpMaskModel->Calc();
    mpMaskModel->Entry();
}

bool RPBilTable::SweepHitsKcl(const EGG::Vector3f* pSegment, f32 radius) {
    RPKclHit hits[1];
    EGG::Vector3f delta = pSegment[1] - pSegment[0];
    s32 stepNum = delta.length() / 0.5f;
    f32 stepNumF = stepNum; // unused; its int->float conversion decides the scratch slots

    for (s32 i = 0; i <= stepNum; i++) {
        EGG::Vector3f pos = pSegment[0] + delta * i / stepNum;

        s32 hitNum;
        if (radius > FLT_EPSILON) {
            hitNum = mpKcl->SearchSphereOctree(hits, 1, pos, radius);
        } else {
            hitNum = mpKcl->CheckPoint(hits, 1, pos);
        }

        if (hitNum > 0) {
            return true;
        }
    }

    return false;
}

void RPBilTable::SetGuideLine(const EGG::Vector3f& rStart, const EGG::Vector3f& rDir) {
    mGuideStart = rStart;
    mGuideStart.y = sGuideLineY;
    mGuideDir.x = rDir.x;
    mGuideDir.z = rDir.z;
    mGuideDir.y = 0.0f;
    mGuideDir.normalise();

    f32 lengthX = 65535.0f;
    f32 lengthZ = 65535.0f;

    if (EGG::Math<f32>::abs(mGuideDir.x) > FLT_EPSILON) {
        if (mGuideDir.x < 0.0f) {
            lengthX = (-mPocketCorner.x - rStart.x) / mGuideDir.x;
        } else {
            lengthX = (mPocketCorner.x - rStart.x) / mGuideDir.x;
        }
    }

    if (EGG::Math<f32>::abs(mGuideDir.z) > FLT_EPSILON) {
        if (mGuideDir.z < 0.0f) {
            lengthZ = (-mPocketCorner.z - rStart.z) / mGuideDir.z;
        } else {
            lengthZ = (mPocketCorner.z - rStart.z) / mGuideDir.z;
        }
    }

    mGuideLength = EGG::Math<f32>::min(lengthX, lengthZ);
    mShowGuideLine = true;
}

void RPBilTable::HideGuideLine() {
    mShowGuideLine = false;
}

bool RPBilTable::IsInsideBallRange(f32 radius, const EGG::Vector3f& rPos) const {
    EGG::Vector3f d = rPos - sCenter;

    if ((mBallRange.x - sCenter.x) - radius - EGG::Math<f32>::abs(d.x) > 0.0f &&
        (mBallRange.z - sCenter.z) - radius - EGG::Math<f32>::abs(d.z) > 0.0f) {
        return true;
    }

    return false;
}

namespace {

struct PocketWords {
    u32 x, y, z;
};

union PocketXY {
    u32 words[2];
};

} // namespace

// The result pointer is the hidden first argument of the public by-value return.
// NONMATCHING-DEBT: as EGG::Vector3f RPBilTable::GetPocketPos(s32) const only pocket 4's copy differs (z is loaded before the x/y stores).
extern "C" void GetPocketPos__10RPBilTableCFl(nw4r::math::VEC3* pOut, RPBilTable* pTable,
                                             s32 pocket) {
    nw4r::math::VEC3 pos = pTable->mPocketCorner;

    if (pocket == 1) {
        pos.x = pTable->mPocketSide.x;
        pos.y = pTable->mPocketSide.y;
        pos.z = -pTable->mPocketSide.z;
        *reinterpret_cast<PocketWords*>(pOut) = *reinterpret_cast<const PocketWords*>(&pos);
        return;
    }

    if (pocket == 4) {
        // Copy x/y together, then read z after the first two stores.
        *reinterpret_cast<PocketXY*>(pOut) = *reinterpret_cast<const PocketXY*>(&pTable->mPocketSide);
        reinterpret_cast<PocketWords*>(pOut)->z =
            reinterpret_cast<const PocketWords*>(&pTable->mPocketSide)->z;
        return;
    }

    if (pocket == 0 || pocket == 2) {
        pos.z = -pos.z;
    }

    if (pocket == 0 || pocket == 3) {
        pos.x = -pos.x;
    }

    *reinterpret_cast<PocketWords*>(pOut) = *reinterpret_cast<const PocketWords*>(&pos);
}

EGG::Vector3f RPBilTable::GetRackOrigin() {
    return EGG::Vector3f(50.0f, 0.0f, 0.0f);
}

void RPBilTable::UserDraw() {
    RPBilScene* pScene =
        static_cast<RPBilScene*>(RPSysSceneMgr::instance()->getCurrentSceneRP());

    if (RPGrpRenderer::GetDrawStep() == 1) {
        if (RPGrpRenderer::GetCurrentViewNo() == 4) {
            mpMaskModel->SetVisible(false);
        } else {
            mpMaskModel->SetVisible(true);
        }
    }

    if ((RPGrpRenderer::GetCurrentViewNo() == 1 || RPGrpRenderer::GetCurrentViewNo() == 4) &&
        RPGrpRenderer::GetDrawStep() == 6 && mShowGuideLine) {
        EGG::Vector3f up(0.0f, 1.0f, 0.0f);
        EGG::Vector3f side = up.cross(mGuideDir);
        f32 width = 5.0f;

        EGG::Matrix34f mtx;
        if (RPGrpRenderer::GetCurrentViewNo() == 4) {
            f32 start = 2.8f;
            SetAxes(mtx, side * width, up, mGuideDir * (mGuideLength - start));
            mtx.setBase(3, mGuideStart + mGuideDir * (0.5f * mGuideLength + 0.5f * start));
        } else {
            SetAxes(mtx, side * width, up, mGuideDir * mGuideLength);
            mtx.setBase(3, mGuideStart + mGuideDir * mGuideLength * 0.5f);
        }

        GXColor color = RPGrpRenderer::GetCurrentViewNo() == 4 ? sGuideLineColors
                                                               : Bil_guideLineColorSub;
        DrawCube(mtx, color);
    }

    if (RPGrpRenderer::GetCurrentViewNo() == 1 && RPGrpRenderer::GetDrawStep() == 3) {
        pScene->GetCapture48()->Capture();
    }

    if (unk80 && RPGrpRenderer::GetCurrentViewNo() == 8 && RPGrpRenderer::GetDrawStep() == 5) {
        RPGrpScreen* pScreen = RPGrpRenderer::GetCurrentScreen();

        EGG::Matrix34f mtx;
        mtx.makeIdentity();
        mtx(0, 0) = pScreen->GetWidth();
        mtx(1, 1) = pScreen->GetHeight();

        EGG::DrawGX::BeginDrawQuad(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_ALWAYS,
                                   EGG::DrawGX::BLEND_NORMAL, true, false);
        pScene->GetCapture48()->GetCapTexture()->load(GX_TEXMAP0);

        GXColor color = {255, 255, 255, sGuideLineWidth};
        EGG::DrawGX::DrawDL(EGG::DrawGX::DL_QUAD_TEXTURE, mtx, color);
    }
}
