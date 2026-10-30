#ifndef RP_PARTY_COMMON_POS_HISTORY_H
#define RP_PARTY_COMMON_POS_HISTORY_H
#include <Pack/types_pack.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Newest-first history of 3D samples, filled by hand or from a controller
 * @details vtable 80357320. Size 0x14 [H] (members end at 0x10, vtable pointer last; embedded in
 * Duc_ShotPointer). The constructor calls Clear through the vtable.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPPosHistory {
public:
    RPPosHistory(s32 channel, s32 capacity);

    /**
     * @brief Shifts the history and stores rPos as the newest sample [H]
     */
    void Push(nw4r::math::VEC3 pos);
    // Wii Play: debug drawing of the samples (ledger RP_drawDebugShape; RPDucShot::UpdatePointer) [L]
    void DrawDebug();
    void SetChannel(s32 channel) {
        mChannel = channel;
    } // Wii Play: by value (RPDucShotPointer::PullFromController)

protected:
    s32 mChannel;                // at 0x0
    s32 mCapacity;               // at 0x4
    s32 mCount;                  // at 0x8
    nw4r::math::VEC3* mpSamples; // at 0xC

public:
    virtual ~RPPosHistory(); // at 0x8
    virtual void Clear();    // at 0xC
    /**
     * @brief Pushes the controller's buffered core statuses (vec at status+0xC), oldest first [H]
     */
    virtual void PullFromController(); // at 0x10
};

//! @}

#endif
