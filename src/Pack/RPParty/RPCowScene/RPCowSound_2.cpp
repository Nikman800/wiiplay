// Charge! sound actors: unit 0x802A94F8-0x802AA9F8 (dtk/game_units.tsv; the old RPCowSound_3
// range, the RPCowObjSound destructor thunk, was merged in). Functions in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowSound.h>

inline nw4r::snd::SoundHandle* RPCowObjSound::StartSoundBase(u32 id, u32 handle) {
    if (this->mIsSilenced) {
        return NULL;
    }
    RPSndAudioMgr::instance()->getPlayer().IsAvailable();
    nw4r::snd::SoundHandle* pHandle = NULL;
    if (!BOOL_0x7C) {
        pHandle = FindFishSoundHandle(this->mSoundHandles, this->mHandleCount, id);
    }
    if (pHandle == NULL) {
        pHandle = FindFreeFishSoundHandle(this->mSoundHandles, this->mHandleCount);
    }
    if (pHandle == NULL) {
        pHandle = FindPriorityFishSoundHandle(this->mSoundHandles, this->mHandleCount, id);
    }
    if (pHandle != NULL) {
        detailStartSound(pHandle, id, handle);
        return pHandle;
    }
    return NULL;
}

inline nw4r::snd::SoundHandle* RPCowObjSound::HoldSoundBase(u32 id, u32 handle) {
    nw4r::snd::SoundHandle* pHandle =
        FindFishSoundHandle(this->mSoundHandles, this->mHandleCount, id);
    if (pHandle == NULL) {
        if (this->mIsSilenced) {
            return NULL;
        }
        pHandle = FindFreeFishSoundHandle(this->mSoundHandles, this->mHandleCount);
    }
    if (pHandle != NULL) {
        detailHoldSound(pHandle, id, handle);
        return pHandle;
    }
    return NULL;
}

// Run-step sound per speed bucket, and per sound mode (RPCowPlayerSound::HoldRunStepSound)
static const u32 sRunStepSoundIds[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
static const u32 sRunStepSoundIdsByMode[4][6] = {
    {0x23, 0x24, 0x25, 0x26, 0x27, 0x28},
    {0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E},
    {0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E},
    {0x2F, 0x30, 0x31, 0x32, 0x33, 0x34},
};

bool RPCowPlayerSound::HoldRunStepSound(s32 type, f32 speed) {
    s32 idx;
    if (speed < 0.05f) {
        idx = 0;
    } else if (speed < 0.2f) {
        idx = 1;
    } else if (speed < 0.4f) {
        idx = 2;
    } else if (speed < 0.6f) {
        idx = 3;
    } else if (speed < 0.8f) {
        idx = 4;
    } else if (speed >= 0.8f) {
        idx = 5;
    } else {
        return false;
    }

    nw4r::snd::SoundHandle* pHandle;
    switch (type) {
    case 0:
        pHandle = holdSound(sRunStepSoundIds[idx]);
        break;
    case 1:
        pHandle = holdSound(sRunStepSoundIdsByMode[mMode][idx]);
        break;
    default:
        return false;
    }
    return pHandle != NULL;
}

bool RPCowPlayerSound::HoldSpeedSound(f32 speed) {
    if (speed < 0.35f) {
        return false;
    }

    unk90 = true;
    nw4r::snd::SoundHandle* pHandle;
    switch (mMode) {
    case 0:
    default:
        pHandle = holdSound(0x3B);
        break;
    case 2:
        pHandle = holdSound(0x3C);
        break;
    case 3:
        pHandle = holdSound(0x3D);
        break;
    }

    if (unk94 < 0.0f) {
        unk94 = 0.3f + speed;
        if (unk94 > 1.0f) {
            unk94 = 1.0f;
        } else if (unk94 < 0.3f) {
            unk94 = 0.3f;
        }
    }

    if (pHandle != NULL) {
        pHandle->SetVolume(unk94, 0);
    }
    return pHandle != NULL;
}

bool RPCowPlayerSound::PlayHit(u32 id, f32 speed) {
    nw4r::snd::SoundHandle* pHandle = startSound(id);
    if (pHandle == NULL) {
        return false;
    }

    // Unused, but their literal-pool entries put 0.3 and 1.0 ahead of 0.15 in .sdata2.
    f32 minVolume = 0.3f;
    f32 maxVolume = 1.0f;
    f32 volume = 0.15f;
    if (volume > 1.0f) {
        volume = 1.0f;
    } else if (volume < 0.65f) {
        volume = 0.65f;
    }
    pHandle->SetVolume(volume, 0);

    if (id == 0x4B) {
        pHandle->SetPan(0.0f);
        if (speed > 0.5f) {
            nw4r::snd::SoundHandle* pLeft = NULL;
            nw4r::snd::SoundHandle* pRight = NULL;
            switch (mMode) {
            case 0:
            default:
                startSound(0xA);
                break;
            case 2:
                pLeft = startSoundWithRemotePlayer(0xAU, 1, 0);
                pRight = startSoundWithRemotePlayer(0xBU, 2, 1);
                break;
            case 3:
                pLeft = startSoundWithRemotePlayer(0xAU, 1, 0);
                pRight = startSoundWithRemotePlayer(0xBU, 2, 1);
                break;
            }
            if (pLeft != NULL) {
                pLeft->SetPan(-2.0f);
            }
            if (pRight != NULL) {
                pRight->SetPan(2.0f);
            }
        }
    } else if (id == 0x4D && speed > 0.5f) {
        switch (mMode) {
        case 0:
        default:
            startSound(8);
            break;
        case 2:
            startSound(8);
            break;
        case 3:
            startSound(9);
            break;
        }
    }
    return true;
}

nw4r::snd::SoundHandle* RPCowObjSound::startSound(u32 id, u32 /* handle */) {
    nw4r::snd::SoundHandle* pHandle;
    switch (mMode) {
    case 0:
    default:
        pHandle = StartSoundBase(id, 0);
        break;
    case 1: {
        pHandle = StartSoundBase(id, 0);
        if (pHandle != NULL) {
            pHandle->SetPan(-2.0f);
        }
        nw4r::snd::SoundHandle* pRight = StartSoundBase(id, 1);
        if (pRight != NULL) {
            pRight->SetPan(2.0f);
        }
        break;
    }
    case 2:
        pHandle = StartSoundBase(id, 0);
        if (pHandle != NULL) {
            pHandle->SetPan(-2.0f);
        }
        break;
    case 3:
        pHandle = StartSoundBase(id, 1);
        if (pHandle != NULL) {
            pHandle->SetPan(2.0f);
        }
        break;
    }
    return pHandle;
}

nw4r::snd::SoundHandle* RPCowObjSound::holdSound(u32 id, u32 /* handle */) {
    nw4r::snd::SoundHandle* pHandle;
    switch (mMode) {
    case 0:
    default:
        pHandle = HoldSoundBase(id, 0);
        break;
    case 1: {
        pHandle = HoldSoundBase(id, 0);
        if (pHandle != NULL) {
            pHandle->SetPan(-2.0f);
        }
        nw4r::snd::SoundHandle* pRight = HoldSoundBase(id, 1);
        if (pRight != NULL) {
            pRight->SetPan(2.0f);
        }
        break;
    }
    case 2:
        pHandle = HoldSoundBase(id, 0);
        if (pHandle != NULL) {
            pHandle->SetPan(-2.0f);
        }
        break;
    case 3:
        pHandle = HoldSoundBase(id, 1);
        if (pHandle != NULL) {
            pHandle->SetPan(2.0f);
        }
        break;
    }
    return pHandle;
}
