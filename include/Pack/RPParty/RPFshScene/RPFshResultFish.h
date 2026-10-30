#ifndef RP_PARTY_FSH_RESULT_FISH_H
#define RP_PARTY_FSH_RESULT_FISH_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>

#include <nw4r/math.h>

//! @addtogroup rp_fsh
//! @{

/**
 * @brief Fish model shown on the result screen (2 x 6 x 50 made by RPFshResultLayout)
 * @details vtable 80358E98. Size 0x78, verified (Fsh_ResultLayout_ctor). The constructor sets
 * the basis to face -X; Create loads the fsh_fish_* model of mType.
 */
// Direct base: an intermediate RPFshActor adds an extra destructor null check.
class RPFshResultFish : public RPActorBase {
public:
    explicit RPFshResultFish(s32 type);
    virtual ~RPFshResultFish(); // at 0x8

    virtual void Create();    // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84 empty, out of line

    /**
     * @brief Places the fish under a screen position (stored in mScreenPos) [M]
     */
    void SetScreenPos(const nw4r::math::VEC2& rPos);
    // Wii Play: returns a copy (a weak out-of-line copy in RPFshResultLayout.cpp)
    nw4r::math::VEC2 GetScreenPos() const {
        return mScreenPos;
    }
    void Show() {
        unk68 = 1;
        unk6C = true;
    }

private:
    s32 mType;                  // at 0x60 RPFshFish::EType
    u32 unk64;                  // at 0x64 [L]
    u32 unk68;                  // at 0x68 [L]
    bool unk6C;                 // at 0x6C [L]
    nw4r::math::VEC2 mScreenPos; // at 0x70
};

//! @}

#endif
