// Laser Hockey sound helpers: speed-scaled trail sounds of the puck and paddles.
// Unit 0x8022061C-0x8022074C (dtk/game_units.tsv); split from the goal phase by its own .sdata2 pool.
#include <Pack/RPParty/RPHkyScene/RPHkySound.h>

bool RPHkyPuckTrail::StartSoundBySpeed(u32 id, f32 speed) {
    nw4r::snd::SoundHandle* pHandle = startSound(id);
    if (pHandle != NULL) {
        f32 volume = 1.5f * speed;
        if (volume < 0.5f) {
            volume = 0.5f;
        } else if (volume > 1.0f) {
            volume = 1.0f;
        }
        pHandle->SetVolume(volume, 0);
        return true;
    }

    return false;
}

bool RPHkyPaddleTrail::HoldSoundBySpeed(u32 id, f32 speed) {
    if (mPrevSpeed < speed && speed > 50.0f) {
        mSwinging = true;
    } else if (speed < 20.0f) {
        mSwinging = false;
    }

    mPrevSpeed = speed;

    if (mSwinging) {
        return holdSound(id) != NULL;
    }

    return false;
}
