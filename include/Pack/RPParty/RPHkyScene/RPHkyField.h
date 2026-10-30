#ifndef RP_PARTY_HKY_FIELD_H
#define RP_PARTY_HKY_FIELD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

//! @addtogroup rp_hky
//! @{

/**
 * @brief Line segment for 2D (XZ-plane) collision: rink walls and paddle outlines
 * @details First wall built in Create(): pos (x, 0, -220), dir (1, 0, 0). Size 0x1C.
 */
struct RPHkySegment {
    nw4r::math::VEC3 mPos; // at 0x0
    nw4r::math::VEC3 mDir; // at 0xC (unit direction)
    f32 mLength;           // at 0x18 [M]
};

/**
 * @brief Laser Hockey rink
 * @details vtable 803592D0. Size 0x240, verified. Source: RPHkyField.cpp (0x802191DC-0x8021A4B4).
 */
class RPHkyField : public RPSimpleModel {
public:
    RPHkyField() {} // inlined into RPHkyGame's constructor
    virtual ~RPHkyField(); // at 0x8

    virtual void Calculate(); // at 0x14

    virtual void Create(); // at 0x1C
    virtual void Load();   // at 0x20
    virtual void Reset();  // at 0x24
    virtual void Exit();   // at 0x28

    /**
     * @brief Index of the wall the iterator last returned, or -1
     */
    s32 GetHitWallIndex() const;
    /**
     * @brief Starts wall animation idx (clr animation, frame 0, rate 1) [M]
     */
    void PlayWallAnim(int idx);
    /**
     * @brief Tests a rectangle against the walls from the iterator down; on a hit returns true
     * with the wall normal and the penetration depth, and steps the iterator [M]
     */
    BOOL TestRectCollision(f32 halfWidth, f32 halfDepth, const nw4r::math::VEC3& rPos,
                           const nw4r::math::VEC3& rDir, nw4r::math::VEC3* pNormal, f32* pDepth);
    /**
     * @brief Tests a circle against the walls from the iterator down (see TestRectCollision) [M]
     */
    BOOL TestCircleCollision(f32 radius, const nw4r::math::VEC3& rPos, nw4r::math::VEC3* pNormal,
                             f32* pDepth);
    void ResetPaddleWallIter();
    void ResetPuckWallIter();

    static f32 sScale;  // 8043B628 (Hky_fieldScale), 1.0
    static f32 sLength; // 8043B630 (Hky_fieldLength), 880.0
    static f32 sWidth;  // 8043B634 (Hky_fieldWidth), 440.0

    // Rink constants in .sdata2, also read by RPHkyPaddle (defined in RPHkyField.cpp)
    static const f32 cRinkDepth; // 8043F750 600
    static const f32 cHalfDepth; // 8043F754 300
    static const f32 cCorner;    // 8043F758 80
    static const f32 cGoalWidth; // 8043F75C 200
    static const f32 cGoalInset; // 8043F760 8

private:
    RPHkySegment mWalls[18];    // at 0x44
    RPHkySegment* mpWallIter;   // at 0x23C
};

//! @}

#endif
