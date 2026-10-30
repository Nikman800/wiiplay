// Fishing sound helpers: the fish sound actor's size-dependent sounds, the ambient and global
// sound handles, and RPFshRodSnd::HoldSplash.
// Unit 0x802126DC-0x80212DF8 (dtk/game_units.tsv). Functions in reverse address order, so
// HoldSplash (0x802126DC) is defined last.
#include <Pack/RPParty/RPFshScene/RPFshSound.h>

#include <Pack/RPAudio.h>

static nw4r::snd::SoundHandle sAmbientHandle;
static nw4r::snd::SoundHandle sGlobalHandle;

void RPFshStartAmbientSound() {
    sAmbientHandle.Stop(0);
    RPSndAudioMgr::instance()->startSound(&sAmbientHandle, 0x28U);
}

bool RPFshStopAmbientSound() {
    if (sAmbientHandle.IsAttachedSound()) {
        sAmbientHandle.Stop(0);
        return true;
    }

    return false;
}

s32 RPFshGetBgmFadeFrames(u32 id) {
    if (id == 0x68) {
        return 2;
    }

    return id == 0x71 ? 10 : 0;
}

bool RPFshHoldGlobalSound(u32 id) {
    return RPSndAudioMgr::instance()->getPlayer().HoldSound(&sGlobalHandle, id);
}

void RPFshFishSnd::SetSizeClass(u32 isRare, u32 isSuka, f32 size) {
    if (isRare == 1) {
        mSizeClass = 4;
    } else if (isSuka == 1) {
        mSizeClass = 0;
    } else if (size > 0.7f) {
        mSizeClass = 3;
    } else if (size < 0.3f) {
        mSizeClass = 1;
    } else {
        mSizeClass = 2;
    }
}

bool RPFshFishSnd::StartCatch(u32 remoteFlag) {
    switch (mSizeClass) {
    case 0:
        startSoundWithRemotePlayer(0x1DU, remoteFlag, 0);
        return startSound(0x18U) != NULL;
    case 1:
    default:
        startSoundWithRemotePlayer(0x19U, remoteFlag, 0);
        return startSound(0x14U) != NULL;
    case 2:
        startSoundWithRemotePlayer(0x1AU, remoteFlag, 0);
        return startSound(0x15U) != NULL;
    case 3:
        startSoundWithRemotePlayer(0x1BU, remoteFlag, 0);
        return startSound(0x16U) != NULL;
    case 4:
        startSoundWithRemotePlayer(0x1CU, remoteFlag, 0);
        return startSound(0x17U) != NULL;
    }
}

bool RPFshFishSnd::HoldStruggle(u32 remoteFlag) {
    switch (mSizeClass) {
    case 0:
    case 1:
    default:
        holdSoundWithRemotePlayer(0x10U, remoteFlag, 0);
        return holdSound(0xCU) != NULL;
    case 2:
        holdSoundWithRemotePlayer(0x11U, remoteFlag, 0);
        return holdSound(0xDU) != NULL;
    case 3:
        holdSoundWithRemotePlayer(0x12U, remoteFlag, 0);
        return holdSound(0xEU) != NULL;
    case 4:
        holdSoundWithRemotePlayer(0x13U, remoteFlag, 0);
        return holdSound(0xFU) != NULL;
    }
}

bool RPFshFishSnd::StartBite(u32 remoteFlag) {
    switch (mSizeClass) {
    case 0:
    case 1:
    default:
        return startSoundWithRemotePlayer(0x7U, remoteFlag, 0) != NULL;
    case 2:
        return startSoundWithRemotePlayer(0x8U, remoteFlag, 0) != NULL;
    case 3:
        return startSoundWithRemotePlayer(0x9U, remoteFlag, 0) != NULL;
    case 4:
        return startSoundWithRemotePlayer(0xAU, remoteFlag, 0) != NULL;
    }
}

bool RPFshRodSnd::HoldSplash(f32 bend) {
    if (bend < 1.0f) {
        return false;
    }

    nw4r::snd::SoundHandle* pHandle = holdSound(6U);
    if (pHandle == NULL) {
        return false;
    }

    if (bend >= 10.0f) {
        pHandle->SetVolume(1.0f, 0);
        pHandle->SetPitch(1.0f);
    } else {
        pHandle->SetVolume(0.1f * bend, 0);
        pHandle->SetPitch(0.05f * bend + 1.0f);
    }

    return true;
}
