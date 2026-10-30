// Tanks tank engine / tread sound (RPTnkTankSound).
// Unit 0x80268488-0x802688E8 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkSound.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>

#include <cmath>
#include <nw4r/math.h>

static const u32 sTreadSoundIds[12] = {0x18, 0x19, 0x1A, 0x1A, 0x1A, 0x1B,
                                       0x1B, 0x1C, 0x1C, 0x1D, 0x1D, 0x1A};

RPTnkTankSound::~RPTnkTankSound() {}

namespace {
// Keep the player reference in a temporary before forwarding through the sound bases.
// With propagation disabled below, this preserves the original entry scheduling.
struct TankSoundPlayer {
    nw4r::snd::SoundArchivePlayer& player;
    TankSoundPlayer() : player(RPSndAudioMgr::instance()->getPlayer()) {}
    operator nw4r::snd::SoundArchivePlayer&() const {
        nw4r::snd::SoundArchivePlayer& result = player;
        return result;
    }
};
}

#pragma opt_propagation off
RPTnkTankSound::RPTnkTankSound()
    : TRPSndGameObject<0>(TankSoundPlayer(),
                          *static_cast<nw4r::snd::Sound3DManager*>(NULL)) {
    mAccelRate = 1.0f / 14.0f;
    mDecelRate = 1.0f / 45.0f;
    mMaxSpeed = 1.0f;
    mTopStep = mMaxSpeed * 0.3f / 200.0f;
    unk90 = mMaxSpeed / mMaxSpeed;
    mSpeed = 0.0f;
    mTarget = 0.0f;
    mStep = 0.0f;
    mTreadTimer = 0;
    mTreadIndex = 0;
}
#pragma opt_propagation reset

void RPTnkTankSound::UpdateTread(f32 speed, s32) {
    f32 s = ApproachSpeed(speed);
    if (s > 0.01f) {
        f32 interval = -30.0f * s + 25.0f;
        if (interval > 25.0f) {
            interval = 25.0f;
        } else if (interval < 4.0f) {
            interval = 4.0f;
        }

        if (mTreadTimer == 1) {
            startSound(sTreadSoundIds[mTreadIndex]);
        } else if (mTreadTimer > (u16)interval) {
            mTreadTimer = 0;
        }

        mTreadTimer++;
    }
}

f32 RPTnkTankSound::ApproachSpeed(f32 target) {
    if (mSpeed > 0.7 * mMaxSpeed && mSpeed < target) {
        mStep = mTopStep;
    } else if (target != mTarget) {
        if (target > mSpeed) {
            mStep = mAccelRate * (target - mSpeed);
        } else {
            mStep = mDecelRate * (target - mSpeed);
        }
        mTarget = target;
    }

    f32 diff = target - mSpeed;
    if (nw4r::math::FAbs(diff) <= nw4r::math::FAbs(mStep)) {
        mSpeed = target;
        mStep = 0.0f;
    } else {
        mSpeed += mStep;
    }

    if (mSpeed > mMaxSpeed) {
        mSpeed = mMaxSpeed;
    } else if (mSpeed < 0.0f) {
        mSpeed = 0.0f;
    }

    return mSpeed;
}
