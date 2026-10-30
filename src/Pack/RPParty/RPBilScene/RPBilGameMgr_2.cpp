// Billiards BGM state (RPBilMusicState): start, per-frame sequence-variable sync, state change
// and the track mutes per state. Unit 0x802C65A0-0x802C6CB8. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilGameMgr.h>

#include <Pack/RPAudio.h>
#include <Pack/RPSystem.h>

// Sounds started by Calculate on a state change, per [state][sequence variable]
const u32 RPBilMusicState::sTrackTable[6][4] = {
    {0x57, 0x58, 0x59, 0x5A}, {0x57, 0x58, 0x59, 0x5A}, {0x1C, 0x1C, 0x1C, 0x1C},
    {0x53, 0x54, 0x55, 0x56}, {0x52, 0x51, 0x50, 0x4F}, {0x1C, 0x1C, 0x1C, 0x1C},
};

void RPBilMusicState::Start() {
    RPSndAudioMgr::instance()->startBgm(0x4E);

    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
    handle.SetVolume(1.0f, 10);
    handle.SetTrackMute(0x86, nw4r::snd::MUTE_OFF);
    handle.SetTrackMute(0x358, nw4r::snd::MUTE_RELEASE);

    unk10 = false;
    unk11 = false;
    mPrevSeqVar = -1;
    mSeqVar = -1;
    mState = 6;
    mNextState = 6;
}

void RPBilMusicState::Calculate() {
    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
    handle.ReadVariable(0, &mSeqVar);

    bool varChanged;
    if (mPrevSeqVar != mSeqVar) {
        mPrevSeqVar = mSeqVar;
        varChanged = true;
    } else {
        varChanged = false;
    }

    if (unk10) {
        unk11 = true;
        unk10 = false;
    }

    if (varChanged && unk11) {
        if ((mState == 4 && mNextState == 0) || (mState == 4 && mNextState == 1) ||
            (mState == 2 && mNextState == 3) || (mState == 3 && mNextState == 4)) {
            if (RPSndAudioMgr::instance()->isDisableFanfare() != true) {
                mSubHandle.Stop(0);
                RPSndAudioMgr::instance()->startSound(&mSubHandle,
                                                      sTrackTable[mNextState][mSeqVar]);
            }
        }

        ApplyState();
        unk11 = false;
    }
}

void RPBilMusicState::SetState(s32 state) {
    mState = mNextState;
    mNextState = state;
    unk10 = true;
}

void RPBilMusicState::ApplyState() {
    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());

    switch (mNextState) {
    case 0:
    case 1:
        handle.SetVolume(1.0f, 10);
        handle.SetTrackMute(0x6, nw4r::snd::MUTE_OFF);
        handle.SetTrackMute(0x358, nw4r::snd::MUTE_NO_STOP);
        break;
    case 2:
        handle.SetVolume(0.5f, 10);
        handle.SetTrackMute(0x6, nw4r::snd::MUTE_OFF);
        handle.SetTrackMute(0x358, nw4r::snd::MUTE_NO_STOP);
        break;
    case 3:
        handle.SetVolume(0.5f, 10);
        handle.SetTrackMute(0xE, nw4r::snd::MUTE_OFF);
        handle.SetTrackMute(0x350, nw4r::snd::MUTE_NO_STOP);
        break;
    case 4:
        handle.SetVolume(1.0f, 10);
        handle.SetTrackMute(0x1E, nw4r::snd::MUTE_OFF);
        handle.SetTrackMute(0x340, nw4r::snd::MUTE_NO_STOP);
        break;
    case 5:
        mSubHandle.Stop(0);
        handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_OFF);
        RPSndAudioMgr::instance()->startBgm(0x5B);
        RPSndAudioMgr::instance()->startStream(0x4D);
        handle.SetVolume(1.0f, 10);
        break;
    case 6:
    default:
        handle.SetVolume(1.0f, 10);
        handle.SetTrackMute(0x6, nw4r::snd::MUTE_OFF);
        handle.SetTrackMute(0x358, nw4r::snd::MUTE_NO_STOP);
        break;
    }
}
