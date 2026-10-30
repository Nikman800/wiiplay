// DRAFT (not linked yet): Charge! per-player camera (RPCowCamera), the whole original source file.
// .text 0x8029CF74-0x802A0EF0 (DrawPass .. the @20@ destructor thunk that ends the file),
// .rodata 0x803259D8-0x80325AA0, .data 0x8035CE40-0x8035CF30 (RPCowCamera vtable + 12 state
// vtables), .sdata2 0x804409C0-0x80440AD0, .sbss2 0x804419A0-0x804419A8, no .ctors.
// The unit RPCowCamera_2.cpp must be re-split to start at 0x8029CF74: its current head (camera
// constructor, RPCowGame::OnTimeUp/OnCourseComplete, the game's __sinit) belongs to RPCowGame.cpp,
// which also holds the camera's weak constructor/destructor copies.
// Functions are defined in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowCamera.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathLightMap.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpLightManager.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPParty/RPPartyCommon/RPActorScene.h>
#include <Pack/RPParty/RPPartyCommon/RPCollisionShape.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPPathFollower.h>
#include <Pack/RPParty/RPPartyCommon/RPResFile.h>
#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>

#include <egg/gfxe.h>
#include <nw4r/math.h>

extern "C" bool RP_isWithinRange(f32 value, f32 center, f32 range);
extern "C" void RP_approachStep(f32* pValue, f32* pVel, f32 target, f32 rate);
extern "C" void RP_accelerateToward(f32* pValue, f32* pVel, f32 target, f32 accel,
                                    f32 maxSpeed);
extern "C" const EGG::Matrix34f RP_eggMtxIdentity;

namespace {

// RPActor's index (0x1A) is protected; the player of a camera is the one with its index
inline RPCowPlayer* GetPlayer(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowPlayer*>(pGame->GetSlotActor(7))[idx];
}

// The collision shape holds the camera placement: position (y height, z distance), rotation
// (y yaw) and their velocities (RPCollisionShape::mVec40/mVec4C)
inline EGG::Vector3f& ShapeVel(RPCollisionShape* pShape) {
    return pShape->GetRot();
}

inline f32& ShapeYaw(RPCollisionShape* pShape) {
    return pShape->GetVec40().y;
}
inline f32& ShapeYawVel(RPCollisionShape* pShape) {
    return pShape->GetVec4C().y;
}

// RPGrpView's flags are protected
struct CowView : public RPGrpView {
    static void ClearSysCalc(RPGrpView* pView) {
        static_cast<CowView*>(pView)->mFlags &= ~EFlag_SysCalc;
    }
    static void SetSysCalc(RPGrpView* pView) {
        static_cast<CowView*>(pView)->mFlags |= EFlag_SysCalc;
    }
};

inline void SetLookAt(RPGrpCamera* pCamera, EGG::Vector3f eye, EGG::Vector3f target) {
    pCamera->SetPosition(eye);
    pCamera->SetTarget(target);
    pCamera->CalcMatrix();
}

// Vertical field of view for the 16:9 screen; 4:3 scales it by 4/3
inline void SetFovyForTVMode(u8 view, f32 fovy) {
    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        RPGrpRenderer::GetCurrent()->GetView(view)->GetScreen()->SetFovy(fovy);
    } else {
        RPGrpRenderer::GetCurrent()->GetView(view)->GetScreen()->SetFovy(fovy * 4.0f / 3.0f);
    }
}

inline void SetScreenRect(EGG::Screen* pScreen, f32 x, f32 y, f32 w, f32 h) {
    pScreen->SetPositionX(x);
    pScreen->SetPositionY(y);
    pScreen->SetSizeX(w);
    pScreen->SetSizeY(h);
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
}

// Defined at the end of the file (inlined late): its color initializer comes after the named
// constants in .sdata2
inline void ResetScreenEffect(const u8& rView);

} // namespace

f32 RPCowCamera::CalcSquareDistance(const EGG::Vector3f& rPos) const {
    return nw4r::math::VEC3DistSq(&rPos, &mpDrawCamera->GetPosition());
}

// Wii Play: kept as the ledger-named C wrapper around the inline member. As a plain member the
// eye and delta temporaries swap stack slots (0x8 / 0x14), so the binary's copy is the
// wrapper's inlined body.
inline bool RPCowCamera::IsInView(const EGG::Vector3f& rPos) const {
    EGG::Vector3f eye(mpDrawCamera->GetPosition().x, mpDrawCamera->GetPosition().y,
                      mpDrawCamera->GetPosition().z);
    EGG::Vector3f delta = rPos - eye;
    return delta.x * unk5C[0] + delta.y * unk5C[1] + delta.z * unk5C[2] > 0.0f &&
           delta.x * unk5C[3] + delta.y * unk5C[4] + delta.z * unk5C[5] > 0.0f;
}

extern "C" bool Cow_Camera_isInView(RPCowCamera* pCamera, const EGG::Vector3f* pPos) {
    return pCamera->IsInView(*pPos);
}

EGG::Vector3f RPCowCamera::GetEyePos() const {
    return EGG::Vector3f(mpDrawCamera->GetPosition().x, mpDrawCamera->GetPosition().y,
                         mpDrawCamera->GetPosition().z);
}

EGG::Vector3f RPCowCamera::GetBackDir() const {
    const nw4r::math::VEC3& f = mpDrawCamera->GetForward();
    const EGG::Vector3f back(-f.x, -f.y, -f.z);
    return EGG::Vector3f(back.x, back.y, back.z);
}

EGG::Vector3f RPCowCamera::ProjectToScreen(const EGG::Vector3f& rPos) const {
    EGG::Vector3f screen;
    mpDrawCamera->GetWorldToScreen(&screen, rPos,
                                   RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen());
    return screen;
}

EGG::Vector3f RPCowCamera::ProjectToScreenClamped(const EGG::Vector3f& rPos, f32 depth) const {
    EGG::Vector3f screen;
    if (depth < sNearDist) {
        mpDrawCamera->GetWorldToScreen(&screen, rPos - GetBackDir() * (depth - sNearDist),
                                       RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen());
    } else {
        mpDrawCamera->GetWorldToScreen(&screen, rPos,
                                       RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen());
    }
    return screen;
}

void RPCowCamera::SetModelVisible(bool visible) {
    mpModel->SetVisible(visible);
}

void RPCowCamera::StateOverview::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    EGG::Vector3f eye;
    EGG::Vector3f target;
    eye = EGG::Vector3f(0.0f, sOverviewHeight, sOverviewDist);
    target = pPlayer->GetPos();
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(target);
    pOwner->mpDrawCamera->CalcMatrix();
}

// Same as SetView, reading the player position each time
inline void RPCowCamera::SetViewFromPlayer(RPCowPlayer* pPlayer) {
    EGG::Vector3f eye;
    eye.x = mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(mpCollision)) +
            pPlayer->GetPos().x;
    eye.y = mpCollision->GetPos().y;
    eye.z = mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(mpCollision)) +
            pPlayer->GetPos().z;
    EGG::Vector3f target;
    target.x = pPlayer->GetPos().x;
    target.y = mTargetY;
    target.z = pPlayer->GetPos().z;
    mpDrawCamera->SetPosition(eye);
    mpDrawCamera->SetTarget(target);
    mpDrawCamera->CalcMatrix();
}

// Places the eye on the orbit around the target (shape y = height, z = distance, rotation y =
// yaw) and looks at the target at mTargetY
inline void RPCowCamera::SetView(const EGG::Vector3f& rTarget) {
    EGG::Vector3f eye;
    eye.x = mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(mpCollision)) + rTarget.x;
    eye.y = mpCollision->GetPos().y;
    eye.z = mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(mpCollision)) + rTarget.z;
    EGG::Vector3f target;
    target.x = rTarget.x;
    target.y = mTargetY;
    target.z = rTarget.z;
    mpDrawCamera->SetPosition(eye);
    mpDrawCamera->SetTarget(target);
    mpDrawCamera->CalcMatrix();
}

void RPCowCamera::StateFollowEase::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                    pPlayer->GetPos().y + sFollowHeight, 0.2f);
    RP_approachStep(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z,
                    -sFollowDist, 0.2f);
    RP_approachStep(&ShapeYaw(pOwner->mpCollision), &ShapeYawVel(pOwner->mpCollision),
                    pPlayer->GetCameraYawVec().y, 0.05f);
    RP_approachStep(&pOwner->mTargetY, &pOwner->mTargetYVel, pPlayer->GetPos().y + 11.3f, 0.2f);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().z;
    EGG::Vector3f target;
    target.x = pPlayer->GetPos().x;
    target.y = pOwner->mTargetY;
    target.z = pPlayer->GetPos().z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(target);
    pOwner->mpDrawCamera->CalcMatrix();
    if (RP_isWithinRange(pOwner->mTargetY, pPlayer->GetPos().y + 11.3f, 0.1f)) {
        pOwner->mpState = &pOwner->mStateFollowSnap;
    }
}

void RPCowCamera::StateFollowSnap::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                    pPlayer->GetPos().y + sFollowHeight, sSnapRate);
    RP_approachStep(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z,
                    -sFollowDist, sSnapRate);
    RP_approachStep(&ShapeYaw(pOwner->mpCollision), &ShapeYawVel(pOwner->mpCollision),
                    pPlayer->GetCameraYawVec().y, 0.05f);
    RP_approachStep(&pOwner->mTargetY, &pOwner->mTargetYVel,
                    pPlayer->GetPos().y + 11.3f, sTargetRate);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().z;
    EGG::Vector3f target;
    target.x = pPlayer->GetPos().x;
    target.y = pOwner->mTargetY;
    target.z = pPlayer->GetPos().z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(target);
    pOwner->mpDrawCamera->CalcMatrix();
}

void RPCowCamera::StateFollowNear::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                    pPlayer->GetPos().y + sFollowHeight, 0.05f);
    RP_approachStep(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z,
                    -sFollowNearDist, 0.05f);
    RP_approachStep(&ShapeYaw(pOwner->mpCollision), &ShapeYawVel(pOwner->mpCollision),
                    pPlayer->GetCameraYawVec().y, 0.05f);
    RP_approachStep(&pOwner->mTargetY, &pOwner->mTargetYVel,
                    pPlayer->GetPos().y + 11.3f, sTargetRate);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().z;
    EGG::Vector3f target;
    target.x = pPlayer->GetPos().x;
    target.y = pOwner->mTargetY;
    target.z = pPlayer->GetPos().z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(target);
    pOwner->mpDrawCamera->CalcMatrix();
}

void RPCowCamera::StateFollow::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* const pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                    pPlayer->GetPos().y + sFollowHeight, 0.05f);
    RP_approachStep(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z,
                    -sFollowDist, 0.05f);
    RP_approachStep(&ShapeYaw(pOwner->mpCollision), &ShapeYawVel(pOwner->mpCollision),
                    pPlayer->GetCameraYawVec().y, 0.05f);
    RP_approachStep(&pOwner->mTargetY, &pOwner->mTargetYVel,
                    pPlayer->GetPos().y + 11.3f, sTargetRate);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().z;
    EGG::Vector3f target;
    target.x = pPlayer->GetPos().x;
    target.y = pOwner->mTargetY;
    target.z = pPlayer->GetPos().z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(target);
    pOwner->mpDrawCamera->CalcMatrix();
    bool done =
        RP_isWithinRange(pOwner->mpCollision->GetPos().y, pPlayer->GetPos().y + sFollowHeight,
                         0.01f) &&
        RP_isWithinRange(pOwner->mpCollision->GetPos().z, -sFollowDist, 0.01f);
    if (done) {
        pOwner->mpState = &pOwner->mStateFollowSnap;
    }
}

void RPCowCamera::StateFinish::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                    pPlayer->GetPos().y + sFollowHeight, 0.05f);
    RP_approachStep(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z,
                    -sResultDist, 0.05f);
    RP_approachStep(&ShapeYaw(pOwner->mpCollision), &ShapeYawVel(pOwner->mpCollision),
                    pPlayer->GetCameraYawVec().y, 0.05f);
    RP_approachStep(&pOwner->mTargetY, &pOwner->mTargetYVel,
                    pPlayer->GetPos().y + 11.3f, sTargetRate);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().z;
    EGG::Vector3f target;
    target.x = pPlayer->GetPos().x;
    target.y = pOwner->mTargetY;
    target.z = pPlayer->GetPos().z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(target);
    pOwner->mpDrawCamera->CalcMatrix();
}

void RPCowCamera::StateResult::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                    pPlayer->GetPos().y + sFollowHeight, 0.01f);
    RP_approachStep(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z,
                    -sResultDist, sSnapRate);
    RP_approachStep(&ShapeYaw(pOwner->mpCollision), &ShapeYawVel(pOwner->mpCollision),
                    pPlayer->GetCameraYawVec().y, 0.05f);
    RP_approachStep(&pOwner->mTargetY, &pOwner->mTargetYVel, 11.3f, 0.01f);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().z;
    EGG::Vector3f target;
    target.x = pPlayer->GetPos().x;
    target.y = pOwner->mTargetY;
    target.z = pPlayer->GetPos().z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(target);
    pOwner->mpDrawCamera->CalcMatrix();
    if (++pOwner->mTimer >= 30) {
        pOwner->mTimer = 0;
        pOwner->mpState = &pOwner->mStateResultOrbit;
    }
}

void RPCowCamera::StateResultOrbit::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    if (pOwner->mpCollision->GetRot().y < 0.0f) {
        RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                        sOrbitHeight, 0.01f);
    } else {
        RP_accelerateToward(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                            sOrbitHeight, sOrbitAccel, 0.01f);
    }
    RP_accelerateToward(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z,
                        -sOrbitDist, sOrbitDistAccel, 0.04f);
    RP_approachStep(&ShapeYaw(pOwner->mpCollision), &ShapeYawVel(pOwner->mpCollision),
                    pPlayer->GetCameraYawVec().y, 0.05f);
    RP_approachStep(&pOwner->mTargetY, &pOwner->mTargetYVel, 11.3f, 0.01f);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            pPlayer->GetPos().z;
    EGG::Vector3f target;
    target.x = pPlayer->GetPos().x;
    target.y = pOwner->mTargetY;
    target.z = pPlayer->GetPos().z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(target);
    pOwner->mpDrawCamera->CalcMatrix();
}

void RPCowCamera::StateIntro::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayers = GetPlayer(pGame, 0);
    RP_approachStep(&pOwner->mBlend, &pOwner->mBlendVel, pOwner->GetUnk1A(), 0.02f);
    EGG::Vector3f target;
    target = pPlayers[0].GetPos() * (1.0f - pOwner->mBlend) +
             pPlayers[1].GetPos() * pOwner->mBlend;
    f32 height = sIntroHeight;
    f32 dist = pGame->GetPlayerNum() <= 1 ? -sIntroDist1P : -sIntroDist2P;
    f32 rate = 0.015f;
    RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                    height, rate);
    RP_approachStep(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z, dist, rate);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            target.x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            target.z;
    EGG::Vector3f look;
    look.x = target.x;
    look.y = pOwner->mTargetY;
    look.z = target.z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(look);
    pOwner->mpDrawCamera->CalcMatrix();
    if (pOwner->mpCollision->GetPos().z >=
        (pGame->GetPlayerNum() <= 1 ? -sIntroEndDist1P : -sIntroEndDist2P)) {
        pOwner->mpState = &pOwner->mStateIntroBlend;
    }
}

void RPCowCamera::StateIntroBlend::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer1 = GetPlayer(pGame, 1);
    RPCowPlayer* pPlayer0 = GetPlayer(pGame, 0);
    RP_approachStep(&pOwner->mBlend, &pOwner->mBlendVel, pOwner->GetUnk1A(), 0.02f);
    EGG::Vector3f target;
    target = pPlayer0->GetPos() * (1.0f - pOwner->mBlend) + pPlayer1->GetPos() * pOwner->mBlend;
    f32 height = sIntroHeight;
    f32 dist = pGame->GetPlayerNum() <= 1 ? -sIntroDist1P : -sIntroDist2P;
    f32 rate = 0.015f;
    RP_approachStep(&pOwner->mpCollision->GetPos().y, &pOwner->mpCollision->GetRot().y,
                    height, rate);
    RP_approachStep(&pOwner->mpCollision->GetPos().z, &pOwner->mpCollision->GetRot().z, dist, rate);
    f32 yaw = (1.0f - pOwner->mBlend) * pPlayer0->GetCameraYawVec().y +
              pOwner->mBlend * pPlayer1->GetCameraYawVec().y;
    if (pOwner->GetUnk1A() == 0) {
        yaw += NW4R_MATH_DEG_TO_RAD(sIntroBlendYawDeg);
    } else if (pOwner->GetUnk1A() == 1) {
        yaw += NW4R_MATH_DEG_TO_RAD(360.0f - sIntroBlendYawDeg);
    }
    RP_accelerateToward(&ShapeYaw(pOwner->mpCollision), &ShapeYawVel(pOwner->mpCollision), yaw,
                        NW4R_MATH_DEG_TO_RAD(sIntroTurnSpeedDeg), 6.981317e-05f);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            target.x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            target.z;
    EGG::Vector3f look;
    look.x = target.x;
    look.y = pOwner->mTargetY;
    look.z = target.z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(look);
    pOwner->mpDrawCamera->CalcMatrix();
}

void RPCowCamera::StateReady::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    EGG::Vector3f target;
    target = pPlayer->GetPos();
    f32 height = pGame->GetPlayerNum() <= 1 ? sReadyHeight1P : sReadyHeight2P;
    f32 dist = pGame->GetPlayerNum() <= 1 ? -sReadyDist1P : -sReadyDist2P;
    RP_accelerateToward(&pOwner->mpCollision->GetPos().y, &ShapeVel(pOwner->mpCollision).y,
                        height, 0.2f, 0.004f);
    RP_accelerateToward(&pOwner->mpCollision->GetPos().z, &ShapeVel(pOwner->mpCollision).z,
                        dist, 2.0f, 0.04f);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            target.x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            target.z;
    EGG::Vector3f look;
    look.x = target.x;
    look.y = pOwner->mTargetY;
    look.z = target.z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(look);
    pOwner->mpDrawCamera->CalcMatrix();
}

void RPCowCamera::StateStart::Calc(RPCowCamera* pOwner) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, pOwner->GetUnk1A());
    EGG::Vector3f target;
    target = pPlayer->GetPos();
    f32 height = sStartHeight;
    f32 dist = -sStartDist;
    RP_accelerateToward(&pOwner->mpCollision->GetPos().y, &ShapeVel(pOwner->mpCollision).y,
                        height, 0.2f, 0.004f);
    RP_accelerateToward(&pOwner->mpCollision->GetPos().z, &ShapeVel(pOwner->mpCollision).z,
                        dist, 2.0f, 0.04f);
    EGG::Vector3f eye;
    eye.x = pOwner->mpCollision->GetPos().z * nw4r::math::SinRad(ShapeYaw(pOwner->mpCollision)) +
            target.x;
    eye.y = pOwner->mpCollision->GetPos().y;
    eye.z = pOwner->mpCollision->GetPos().z * nw4r::math::CosRad(ShapeYaw(pOwner->mpCollision)) +
            target.z;
    EGG::Vector3f look;
    look.x = target.x;
    look.y = pOwner->mTargetY;
    look.z = target.z;
    pOwner->mpDrawCamera->SetPosition(eye);
    pOwner->mpDrawCamera->SetTarget(look);
    pOwner->mpDrawCamera->CalcMatrix();
    if (RP_isWithinRange(pOwner->mpCollision->GetPos().y, height, 0.0001f) &&
        RP_isWithinRange(pOwner->mpCollision->GetPos().z, dist, 0.0001f)) {
        pOwner->mpState = &pOwner->mStateFollowEase;
    }
}

void RPCowCamera::StartIntro() {
    SetupIntro();
    RPGrpModelManager::GetCurrent()->GetLightManager(0)->SetBinary(mpDemoLight);
}

void RPCowCamera::SetupIntro() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, GetUnk1A());
    f32 dist = -sIntroStartDist;
    mpCollision->GetPos().z = dist;
    f32 height = sIntroStartHeight;
    mpCollision->GetPos().y = height;
    ShapeYaw(mpCollision) = pPlayer->GetCameraYawVec().y + sIntroYawDeg * (NW4R_MATH_PI / 180.0f);
    if (pGame->GetPlayerNum() == 2) {
        mBlend = 0.5f;
        mBlendVel = 0.0f;
    } else {
        mBlend = GetUnk1A();
        mBlendVel = 0.0f;
    }
    mTargetY = 11.3f;
    mTargetYVel = 0.0f;
    mpState = &mStateIntro;
}

bool RPCowCamera::IsIntroDone() const {
    RPCowGame* pGame = RPCowGame::GetInstance();
    f32 height = sIntroHeight;
    f32 dist = pGame->GetPlayerNum() <= 1 ? -sIntroDist1P : -sIntroDist2P;
    f32 range = pGame->GetPlayerNum() <= 1 ? sIntroRange1P : sIntroRange2P;
    bool done = false;
    if (RP_isWithinRange(mpCollision->GetPos().y, height, range)) {
        if (RP_isWithinRange(mpCollision->GetPos().z, dist, range)) {
            done = true;
        }
    }
    return done;
}

void RPCowCamera::SetupReady() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, GetUnk1A());
    f32 value = -sReadyStartDist;
    mpCollision->GetPos().z = value;
    value = sReadyStartHeight;
    mpCollision->GetPos().y = value;
    value = 0.0f;
    mpCollision->GetRot().z = value;
    value = 0.0f;
    mpCollision->GetRot().y = value;
    ShapeYaw(mpCollision) = pPlayer->GetCameraYawVec().y + NW4R_MATH_DEG_TO_RAD(sReadyYawDeg);
    mBlend = GetUnk1A();
    mBlendVel = 0.0f;
    mTargetY = 11.3f;
    mTargetYVel = 0.0f;
    mpState = &mStateReady;
    RPGrpModelManager::GetCurrent()->GetLightManager(0)->SetBinary(mpSceneLight);
}

// Unreferenced stand-in (dropped by the link): the original pool has 0.01 and 0.0001 (the
// StateFollow/StateStart ranges) between VF_0x3C's and SetupReady's constants
extern "C" bool Cow_Camera_poolOrderRanges(f32 value, f32 center) {
    return RP_isWithinRange(value, center, 0.01f) && RP_isWithinRange(value, center, 0.0001f);
}

void RPCowCamera::StartStart() {
    mpState = &mStateStart;
}

bool RPCowCamera::IsPullbackDone() {
    return mpCollision->GetPos().z <= -sPullbackDist;
}

void RPCowCamera::SetFollowNearState() {
    mpState = &mStateFollowNear;
}

void RPCowCamera::SetFollowState() {
    mpState = &mStateFollow;
}

void RPCowCamera::StartFinish() {
    mTimer = 0;
    mpState = &mStateFinish;
}

void RPCowCamera::StartResult() {
    mTimer = 0;
    mpState = &mStateResult;
}

void RPCowCamera::VF_0x3C() {
    u8 sceneB;
    if (GetUnk1A() == 0) {
        RPGrpScreen* pScreen = new RPGrpScreen();
        RPActorList::CreateView2D(4, pScreen);
        RPGrpRenderer::GetCurrent()->GetView(4)->SetDrawScene(3);
        CowView::SetSysCalc(RPGrpRenderer::GetCurrent()->GetView(4));
        pScreen = new RPGrpScreen();
        RPActorList::CreateView2D(5, pScreen);
        RPGrpRenderer::GetCurrent()->GetView(5)->SetDrawScene(4);
        CowView::SetSysCalc(RPGrpRenderer::GetCurrent()->GetView(5));
        sceneB = 1;
        mViewA = 0;
        mViewB = 2;
    } else if (GetUnk1A() == 1) {
        mViewA = 1;
        sceneB = 2;
        mViewB = 3;
    } else {
        mViewA = 4;
        mViewB = 4;
        return;
    }
    RPGrpScreen* pScreen = new RPGrpScreen();
    mpDrawCamera = RPActorList::CreateView3D(mViewA, pScreen);
    RPGrpRenderer::GetCurrent()->GetView(mViewA)->SetDrawScene(0);
    CowView::ClearSysCalc(RPGrpRenderer::GetCurrent()->GetView(mViewA));
    pScreen = new RPGrpScreen();
    RPActorList::CreateView2D(mViewB, pScreen);
    RPGrpRenderer::GetCurrent()->GetView(mViewB)->SetDrawScene(sceneB);
    CowView::ClearSysCalc(RPGrpRenderer::GetCurrent()->GetView(mViewB));
}

void RPCowCamera::VF_0x40() {}

inline void SetCameraLightMap(void* pLightMap) {
    RPGrpModelManager::GetCurrent()
        ->GetDrawPathManager(0)
        ->GetDrawPathLightMap()
        ->GetLightTextureManager()
        ->SetBinary(pLightMap);
}

void RPCowCamera::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPGrpRenderer::GetCurrent()->GetView(mViewB)->CreateScreenEffect();
    if (pGame->GetPlayerNum() <= 1) {
        mpDemoLight = RPSysResourceManager::GetFileFromArchive(
            RPSysResourceManager::LoadGameCommonArchive(),
            "Environment/demo_Light1P.plight", NULL);
        mpSceneLight = RPSysResourceManager::GetFileFromArchive(
            RPSysResourceManager::LoadGameCommonArchive(),
            "Environment/scene_Light1P.plight", NULL);
    } else {
        mpDemoLight = RPSysResourceManager::GetFileFromArchive(
            RPSysResourceManager::LoadGameCommonArchive(),
            "Environment/demo_Light2P.plight", NULL);
        mpSceneLight = RPSysResourceManager::GetFileFromArchive(
            RPSysResourceManager::LoadGameCommonArchive(),
            "Environment/scene_Light2P.plight", NULL);
    }
    RPGrpModelManager::GetCurrent()->GetLightManager(0)->SetBinary(mpSceneLight);
    RPSysKokeshiManager::CreateLightTexture(0);
    SetCameraLightMap(RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(),
        "Environment/scene_Lm.plmap", NULL));
    RPGrpModelManager::GetCurrent()
        ->GetDrawPathManager(0)
        ->GetDrawPathLightMap()
        ->GetLightTextureManager()
        ->onFlag2();
    mpCollision = new RPCollisionShape();
    mpModel = RPGrpModel::Construct(3, 2, 0, 0);
    const u16 num = pGame->GetResTable()->CountEntriesOfType(0x70001);
    const u16 idx = GetUnk1A() < num ? GetUnk1A() : 0;
    mpPathObj = new RPPathObject(0.1f, 1.0f,
                                 pGame->GetResTable()->FindNthEntryOfType(0x70001, idx));
}

void RPCowCamera::OnStart() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    ResetScreenEffect(mViewB);
    mpCollision->Reset();
    mpCollision->GetPos() = mpPathObj->GetPos();
    ShapeYaw(mpCollision) = nw4r::math::Atan2Rad(mpPathObj->GetVec20().x, mpPathObj->GetVec20().z);
    mpDrawCamera->SetCameraType(RPGrpCamera::ECameraType_LookAt);
    mpDrawCamera->SetPosition(0.0f, sOverviewHeight, sOverviewDist);
    mpDrawCamera->SetUp(nw4r::math::VEC3(0.0f, 1.0f, 0.0f));
    mpDrawCamera->SetTarget(nw4r::math::VEC3(0.0f, 0.0f, 0.0f));
    mpDrawCamera->CalcMatrix();
    RPGrpRenderer::GetCurrent()->GetView(mViewA)->SetProjType(RPGrpView::EProjType_Persp);
    RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen()->SetScale(
        nw4r::math::VEC3(1.0f, 1.0f, 1.0f));
    CowView::SetSysCalc(RPGrpRenderer::GetCurrent()->GetView(mViewA));
    CowView::SetSysCalc(RPGrpRenderer::GetCurrent()->GetView(mViewB));
    s32 playerNum = pGame->GetPlayerNum();
    if (playerNum == 2) {
        if (GetUnk1A() == 0) {
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(4)->GetScreen(), 0.0f, 0.0f,
                          EGG::Screen::GetSizeXMax(), EGG::Screen::GetSizeYMax());
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(5)->GetScreen(), 0.0f, 0.0f,
                          EGG::Screen::GetSizeXMax(), EGG::Screen::GetSizeYMax());
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen(), 0.0f, 0.0f,
                          EGG::Screen::GetSizeXMax(), (u16)(EGG::Screen::GetSizeYMax() / 2));
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewB)->GetScreen(), 0.0f, 0.0f,
                          EGG::Screen::GetSizeXMax(), (u16)(EGG::Screen::GetSizeYMax() / 2));
        } else if (GetUnk1A() == 1) {
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen(), 0.0f,
                          (s16)(EGG::Screen::GetSizeYMax() / 2), EGG::Screen::GetSizeXMax(),
                          (u16)(EGG::Screen::GetSizeYMax() / 2));
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewB)->GetScreen(), 0.0f,
                          (s16)(EGG::Screen::GetSizeYMax() / 2), EGG::Screen::GetSizeXMax(),
                          (u16)(EGG::Screen::GetSizeYMax() / 2));
        }
    } else if (playerNum == 3) {
        if (GetUnk1A() == 0) {
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen(), 0.0f, 0.0f,
                          (u16)(EGG::Screen::GetSizeXMax() / 2), EGG::Screen::GetSizeYMax());
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewB)->GetScreen(), 0.0f, 0.0f,
                          (u16)(EGG::Screen::GetSizeXMax() / 2), EGG::Screen::GetSizeYMax());
        } else if (GetUnk1A() == 1) {
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen(),
                          (s16)(EGG::Screen::GetSizeXMax() / 2), 0.0f,
                          (u16)(EGG::Screen::GetSizeXMax() / 2), EGG::Screen::GetSizeYMax());
            SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewB)->GetScreen(),
                          (s16)(EGG::Screen::GetSizeXMax() / 2), 0.0f,
                          (u16)(EGG::Screen::GetSizeXMax() / 2), EGG::Screen::GetSizeYMax());
        }
    } else {
        SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(4)->GetScreen(), 0.0f, 0.0f,
                      EGG::Screen::GetSizeXMax(), EGG::Screen::GetSizeYMax());
        SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(5)->GetScreen(), 0.0f, 0.0f,
                      EGG::Screen::GetSizeXMax(), EGG::Screen::GetSizeYMax());
        SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen(), 0.0f, 0.0f,
                      EGG::Screen::GetSizeXMax(), EGG::Screen::GetSizeYMax());
        SetScreenRect(RPGrpRenderer::GetCurrent()->GetView(mViewB)->GetScreen(), 0.0f, 0.0f,
                      EGG::Screen::GetSizeXMax(), EGG::Screen::GetSizeYMax());
    }
    mTimer = 0;
    mpState = NULL;
    unk9C = 0;
}

void RPCowCamera::OnStop() {
    CowView::ClearSysCalc(RPGrpRenderer::GetCurrent()->GetView(mViewA));
    CowView::ClearSysCalc(RPGrpRenderer::GetCurrent()->GetView(mViewB));
}

void RPCowCamera::OnActivate() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowPlayer* pPlayer = GetPlayer(pGame, GetUnk1A());
    GXColor color = RPCommon::GetInstance()->GetScreenFade()->GetColor();
    RPGrpRenderer::GetCurrent()->GetView(mViewB)->GetScreenEffect()->SetColorMultiply(color);
    RPGrpRenderer::GetCurrent()->GetView(mViewB)->GetScreenEffect()->SetBlurAlpha(color.a);
    if (mpState != NULL) {
        mpState->Calc(this);
    }
    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen()->SetFovy(sFovy);
    } else {
        RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen()->SetFovy(4.0f * sFovy / 3.0f);
    }
    RPGrpCamera* pCamera = mpDrawCamera;
    f32 dist = -pCamera->GetDistance();
    f32 tan = RPGrpRenderer::GetCurrent()->GetView(mViewA)->GetScreen()->GetTanFovy();
    // Separate product: the original has no fmadds here.
    f32 prod = dist * tan;
    EGG::Vector3f side;
    const nw4r::math::VEC3 scaled = pCamera->GetRight() * (prod + sViewMargin);
    f32 x, z, y;
    y = scaled.y;
    x = scaled.x;
    z = scaled.z;
    side.set(x, y, z);
    mViewDir = static_cast<const EGG::Vector3f&>(mpDrawCamera->GetTargetRef() -
                                                 mpDrawCamera->GetPosition());
    mEdgeL = mViewDir + side;
    mEdgeR = mViewDir - side;
    unk5C[0] = mEdgeL.z;
    unk5C[1] = 0.0f;
    unk5C[2] = -mEdgeL.x;
    unk5C[3] = -mEdgeR.z;
    unk5C[4] = 0.0f;
    unk5C[5] = mEdgeR.x;
    mNearDistSq = sNearDist * sNearDist;
    mFarDistSq = sFarDist * sFarDist;
    EGG::Matrix34f mtx = RP_eggMtxIdentity;
    mtx(0, 3) = pPlayer->GetPos().x;
    mtx(1, 3) = 0.0f;
    mtx(2, 3) = pPlayer->GetPos().z;
    CalcModel(mpModel, sModelScale, &mtx);
    const s32 idx = GetUnk1A();
    RPSndAudioMgr* const pAudioMgr = RPSndAudioMgr::instance();
    const nw4r::math::MTX34& rMtx = mpDrawCamera->GetViewMatrix();
    if (pAudioMgr->getCameraCount() > idx) {
        pAudioMgr->setListenerMatrix(idx, rMtx);
    }
}

void RPCowCamera::Calculate() {
    EntryModel(mpModel);
}

void RPCowCamera::DrawPass(u8 pass) {}

const f32 RPCowCamera::sModelScale = 1.0f;
const f32 RPCowCamera::sIntroStartHeight = 20.0f;
const f32 RPCowCamera::sIntroHeight = 15.0f;
const f32 RPCowCamera::sIntroStartDist = 200.0f;
const f32 RPCowCamera::sIntroEndDist1P = 140.0f;
const f32 RPCowCamera::sIntroEndDist2P = 120.0f;
const f32 RPCowCamera::sIntroDist1P = 35.0f;
const f32 RPCowCamera::sIntroDist2P = 20.0f;
const f32 RPCowCamera::sIntroRange1P = 4.0f;
const f32 RPCowCamera::sIntroRange2P = 4.0f;
const f32 RPCowCamera::sIntroYawDeg = 180.0f;
const f32 RPCowCamera::sIntroBlendYawDeg = 240.0f;
const f32 RPCowCamera::sIntroTurnSpeedDeg = 1.0f;
const f32 RPCowCamera::sReadyStartHeight = 14.0f;
const f32 RPCowCamera::sReadyStartDist = 20.0f;
const f32 RPCowCamera::sReadyDist2P = 30.0f;
const f32 RPCowCamera::sReadyDist1P = 40.0f;
const f32 RPCowCamera::sPullbackDist = 40.0f;
const f32 RPCowCamera::sStartDist = 80.0f;
const f32 RPCowCamera::sReadyHeight2P = 15.0f;
const f32 RPCowCamera::sReadyHeight1P = 16.0f;
const f32 RPCowCamera::sStartHeight = 20.0f;
const f32 RPCowCamera::sFollowHeight = 20.0f;
const f32 RPCowCamera::sFollowDist = 80.0f;
const f32 RPCowCamera::sSnapRate = 1.0f;
const f32 RPCowCamera::sTargetRate = 1.0f;
const f32 RPCowCamera::sFollowNearDist = 60.0f;
const f32 RPCowCamera::sResultDist = 100.0f;
const f32 RPCowCamera::sOrbitHeight = 80.0f;
const f32 RPCowCamera::sOrbitAccel = 1.0f;
const f32 RPCowCamera::sOrbitDist = 320.0f;
const f32 RPCowCamera::sOrbitDistAccel = 16.0f;
const f32 RPCowCamera::sFovy = 20.0f;
const f32 RPCowCamera::sOverviewHeight = 1000.0f;
const f32 RPCowCamera::sOverviewDist = -1500.0f;
const f32 RPCowCamera::sNearDist = 30.0f;
const f32 RPCowCamera::sFarDist = 2500.0f;
const f32 RPCowCamera::sViewMargin = 100.0f;
const f32 RPCowCamera::sReadyYawDeg = 0;
const f32 RPCowCamera::sUnusedZero = 0;

namespace {

inline void ResetScreenEffect(const u8& rView) {
    RPGrpView::ScreenEffect* pEffect =
        RPGrpRenderer::GetCurrent()->GetView(rView)->GetScreenEffect();
    GXColor white = {0xFF, 0xFF, 0xFF, 0x00};
    pEffect->SetColorMultiply(white);
    RPGrpRenderer::GetCurrent()->GetView(rView)->GetScreenEffect()->SetBlurAlpha(0);
    RPGrpRenderer::GetCurrent()->GetView(rView)->GetScreenEffect()->SetBlurOffset(1.0f);
}

} // namespace
