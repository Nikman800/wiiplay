// Tanks enemy AI controller (RPTnkAiController): owns the attack and movement AIs.
// Unit 0x80268B64-0x80268DB8 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkAi.h>

RPTnkAiController::RPTnkAiController(RPTnkTank* pTank) : mpTank(pTank) {
    mpAttack = new RPTnkAiAttack(mpTank);
    mpMove = new RPTnkAiMove(mpTank);
}

RPTnkAiController::~RPTnkAiController() {}

void RPTnkAiController::Reset() {
    mMode = 0;
    mpAttack->LoadParams(0);
    mpAttack->ResetAttack();
    mpMove->LoadParams(0);
    mpMove->ResetMovement();
    mFlags = 0;
}

void RPTnkAiController::SetMode(s32 mode) {
    mMode = mode;
    mpAttack->LoadParams(mode);
    mpAttack->ResetAttack();
    mpMove->LoadParams(mode);
    mpMove->ResetMovement();
}

void RPTnkAiController::SetFlags(u32 flags, BOOL set) {
    if (set) {
        mFlags |= flags;
    } else {
        mFlags &= ~flags;
    }
}

void RPTnkAiController::SetMoveDirection(const nw4r::math::VEC3& rDir) {
    mpMove->SetDirection(rDir);
}

void RPTnkAiController::SetAimDirection(const nw4r::math::VEC3& rDir) {
    mpAttack->SetAimDirection(rDir);
}

void RPTnkAiController::UpdateMove(RPAxes* pRot, nw4r::math::VEC3* pVel,
                                   const nw4r::math::VEC3& rPos) {
    mpMove->Update(pRot, pVel, rPos);
}

void RPTnkAiController::UpdateAttack(f32 muzzleDist, RPAxes* pTurret,
                                     const nw4r::math::VEC3& rPos) {
    mpAttack->Update(muzzleDist, pTurret, rPos);
}
