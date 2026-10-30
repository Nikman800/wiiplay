// Billiards delayed hint controller (RPBilHint, singleton RPBilHint::sInstance): after 180 idle frames the
// hint fades in (20 frames), stays 240 frames and fades out (40 frames).
// Unit 0x802D2558-0x802D2D28. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPBilScene/RPBilHint.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPSystem.h>

RPBilHint* RPBilHint::sInstance;

s32 RPBilHint::sDelayFrames = 180;
s32 RPBilHint::sFadeInFrames = 20;
s32 RPBilHint::sFadeOutFrames = 40;
s32 RPBilHint::sShownFrames = 240;

inline RPBilHintLyt::RPBilHintLyt() {}

inline RPBilHint::RPBilHint() {
    mpStateMachine = new RPStateMachine<RPBilHint>(this, STATE_NUM);

    RPBilStateFuncs<RPBilHint> funcs[STATE_NUM] = {
        {&RPBilHint::IdleEnter, &RPBilHint::IdleCalc},
        {&RPBilHint::FadeInEnter, &RPBilHint::FadeInCalc},
        {&RPBilHint::ShownEnter, &RPBilHint::ShownCalc},
        {&RPBilHint::FadeOutEnter, &RPBilHint::FadeOutCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpLyt = new RPBilHintLyt();
    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);
}

void RPBilHint::CreateInstance() {
    sInstance = new RPBilHint();
}

void RPBilHint::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilHint::~RPBilHint() {}

void RPBilHint::Load() {
    mpLyt->Load(RPSysResourceManager::LoadGameLocalArchive(-1, NULL));
}

void RPBilHint::Reset() {
    mpStateMachine->Reset(STATE_IDLE);
    mRequested = false;
    mIdleTimer = sDelayFrames;
    mpLyt->Reset();
}

void RPBilHint::Calculate() {
    mpStateMachine->Calculate();
    if (mpStateMachine->GetState() != STATE_IDLE) {
        mpLyt->Calculate();
    }
}

void RPBilHint::IdleEnter() {}

void RPBilHint::IdleCalc() {
    if (!mRequested) {
        return;
    }
    if (mpStateMachine->GetTimer() <= mIdleTimer) {
        return;
    }
    mIdleTimer = sDelayFrames;
    mpStateMachine->ChangeState(STATE_FADE_IN);
}

void RPBilHint::FadeInEnter() {}

void RPBilHint::FadeInCalc() {
    s32 alpha = mpStateMachine->GetTimer() * 255 / sFadeInFrames;
    if (alpha < 0) {
        alpha = 0;
    } else if (alpha > 255) {
        alpha = 255;
    }
    mpLyt->SetAlpha(alpha);

    if (mpStateMachine->GetTimer() >= sFadeInFrames) {
        mpStateMachine->ChangeState(STATE_SHOWN);
    }
}

void RPBilHint::ShownEnter() {}

void RPBilHint::ShownCalc() {
    if (mRequested && mpStateMachine->GetTimer() < sShownFrames) {
        return;
    }
    mpStateMachine->ChangeState(STATE_FADE_OUT);
}

void RPBilHint::FadeOutEnter() {}

void RPBilHint::FadeOutCalc() {
    s32 alpha = 255 - mpStateMachine->GetTimer() * 255 / sFadeOutFrames;
    if (alpha < 0) {
        alpha = 0;
    } else if (alpha > 255) {
        alpha = 255;
    }
    mpLyt->SetAlpha(alpha);

    if (mpStateMachine->GetTimer() >= sFadeOutFrames) {
        mpStateMachine->ChangeState(STATE_IDLE);
    }
}

void RPBilHint::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() != 5) {
        return;
    }
    if (RPGrpRenderer::GetDrawStep() != 5) {
        return;
    }
    if (mpStateMachine->GetState() == STATE_IDLE) {
        return;
    }
    mpLyt->Draw();
}

void RPBilHint::Request(bool restart) {
    mRequested = true;
    if (mpStateMachine->GetState() == STATE_IDLE && restart) {
        mpStateMachine->ResetTimer();
    }
}

void RPBilHint::Hide() {
    mRequested = false;
    if (mpStateMachine->IsStateSettled(STATE_SHOWN)) {
        mpStateMachine->ChangeState(STATE_FADE_OUT);
    }
    if (mpStateMachine->IsStateSettled(STATE_IDLE)) {
        mpStateMachine->ResetTimer();
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPBilHint>::RPStateMachine(RPBilHint*, s32);
template void RPStateMachine<RPBilHint>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPBilHint>::Reset(s32);
template void RPStateMachine<RPBilHint>::ChangeState(s32);
template void RPStateMachine<RPBilHint>::Calculate();
template void RPStateMachine<RPBilHint>::CallExit();
template void RPStateMachine<RPBilHint>::CallCalc();
template void RPStateMachine<RPBilHint>::CallEnter();
template bool RPStateMachine<RPBilHint>::IsStateSettled(s32) const;
