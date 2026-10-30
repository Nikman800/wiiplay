#include <Pack/RPParty/RPWlyScene/RPWlyLevelTable.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameConfig.h>
#include <Pack/RPParty/RPWlyScene/RPWlySound.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <egg/gfxe/eggScreen.h>

#include <cstring>

/******************************************************************************
 *
 * RPWlySoundActor / RPWlyTimerSe (0x8028D5B8..0x8028DEF0)
 * A separate source file from the level table: it has its own .sdata2 pool (1.0, 0.8333, 0.0,
 * then the zeroed 127.0 of TRPSndObject<4>'s weak copies), and the level table's pool repeats 0.0.
 *
 ******************************************************************************/

s32 RPWlySoundActor::sFieldIndex;

bool RPWlyTimerSe::sRoundSoundActive;

const u32 RPWlyTimerSe::sTrackMasks[10] = {0x000, 0x200, 0x200, 0x201, 0x203,
                                     0x207, 0x20F, 0x121F, 0x121F, 0x019};

const u32 RPWlySoundActor::sVoiceLabels[4][10] = {
    {0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x28, 0x29, 0x2A, 0x2B, 0x2C},
    {0x1E, 0x1F, 0x20, 0x21, 0x22, 0x1E, 0x1F, 0x20, 0x21, 0x22},
    {0x14, 0x15, 0x16, 0x17, 0x18, 0x14, 0x15, 0x16, 0x17, 0x18},
    {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E},
};

const u32 RPWlySoundActor::sVoiceIds[4][10] = {
    {0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x2D, 0x2E, 0x2F, 0x30, 0x31},
    {0x23, 0x24, 0x25, 0x26, 0x27, 0x23, 0x24, 0x25, 0x26, 0x27},
    {0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x19, 0x1A, 0x1B, 0x1C, 0x1D},
    {0x0F, 0x10, 0x11, 0x12, 0x13, 0x0F, 0x10, 0x11, 0x12, 0x13},
};

bool RPWlySoundActor::PlayVoice(s32 kind, s32 player) {
    if (kind >= 4) {
        if (player == 1) {
            startSoundWithRemotePlayer(9u, 1, 0);
        } else {
            startSoundWithRemotePlayer(9u, 2, 0);
        }

        return startSound(8u) != NULL;
    }

    if (kind == 1) {
        mpTimerSe->ResetAndPlayFound();
    }

    if (player == 1) {
        startSoundWithRemotePlayer(sVoiceIds[kind][sFieldIndex], 1, 0);
    } else {
        startSoundWithRemotePlayer(sVoiceIds[kind][sFieldIndex], 2, 0);
    }

    return startSound(sVoiceLabels[kind][sFieldIndex]) != NULL;
}

inline void RPWlyTimerSe::ClearState() {
    sRoundSoundActive = false;
    mLastSeconds = -1;
    mPrevSeqVar = -1;
    mSeqVar = -1;
    mMaskIdx = 0;
    mSeconds = 30;
    mFadeSecond = 10;
}

RPWlyTimerSe::RPWlyTimerSe() {
    mOnVolume = 1.0f;
    mOffVolume = 0.0f;
    ClearState();
}

// The original's PlayVoice is in another source file and cannot inline this
#pragma auto_inline off
void RPWlyTimerSe::ResetAndPlayFound() {
    RPSndAudioMgr::instance()->stopStream(0);
    RPSndAudioMgr::instance()->stopBgm(0);
    ClearState();
    RPSndAudioMgr::instance()->startSystemSe(0x79);
}
#pragma auto_inline reset

void RPWlyTimerSe::StartRoundSound() {
    sRoundSoundActive = false;
    mLastSeconds = -1;
    mPrevSeqVar = -1;
    mSeqVar = -1;
    RPSndAudioMgr::instance()->stopStream(0);

    if (mSeconds >= 6) {
        if (mSeconds % 2 == 0) {
            RPSndAudioMgr::instance()->startBgm(0x7Au);
        } else {
            RPSndAudioMgr::instance()->startBgm(0x7Au);
        }

        nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());

        if (mSeconds <= 15) {
            mFadeSecond = 10;
        } else if (mSeconds <= 25 && mSeconds > 15) {
            mFadeSecond = mSeconds - 5;
        } else {
            mFadeSecond = 20;
        }

        if (mSeconds > 10) {
            handle.SetVolume(0.0f, 0);
        } else {
            handle.SetVolume(1.0f, 0);
        }

        mMaskIdx = mSeconds <= 10 && mSeconds >= 5;
        handle.SetTrackVolume(sTrackMasks[mMaskIdx], mOnVolume);
        handle.SetTrackVolume(~sTrackMasks[mMaskIdx], mOffVolume);
        sRoundSoundActive = true;
    }
}

void RPWlyTimerSe::Update(s32 seconds) {
    mSeconds = seconds;
    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
    handle.ReadVariable(0, &mSeqVar);

    bool changed;
    if (mPrevSeqVar != mSeqVar) {
        mPrevSeqVar = mSeqVar;
        changed = true;
    } else {
        changed = false;
    }

    if (mSeqVar == 1 || mSeqVar == 3) {
        changed = false;
    }

    if (mLastSeconds != seconds) {
        mLastSeconds = seconds;

        if (seconds == mFadeSecond) {
            handle.SetVolume(1.0f, 180);
        }

        if (seconds < 6) {
            mMaskIdx = 9;
        }

        if (sRoundSoundActive) {
            if (seconds >= 6) {
                mMaskIdx = 9 - (seconds - 5) / 2;
                if (mMaskIdx < 0) {
                    mMaskIdx = 0;
                }
            }
        } else if (seconds == 0) {
            RPSndAudioMgr::instance()->stopBgm(0);
        }
    }

    if (changed) {
        if (seconds < 6 && sRoundSoundActive) {
            int frames;
            switch (seconds) {
            case 5:
            default:
                frames = 240;
                break;
            case 4:
                frames = 180;
                break;
            case 3:
                frames = 120;
                break;
            case 2:
                frames = 60;
                break;
            case 1:
                frames = 30;
                break;
            case 0:
                frames = 0;
                break;
            }

            RPSndAudioMgr::instance()->stopBgm(frames);
            sRoundSoundActive = false;
            return;
        }

        handle.SetTrackVolume(sTrackMasks[mMaskIdx], mOnVolume);
        handle.SetTrackVolume(~sTrackMasks[mMaskIdx], mOffVolume);
    }
}

void RPWlyTimerSe::Reset() {
    RPSndAudioMgr::instance()->stopStream(0);
    RPSndAudioMgr::instance()->stopBgm(0);
    ClearState();
}
