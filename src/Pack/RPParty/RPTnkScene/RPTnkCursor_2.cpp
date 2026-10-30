#include <Pack/RPParty/RPTnkScene/RPTnkCursor.h>

#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPPartyCommon/RPCursorTrail.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPTnkScene/RPTnkCamera.h>
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTank.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTankManager.h>

#include <egg/core/eggController.h>
#include <egg/gfxe/eggScreen.h>
#include <cmath>

extern nw4r::math::VEC3 RP_vecZero;
extern nw4r::math::VEC3 RP_vecOne;
extern nw4r::math::VEC2 RP_vec2Zero;
extern RPAxes RP_mtx33Identity;

// The original symbol names this VEC2 normalization routine as a matrix helper.
static inline nw4r::math::VEC2 GetTrailPos(RPCursorTrail* trail) {
    return trail->GetPos(trail->GetLatest());
}

static inline nw4r::math::VEC2 ProjectScreen(const nw4r::math::VEC3& world) {
    nw4r::math::VEC2 out;
    RPTnkCamera::sInstance->ProjectToScreen(&out, &world);
    return out;
}

static inline nw4r::math::VEC2 ProjectTank(RPTnkTank* tank) {
    nw4r::math::VEC3 world = tank->GetPos();
    world.y += 11.0f * tank->GetScale().y;
    nw4r::math::VEC2 out;
    RPTnkCamera::sInstance->ProjectToScreen(&out, &world);
    return out;
}

static inline f32 CursorMax(f32 value, f32 bound) {
    return bound > value ? bound : value;
}

RPTnkCursor::RPTnkCursor() {
    mPos.x = RP_vecZero.x;
    mPos.y = RP_vecZero.y;
    mPos.z = RP_vecZero.z;
    mScale.x = RP_vecOne.x;
    mScale.y = RP_vecOne.y;
    mScale.z = RP_vecOne.z;
    mRot = RP_mtx33Identity;
    mPointerPos.x = RP_vec2Zero.x;
    mPointerPos.y = RP_vec2Zero.y;
    mPlayer = 0;
}

RPTnkCursor::~RPTnkCursor() {}

void RPTnkCursor::UpdatePointerPos() {
    mOutOfScreen = true;
    mpTrail->Update();
    if (mpTrail->GetLatest() != -1) {
        nw4r::math::VEC2 pos = GetTrailPos(mpTrail);
        if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
            f32 x = CursorMax(pos.x, -0.82f);
            mPointerPos.x = 0.82f < x ? 0.82f : x;
            f32 y = CursorMax(pos.y, -0.44f);
            mPointerPos.y = 0.41f < y ? 0.41f : y;
            if (nw4r::math::FAbs(mPointerPos.x) == 0.82f) {
                mOutOfScreen = false;
            }
            if (mPointerPos.y == -0.44f || mPointerPos.y == 0.41f) {
                mOutOfScreen = false;
            }
        } else if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_STD) {
            f32 x = CursorMax(pos.x, -0.82f);
            mPointerPos.x = 0.82f < x ? 0.82f : x;
            f32 y = CursorMax(pos.y, -0.61f);
            mPointerPos.y = 0.56f < y ? 0.56f : y;
            if (nw4r::math::FAbs(mPointerPos.x) == 0.82f) {
                mOutOfScreen = false;
            }
            if (mPointerPos.y == -0.61f || mPointerPos.y == 0.56f) {
                mOutOfScreen = false;
            }
        }
    }
}

#pragma auto_inline off

void RPTnkCursor::UpdateScreenPosAndMode() {
    mScreenPos.x = mPointerPos.x * mScreenHalfWidth;
    mScreenPos.y = -mPointerPos.y * mScreenHalfWidth;
    RPTnkTank* tank = RPTnkTankManager::sInstance->GetPlayerTank(mPlayer);
    bool dead = tank->GetAlive() == 0;
    nw4r::math::VEC3 world = tank->GetPos();
    world.y += 11.0f * tank->GetScale().y;
    nw4r::math::VEC2 pos = ProjectScreen(world);
    nw4r::math::VEC2 delta;
    delta.x = mScreenPos.x - pos.x;
    delta.y = mScreenPos.y - pos.y;
    f32 distance = nw4r::math::VEC2Len(&delta);
    f32 limit = 480.0f;
    if (!dead && distance > limit) {
        nw4r::math::VEC2Normalize(&delta, &delta);
        mScreenPos.x = delta.x * limit + pos.x;
        mScreenPos.y = delta.y * limit + pos.y;
    }
    switch (RPTnkGameManager::sInstance->GetState()) {
    case 2:
    case 3:
        SetDisplayMode(2);
        break;
    case 1:
    case 7:
    case 10:
        SetDisplayMode(1);
        break;
    case 11:
    case 12:
        SetDisplayMode(0);
        break;
    default:
        if (!dead) {
            if (distance >= limit || !mOutOfScreen) {
                SetDisplayMode(2);
            } else {
                SetDisplayMode(1);
            }
        } else if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
            SetDisplayMode(3);
        } else {
            SetDisplayMode(1);
        }
        break;
    }
}

void RPTnkCursor::CalcWorldPos() {
    RPTnkCamera* camera = RPTnkCamera::sInstance;
    nw4r::math::VEC3 dir = camera->GetDir();
    nw4r::math::VEC3 side = camera->GetSide();
    side.x = -side.x;
    side.y = -side.y;
    side.z = -side.z;
    nw4r::math::VEC3 up;
    PSVECCrossProduct(side, dir, up);
    nw4r::math::VEC3 lookPos = camera->GetLookPos();
    UpdatePointerPos();
    UpdateScreenPosAndMode();
    nw4r::math::VEC3 scale = RPGrpRenderer::GetCurrent()->GetView(1)->GetScreen()->GetScale();
    f32 height = RPTnkTankManager::sInstance->GetPlayerTank(0)->GetBodyHeight();
    nw4r::math::VEC2 screen;
    screen.x = mScreenPos.x * scale.x;
    screen.y = mScreenPos.y * scale.y;
    nw4r::math::VEC3 origin;
    origin.x = screen.y * up.x + (screen.x * side.x + lookPos.x);
    origin.y = screen.y * up.y + (screen.x * side.y + lookPos.y);
    origin.z = screen.y * up.z + (screen.x * side.z + lookPos.z);
    nw4r::math::VEC3 end;
    end.x = origin.x + dir.x;
    end.y = origin.y + dir.y;
    end.z = origin.z + dir.z;
    nw4r::math::VEC3 plane(0.0f, height, 0.0f);
    nw4r::math::VEC3 normal(0.0f, 1.0f, 0.0f);
    RPIntersectLinePlane(&mPos, &origin, &end, &plane, &normal);
    mScale.x = 1.2f;
    mScale.y = 1.2f;
    mScale.z = 1.2f;
    RPAxes& rot = mRot;
    rot.x.x = side.x;
    rot.x.y = side.y;
    rot.x.z = side.z;
    rot.y.x = up.x;
    rot.y.y = up.y;
    rot.y.z = up.z;
    rot.z.x = -dir.x;
    rot.z.y = -dir.y;
    rot.z.z = -dir.z;
}

#pragma auto_inline on

void RPTnkCursor::Create() {
    mpTrail = new RPCursorTrail(0, 10);
}

void RPTnkCursor::Reset() {
    mScreenHalfWidth = RPGrpRenderer::GetCurrent()->GetView(1)->GetScreen()->GetWidth() * 0.5f;
    mPointerPos.x = mPlayer == 0 ? -0.4f : 0.4f;
    mPointerPos.y = 0.15f;
    mOutOfScreen = true;
    UpdateScreenPosAndMode();
    mpTrail->SetChannel(mPlayer);
    mpTrail->Reset();
}

// Inlined into the four callers below
inline void RPTnkCursor::UpdateEffects() {
    nw4r::math::VEC3 pos(mScreenPos.x, mScreenPos.y, 0.0f);
    switch (mPlayer) {
    case 0:
        mpTailEffects[0]->setPos(pos);
        mpTailEffects[0]->update();
        mpMainEffects[0]->setPos(pos);
        mpMainEffects[0]->update();
        break;
    case 1:
        mpTailEffects[1]->setPos(pos);
        mpTailEffects[1]->update();
        mpMainEffects[1]->setPos(pos);
        mpMainEffects[1]->update();
        break;
    }
}

void RPTnkCursor::RestartMainEffect() {
    RPSysEffect* effect;
    switch (mPlayer) {
    case 0:
        effect = mpMainEffects[0];
        break;
    case 1:
        effect = mpMainEffects[1];
        break;
    }
    effect->forceKill();
    effect->create();
    UpdateEffects();
}

void RPTnkCursor::RestartTailEffect() {
    RPSysEffect* effect;
    switch (mPlayer) {
    case 0:
        effect = mpTailEffects[0];
        break;
    case 1:
        effect = mpTailEffects[1];
        break;
    }
    effect->forceKill();
    effect->create();
    UpdateEffects();
}

void RPTnkCursor::RestartLineEffect() {
    RPSysEffect* effect;
    switch (mPlayer) {
    case 0:
        effect = mpLineEffects[0];
        break;
    case 1:
        effect = mpLineEffects[1];
        break;
    }
    effect->forceKill();
    effect->create();
    UpdateLineEffect();
}

void RPTnkCursor::SetDisplayMode(s32 mode) {
    RPSysEffect* main;
    RPSysEffect* tail;
    RPSysEffect* line;
    mDisplayMode = mode;
    switch (mPlayer) {
    case 0:
        main = mpMainEffects[0];
        tail = mpTailEffects[0];
        line = mpLineEffects[0];
        break;
    case 1:
        main = mpMainEffects[1];
        tail = mpTailEffects[1];
        line = mpLineEffects[1];
        break;
    }
    switch (mDisplayMode) {
    case 0:
        line->setDisableDraw(true);
        tail->setDisableDraw(true);
        main->setDisableDraw(true);
        break;
    case 1:
        line->setDisableDraw(true);
        tail->setDisableDraw(false);
        main->setDisableDraw(false);
        main->setColor(255, 255, 255, 255);
        break;
    case 2:
        line->setDisableDraw(false);
        tail->setDisableDraw(false);
        main->setDisableDraw(false);
        main->setColor(255, 255, 255, 255);
        break;
    case 3:
        line->setDisableDraw(true);
        tail->setDisableDraw(true);
        main->setDisableDraw(false);
        main->setColor(255, 255, 255, 128);
        break;
    }
    UpdateEffects();
    UpdateLineEffect();
}

#pragma auto_inline off

void RPTnkCursor::UpdateLineEffect() {
    nw4r::math::VEC3 screen(mScreenPos.x, mScreenPos.y, 0.0f);
    RPTnkTank* tank = RPTnkTankManager::sInstance->GetPlayerTank(mPlayer);
    nw4r::math::VEC2 pos = ProjectTank(tank);
    nw4r::math::VEC3 start(pos.x, pos.y, 0.0f);
    nw4r::math::VEC3 delta;
    delta.x = screen.x - start.x;
    delta.y = screen.y - start.y;
    delta.z = 0.0f;
    f32 distance = PSVECMag(delta);
    if (distance != 0.0f) {
        PSVECNormalize(delta, delta);
    } else {
        delta.x = RP_vecEx.x;
        delta.y = RP_vecEx.y;
        delta.z = RP_vecEx.z;
    }
    RPAxes rot;
    rot.x.x = delta.x;
    rot.x.y = delta.y;
    rot.x.z = delta.z;
    rot.z.x = RP_vecEz.x;
    rot.z.y = RP_vecEz.y;
    rot.z.z = RP_vecEz.z;
    PSVECCrossProduct(rot.z, rot.x, rot.y);
    nw4r::math::MTX34 mtx;
    f32 length = distance / 100.0f;
    RPDucMakeMtx(&mtx, rot, RP_vecOne, RP_vecZero);
    nw4r::math::VEC3 effectScale(length, length, length);
    nw4r::math::VEC3 effectPos(start.x, start.y, start.z);
    switch (mPlayer) {
    case 0:
        mpLineEffects[0]->setLocalScale(effectScale);
        mpLineEffects[0]->setMtx(mtx);
        mpLineEffects[0]->setPos(effectPos);
        mpLineEffects[0]->update();
        break;
    case 1:
        mpLineEffects[1]->setLocalScale(effectScale);
        mpLineEffects[1]->setMtx(mtx);
        mpLineEffects[1]->setPos(effectPos);
        mpLineEffects[1]->update();
        break;
    }
}

#pragma auto_inline on

void RPTnkCursor::Calculate() {
    EGG::CoreControllerMgr::instance()->getNthController(static_cast<u16>(mPlayer));
    CalcWorldPos();
    UpdateEffects();
}


void RPTnkCursor::Draw() {}

void RPTnkCursor::CreateParts(s32 player) {
    mPlayer = player;
    switch (player) {
    case 0:
        mpMainEffects[0] = new RPSysEffect("Tnk_cursor_1p_main", 4, 0);
        mpTailEffects[0] = new RPSysEffect("Tnk_cursor_1p_tail", 4, 0);
        mpLineEffects[0] = new RPSysEffect("Tnk_cursor_1p_line", 4, 0);
        break;
    case 1:
        mpMainEffects[1] = new RPSysEffect("Tnk_cursor_2p_main", 4, 0);
        mpTailEffects[1] = new RPSysEffect("Tnk_cursor_2p_tail", 4, 0);
        mpLineEffects[1] = new RPSysEffect("Tnk_cursor_2p_line", 4, 0);
        break;
    }
}
