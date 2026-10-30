// Pose Mii BGM control (RPBomBgm, RPBomGameManager::mpBgm): track selection, muting and the
// jingle handle. The first source file of the old RPBomScreenFx range (its own __sinit):
// 0x802DCEB0-0x802DDCEC. Functions in reverse address order.
#include <Pack/RPParty/RPBomScene/RPBomSound.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <nw4r/snd.h>

static nw4r::snd::SoundHandle sJingleHandle;

namespace {

// RPSndAudioMgr::startBgm with a u32 ID: startSound(SoundHandle*, u32) (slot 0xB4), where the
// shared inline takes unsigned int (slot 0xB8)
inline bool StartBgm(RPSndAudioMgr* pMgr, u32 id) {
    if (pMgr->isDisableFanfare()) {
        return false;
    }

    if (pMgr->getBgmHandle().IsAttachedSound() && pMgr->getBgmHandle().GetId() == id) {
        return false;
    }

    pMgr->getBgmHandle().Stop(0);
    bool success = pMgr->startSound(&pMgr->getBgmHandle(), id);

    if (RPSysProjectLocal::instance()->getFrameRate() == RPSysProjectLocal::EFrameRate_50Hz &&
        pMgr->getSoundArchive().GetSoundType(id) == nw4r::snd::SOUND_TYPE_SEQ &&
        pMgr->getBgmHandle().IsAttachedSound() && (pMgr->getSoundArchive().GetSoundUserParam(id) & 0x80000000)) {
        nw4r::snd::SeqSoundHandle handle(&pMgr->getBgmHandle());
        handle.SetTempoRatio(50.0f / 60.0f);
    }

    return success;
}

} // namespace

RPBomBgm::RPBomBgm()
    : mTrack(11),
      mChanged(false),
      mSeqVar(0),
      unk8(0),
      mLoopWait(true),
      unkC(0),
      mTrackMask(1) {}

void RPBomBgm::Stop() {
    RPSndAudioMgr::instance()->stopBgm(10);
    mTrack = 11;
    mChanged = false;
    mSeqVar = 0;
    unk8 = 0;
    mLoopWait = true;
    unkC = 0;
    mTrackMask = 1;
}

void RPBomBgm::RequestTrack(s32 track) {
    if (mTrack == track) {
        return;
    }
    mTrack = track;
    mChanged = true;
    if (track == 0) {
        mTrackMask = 1;
        StartBgm(RPSndAudioMgr::instance(), 0x21);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = false;
    }
}

void RPBomBgm::ApplyTrack() {
    switch (mTrack) {
    case 0: {
        StartBgm(RPSndAudioMgr::instance(), 0x21);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = false;
        break;
    }
    case 1:
        break;
    case 3: {
        StartBgm(RPSndAudioMgr::instance(), 0x25);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = false;
        break;
    }
    case 4: {
        StartBgm(RPSndAudioMgr::instance(), 0x1D);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = true;
        break;
    }
    case 5: {
        StartBgm(RPSndAudioMgr::instance(), 0x1E);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = true;
        break;
    }
    case 6: {
        StartBgm(RPSndAudioMgr::instance(), 0x1F);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = true;
        break;
    }
    case 7: {
        StartBgm(RPSndAudioMgr::instance(), 0x20);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = true;
        break;
    }
    case 8:
    case 9: {
        StartBgm(RPSndAudioMgr::instance(), 0x22);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = true;
        break;
    }
    case 10: {
        StartBgm(RPSndAudioMgr::instance(), 0x26);
        {
            nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
            handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
            handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
        }
        mLoopWait = true;
        break;
    }
    case 2:
    case 11:
    default:
        Stop();
        mLoopWait = true;
        break;
    }
}

void RPBomBgm::Update(s32 activeBubbles, s32 level) {
    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
    if (mTrack == 9) {
        mTrackMask = 1;
    } else if (mTrack == 8) {
        mTrackMask = 1;
    } else if (mTrack == 1) {
        mTrackMask = 3;
    } else {
        mTrackMask = 0x7F;
    }
    handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
    handle.SetTrackMute(~mTrackMask, nw4r::snd::MUTE_NO_STOP);

    if (!mLoopWait) {
        nw4r::snd::detail::SeqSound::ReadGlobalVariable(0, &mSeqVar);
        if (mSeqVar < 0) {
            mSeqVar = 0;
        }
        if (mSeqVar == 4) {
            if (mTrack == 5 || mTrack == 9) {
                if (mTrack != 9) {
                    mTrack = 9;
                    mChanged = true;
                }
            } else if (mTrack == 4 || mTrack == 8 || mTrack == 0) {
                if (mTrack != 8) {
                    mTrack = 8;
                    mChanged = true;
                }
            }
            mLoopWait = true;
            ApplyTrack();
            mChanged = false;
        }
    } else if (mChanged) {
        ApplyTrack();
        mChanged = false;
    }
}

BOOL RPBomBgm::HoldJingle(u32 id) {
    return RPSndAudioMgr::instance()->getPlayer().HoldSound(&sJingleHandle, id);
}
