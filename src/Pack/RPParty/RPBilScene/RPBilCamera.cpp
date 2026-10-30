#include <Pack/RPParty/RPBilScene/RPBilCamera.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>

#include <egg/core/eggHeap.h>

RPBilCameraMgr* RPBilCameraMgr::sInstance;

inline RPBilCameraMgr::RPBilCameraMgr() {
    mpCamera = new RPBilCamera();
    mpAuxCamera = new RPBilAuxCamera();
    for (int i = 0; i < POCKET_CAM_NUM; i++) {
        mpPocketCams[i] = new RPBilPocketCam(i);
    }
}

void RPBilCameraMgr::CreateInstance() {
    sInstance = new RPBilCameraMgr();
}

void RPBilCameraMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilCameraMgr::~RPBilCameraMgr() {}

void RPBilCameraMgr::Reset() {
    mpCamera->Reset();
    mpAuxCamera->Reset();
    for (int i = 0; i < POCKET_CAM_NUM; i++) {
        mpPocketCams[i]->Reset();
    }

    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    const nw4r::math::MTX34& rMtx = mpCamera->GetViewMatrix();
    if (pAudioMgr->getCameraCount() > 0) {
        pAudioMgr->setListenerMatrix(0, rMtx);
    }
}

void RPBilCameraMgr::Calculate() {
    mpCamera->Calculate();
    mpAuxCamera->Calculate();
    for (int i = 0; i < POCKET_CAM_NUM; i++) {
        mpPocketCams[i]->Calculate();
    }

    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    const nw4r::math::MTX34& rMtx = mpCamera->GetViewMatrix();
    if (pAudioMgr->getCameraCount() > 0) {
        pAudioMgr->setListenerMatrix(0, rMtx);
    }
}
