// Fishing camera holder (RPFshCameraMgr): runs the camera and feeds its view matrix to the 3D
// sound listener.
// Unit 0x8020AE5C-0x8020B0F0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshCamera.h>

#include <Pack/RPAudio.h>

RPFshCameraMgr* RPFshCameraMgr::sInstance;

void RPFshCameraMgr::CreateInstance() {
    sInstance = new RPFshCameraMgr();
}

void RPFshCameraMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshCameraMgr::~RPFshCameraMgr() {}

void RPFshCameraMgr::Reset() {
    mpCamera->Reset();

    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    const nw4r::math::MTX34& rMtx = mpCamera->GetViewMatrix();
    if (pAudioMgr->getCameraCount() > 0) {
        pAudioMgr->setListenerMatrix(0, rMtx);
    }
}

void RPFshCameraMgr::Calculate() {
    mpCamera->Calculate();

    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    const nw4r::math::MTX34& rMtx = mpCamera->GetViewMatrix();
    if (pAudioMgr->getCameraCount() > 0) {
        pAudioMgr->setListenerMatrix(0, rMtx);
    }
}
