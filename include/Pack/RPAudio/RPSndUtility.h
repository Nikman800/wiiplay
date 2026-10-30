#ifndef RP_AUDIO_SND_UTILITY_H
#define RP_AUDIO_SND_UTILITY_H
#include <Pack/types_pack.h>

#include <nw4r/math.h>
#include <nw4r/snd.h>

//! @addtogroup rp_audio
//! @{

/**
 * @brief Sound utilities
 */
class RPSndUtility {
public:
    /**
     * @brief Sets the panning of the sound handle to match the DPD position
     *
     * @param rHandle Sound handle
     * @param rDpdPos DPD position
     * @param scale Panning scale
     * @return Success
     */
    static bool setHandleDpdPan(nw4r::snd::SoundHandle& rHandle,
                                const nw4r::math::VEC2& rDpdPos, f32 scale);

    /**
     * @brief Gets the active sound archive player
     */
    static nw4r::snd::SoundArchivePlayer* getPlayer() {
        return spSoundArchivePlayer;
    }

    /**
     * @brief Sets the active sound archive player
     *
     * @param pPlayer Sound archive player
     */
    static void setPlayer(nw4r::snd::SoundArchivePlayer* pPlayer) {
        spSoundArchivePlayer = pPlayer;
    }

private:
    //! Active sound archive player
    static nw4r::snd::SoundArchivePlayer* spSoundArchivePlayer;
};

/**
 * @brief Table that maps value ranges to sound IDs
 * @details Wii Play: found in RPSndUtility.cpp (assert "mInfo"); not in the vendored headers,
 * and the name is a project convention. No vtable. Used by Fsh_startSoundByName.
 */
class RPSndRangeTable {
public:
    /**
     * @brief One range: values in (previous upper, upper] map to value
     */
    struct Entry {
        s32 upper; // at 0x0 (-1 ends the table)
        s32 value; // at 0x4
    };

    /**
     * @brief Table data
     */
    struct Info {
        u32 num;          // at 0x0
        Entry entries[1]; // at 0x4
    };

public:
    // Wii Play: RPFshStartLayout's constructor [H]
    RPSndRangeTable() {
        mInfo = NULL;
    }

    /**
     * @brief Sets the table data (out of line in RPSndUtility.cpp)
     */
    void setInfo(const Info* pInfo);

    /**
     * @brief Returns the value of the range containing x, or -1 (ledger
     * Snd_SndUtility_lookupRangeValue) [H]
     */
    u32 Lookup(s32 x);

    /**
     * @brief Starts the sound of the range containing x (ledger
     * Snd_SndUtility_startRangeSound)
     *
     * @return Success
     */
    bool startSound(s32 x);

private:
    const Info* mInfo;                // at 0x0
    nw4r::snd::SoundHandle mHandle; // at 0x4
};

//! @}

#endif
