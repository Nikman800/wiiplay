// Table Tennis rally music: rally BGM, tempo from the rally count, extra tracks faded in and out.
// Unit 0x802352C8-0x80235A54 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpBgmCtrl.h>

#include <Pack/RPAudio.h>

RPPnpBgmCtrl::RPPnpBgmCtrl() {
    mRallyMod10 = 0;
    mRally = 0;
    mFadeStep = 0;
    mTrackMask = 0xCB8F;
    mFadeTrackMask = 0x8B80;
    mLastFlag = 0;
    mFading = false;
    mFadeVolume = 1.0f;
    mPrevSeqVar = -1;
    mSeqVar = -1;
    mTempo = 1.0f;
    mCount = 0;
    mStarted = false;
}

void RPPnpBgmCtrl::StartRallyBgm(BOOL multiPlayer) {
    if (multiPlayer) {
        RPSndAudioMgr::instance()->startBgm(0x6C);
    } else {
        RPSndAudioMgr::instance()->startBgm(0x6B);
    }

    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
    handle.SetVolume(1.0f, 0);
    handle.SetTempoRatio(1.0f);
    handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_OFF);
    handle.SetTrackMute(0x8F80, nw4r::snd::MUTE_RELEASE);

    mRallyMod10 = 0;
    mRally = 0;
    mFadeStep = 0;
    mLastFlag = 0;
    mFading = false;
    mFadeVolume = 1.0f;
    mTempo = 1.0f;
    mCount = 0;
    mStarted = true;
}

void RPPnpBgmCtrl::UpdateRally(f32 ballSpeed, s32 rally) {
    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());

    if (mStarted) {
        if (rally == 0) {
            mTempo = 0.96f;
            handle.SetTempoRatio(0.96f);
        } else if (mRally != rally) {
            mRally = rally;
            mRallyMod10 = rally % 10;

            if (rally < 100) {
                mTempo = 0.3968f * ballSpeed + 0.61f;
            } else if (rally % 100 == 0) {
                mTempo = 0.97f;
            } else {
                mTempo += 0.0015f;
            }

            if (mTempo < 0.96f) {
                mTempo = 0.96f;
            } else if (mTempo > 1.14f) {
                mTempo = 1.14f;
            }

            handle.SetTempoRatio(mTempo);
        }

        if (rally < 4) {
            handle.SetTrackMute(~mTrackMask, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask | 0x70, nw4r::snd::MUTE_OFF);
            handle.SetTrackVolume(mFadeTrackMask, 0.0f);
            return;
        }
    } else {
        handle.SetTempoRatio(mTempo);
        handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_OFF);
        handle.SetTrackVolume(0xFFFF, 1.0f);
    }

    UpdateTracks(mRallyMod10 == 0);
}

void RPPnpBgmCtrl::SetCount(u32 pointState, u32 count) {
    if (mCount != count) {
        mCount = count;
    }

    UpdateTracks(pointState);
}

void RPPnpBgmCtrl::UpdateTracks(u32 flag) {
    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
    handle.ReadVariable(0, &mSeqVar);

    bool varChanged;
    if (mPrevSeqVar != mSeqVar) {
        mPrevSeqVar = mSeqVar;
        varChanged = true;
    } else {
        varChanged = false;
    }

    bool flagChanged = false;
    if (mLastFlag != flag) {
        flagChanged = true;
        mLastFlag = flag;
    }

    if (flag && flagChanged) {
        if (mCount < 11) {
            mFading = true;
        }

        handle.SetTrackMute(~mTrackMask, nw4r::snd::MUTE_NO_STOP);
        handle.SetTrackMute(mTrackMask | 0xC00, nw4r::snd::MUTE_OFF);
        handle.SetTrackVolume(mFadeTrackMask, 1.0f);
        return;
    }

    if (mFading) {
        if (mFadeStep > 2) {
            mFadeVolume -= 0.02f;
            if (mFadeVolume < 0.02f) {
                mFadeStep = 0;
                mFadeVolume = 1.0f;
                mFading = false;
            } else {
                handle.SetTrackMute(~mTrackMask, nw4r::snd::MUTE_NO_STOP);
                handle.SetTrackMute(mTrackMask | 0x70, nw4r::snd::MUTE_OFF);
                handle.SetTrackVolume(mFadeTrackMask, mFadeVolume);
            }
        }

        if (varChanged && mSeqVar == 0) {
            mFadeStep++;
        }
    }
}

void RPPnpBgmCtrl::Stop() {
    RPSndAudioMgr::instance()->stopBgm(10);

    mRallyMod10 = 0;
    mRally = 0;
    mFadeStep = 0;
    mLastFlag = 0;
    mFading = false;
    mFadeVolume = 1.0f;
    mPrevSeqVar = -1;
    mSeqVar = -1;
    mTempo = 1.0f;
    mCount = 0;
    mStarted = false;
}
