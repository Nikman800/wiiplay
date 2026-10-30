// Laser Hockey music: game BGM, track unmuting as the clock runs down, jingles on request.
// Unit 0x8022074C-0x80220D1C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyBgmCtrl.h>

#include <Pack/RPAudio.h>

RPHkyBgmCtrl::RPHkyBgmCtrl() {
    Init();
}

void RPHkyBgmCtrl::Stop() {
    RPSndAudioMgr::instance()->stopBgm(10);
    RPSndAudioMgr::instance()->stopStream(0);
    Init();
}

void RPHkyBgmCtrl::StartGameBgm() {
    RPSndAudioMgr::instance()->startBgm(0x44);

    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
    handle.SetVolume(1.0f, 10);
    handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_OFF);
    handle.SetTrackMute(0x7001, nw4r::snd::MUTE_RELEASE);
}

void RPHkyBgmCtrl::Request(s32 request) {
    mRequest = request;
    mRequested = true;
}

void RPHkyBgmCtrl::Update(s32 secondsLeft) {
    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());
    handle.ReadVariable(0, &mSeqVar);

    if (mPrevSeqVar != mSeqVar) {
        mPrevSeqVar = mSeqVar;
    }

    if (secondsLeft < 31) {
        handle.SetTrackMute(0x5001, nw4r::snd::MUTE_OFF);
    }

    if (mRequest == REQUEST_3) {
        handle.SetTrackMute(0x6001, nw4r::snd::MUTE_OFF);
    }

    if (mRequested) {
        switch (mRequest) {
        case REQUEST_NONE:
            break;
        case REQUEST_1:
            RPSndAudioMgr::instance()->stopStream(0);
            RPSndAudioMgr::instance()->startStream(0x41);
            break;
        case REQUEST_2:
            RPSndAudioMgr::instance()->stopStream(0);
            RPSndAudioMgr::instance()->startStream(0x42);
            break;
        case REQUEST_TIME_UP:
            handle.Stop(0);
            RPSndAudioMgr::instance()->startSystemSe(0x12);
            RPSndAudioMgr::instance()->startBgm(0x45);
            break;
        }

        mRequested = false;
    }
}
