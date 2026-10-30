// Table Tennis sound helper: the ball's hit sounds (per-kind Remote speaker, volume by strength,
// pitch rising over a rally). Unit 0x8022C478-0x8022C6A0 (dtk/game_units.tsv); split from the
// serve phase by its own .sdata2 pool (8043FAB8, after the serve phase's 180.0 padded to 8), like
// Laser Hockey's RPHkySound.cpp.
#include <Pack/RPParty/RPPnpScene/RPPnpSound.h>

u32 RPPnpBallTrail::sHitSoundTable[2][14] = {
    {0x13, 0x14, 0x17, 0x18, 0x19, 0x1A, 0x15, 0x16, 0x2A, 0x23, 0x24, 0x25, 0x26, 0x26},
    {0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x2A, 0x23, 0x24, 0x25, 0x26, 0x26},
};

bool RPPnpBallTrail::StartHitSound(s32 hit, s32 strength, s32 kind, s32 row) {
    if (kind > 14) {
        return false;
    }

    nw4r::snd::SoundHandle* pHandle;
    if (kind == 0 || kind == 2 || kind == 4 || kind == 6) {
        pHandle = startSoundWithRemotePlayer(sHitSoundTable[row][kind], 1, 0);
    } else if (kind == 1 || kind == 3 || kind == 5 || kind == 7) {
        pHandle = startSoundWithRemotePlayer(sHitSoundTable[row][kind], 2, 0);
    } else {
        pHandle = startSound(sHitSoundTable[row][kind]);
    }

    if (pHandle != NULL) {
        if (strength > 0) {
            f32 volume = 1.0f - 0.08f * strength;
            if (volume < 0.1f) {
                volume = 0.1f;
            }
            pHandle->SetVolume(volume, 0);
        } else {
            if (hit < 2) {
                mLastHit = 0;
                mPitch = 1.0f;
            } else if (hit - mLastHit == 1) {
                mPitch *= 1.012f;
                if (mPitch > 1.5f) {
                    mPitch = 1.5f;
                } else if (mPitch < 1.0f) {
                    mPitch = 1.0f;
                }
                mLastHit = hit;
            }
            pHandle->SetPitch(mPitch);
        }

        return true;
    }

    return false;
}
