#ifndef RP_PARTY_HKY_BGM_CTRL_H
#define RP_PARTY_HKY_BGM_CTRL_H
#include <Pack/types_pack.h>

//! @addtogroup rp_hky
//! @{

/**
 * @brief Laser Hockey music control (owned by RPHkyGame at 0x8)
 * @details No vtable; allocated with 0xC bytes in RPHkyGame::Load (out-of-line constructor). Starts
 * the game BGM (sound 0x44), unmutes tracks as the clock runs down (track mask 0x5001 under 31
 * seconds, 0x6001 on request 3) and plays the requested jingle once.
 * Size 0xC, verified.
 */
class RPHkyBgmCtrl {
public:
    enum Request {
        REQUEST_NONE = 0,
        REQUEST_1 = 1,       //!< sound 0x41 [L]
        REQUEST_2 = 2,       //!< sound 0x42 [L]
        REQUEST_3 = 3,       //!< unmutes track mask 0x6001
        REQUEST_TIME_UP = 4, //!< time-up jingle 0x12 and result BGM 0x45
    };

public:
    RPHkyBgmCtrl();
    void Init() {
        mRequest = REQUEST_NONE;
        mRequested = false;
        mPrevSeqVar = -1;
        mSeqVar = -1;
    }

    /**
     * @brief Queues a request handled by the next Update
     */
    void Request(s32 request);
    /**
     * @brief Starts the game BGM (0x44, tempo 0.83 on some regions) with track mask 0x7001
     */
    void StartGameBgm();
    void Stop();
    /**
     * @brief Per frame: reads sequence variable 0, adjusts track mutes for the remaining seconds
     * and runs the pending request
     */
    void Update(s32 secondsLeft);

private:
    s32 mRequest;     // at 0x0
    bool mRequested;  // at 0x4
    s16 mPrevSeqVar;  // at 0x6 [M] last value of sequence variable 0
    s16 mSeqVar;      // at 0x8 [M] sequence variable 0 (read each frame)
};

//! @}

#endif
