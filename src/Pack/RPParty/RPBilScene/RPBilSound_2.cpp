// Billiards 2D sound actor (RPBilSoundActor): camera tick and pocket voice.
// Unit 0x802CABDC-0x802CACF0. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilSound.h>

bool RPBilSoundActor::PlayCameraTick(f32 delta) {
    if (delta < 0.0f) {
        delta = -delta;
    }

    if (delta < 0.00005f) {
        return false;
    }

    f32 interval = 30.0f + -14000.0f * delta;
    if (interval > 30.0f) {
        interval = 30.0f;
    } else if (interval < 2.0f) {
        interval = 2.0f;
    }

    nw4r::snd::SoundHandle* pHandle;
    if (mTickCount == 1) {
        pHandle = startSound(0x12);
    } else if (mTickCount > (u16)interval) {
        mTickCount = 0;
    }

    mTickCount++;
    return pHandle != NULL;
}

bool RPBilSoundActor::StartPocketSound(u32 id) {
    return startSound(id, 3) != NULL;
}
