#include <Pack/RPAudio.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPAudio/RPSndUtility.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <egg/audio.h>

/**
 * @brief Active sound archive player
 */
nw4r::snd::SoundArchivePlayer* RPSndUtility::spSoundArchivePlayer = NULL;

// Wii Play: RPSndRangeTable is not in Wii Sports.
void RPSndRangeTable::setInfo(const Info* pInfo) {
    mInfo = pInfo;
}

#pragma push
#pragma dont_inline on
u32 RPSndRangeTable::Lookup(s32 x) {
#line 34 "RPSndUtility.cpp"
    RP_ASSERT(mInfo);

    s32 lower = -1;
    for (u32 i = 0; i < mInfo->num; i++) {
        s32 upper = mInfo->entries[i].upper;
        if (upper == -1) {
            break;
        }

        if (x > lower && x <= upper) {
            return mInfo->entries[i].value;
        }

        lower = upper;
    }

    return -1;
}
#pragma pop

bool RPSndRangeTable::startSound(s32 x) {
    u32 id = Lookup(x);
    if (id != -1) {
        return RP_GET_INSTANCE(RPSndAudioMgr)->getPlayer().StartSound(&mHandle, id);
    }

    return false;
}

bool RPSndUtility::setHandleDpdPan(nw4r::snd::SoundHandle& rHandle,
                                   const nw4r::math::VEC2& rDpdPos, f32 scale) {

    if (rHandle.IsAttachedSound()) {
        f32 pan = rDpdPos.x * scale;

        if (pan > 1.0f) {
            pan = 1.0f;
        } else if (pan < -1.0f) {
            pan = -1.0f;
        }

        rHandle.SetPan(pan);

        if (scale > 1.0f && pan > 0.8f) {
            f32 surroundPan = scale - 0.8f;

            if (surroundPan > 0.5f) {
                surroundPan = 0.5f;
            }

            rHandle.SetSurroundPan(surroundPan);
        }

        return true;
    }

    return false;
}
