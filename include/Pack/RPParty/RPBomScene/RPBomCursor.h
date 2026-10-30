#ifndef RP_PARTY_BOM_CURSOR_H
#define RP_PARTY_BOM_CURSOR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPAxes.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPCursorTrail;
class RPBomGameManager;

/**
 * @brief One player's pointer, as a position and orientation on the play field
 * @details vtable 8035E800. Size 0x60, verified (Bom_CursorManager_createCursors). Not drawn
 * (Draw is empty): the player's Mii (RPBomPlayer) follows it. Calculate takes the latest
 * pointer sample of the RPCursorTrail, clamps it (x +-0.965, y +-0.53 on 16:9 or +-0.73 on
 * 4:3), scales it by half the screen width into mPos (y fixed at 1000) and turns the
 * controller's horizontal direction into mAngle; mRot is built like RPModelObj's.
 * @note Name is a project convention (ledger Bom_Cursor).
 */
class RPBomCursor : public RPSceneObj {
public:
    RPBomCursor();
    virtual ~RPBomCursor(); // at 0x8

    /**
     * @brief Creates the RPCursorTrail (10 samples) [H]
     */
    virtual void Create();    // at 0xC
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Draw();      // at 0x20 empty

    void SetControllerIndex(s32 idx);
    void UpdateFromPointer();

public:
    nw4r::math::VEC3 mPos;     // at 0x4 read by RPBomPlayer
    nw4r::math::VEC3 mScale;   // at 0x10 (2.8, 2.8, 2.8)
    RPAxes mRot;               // at 0x1C
    RPCursorTrail* mpTrail;    // at 0x40
    f32 mRange;                // at 0x44 half the screen width
    nw4r::math::VEC2 mPointer; // at 0x48 clamped pointer position (-0.4 / 0.4, 0.15 after Reset)
    nw4r::math::VEC2 mDir;     // at 0x50 [M] smoothed horizontal direction
    f32 mAngle;                // at 0x58 [M] degrees, from mDir (RPBomPlayer turns by 4x)
    s32 mController;           // at 0x5C
};

/**
 * @brief Owner of the player cursors (singleton RPBomCursorManager::sInstance)
 * @details vtable 8035E7D8. Size 0x1C, verified (Bom_CursorManager_createInstance). Reset
 * sets the active count from the game mode (1 or 2); Calculate and Draw run the active
 * cursors, Reset and Exit all created ones.
 * @note Name is a project convention (ledger Bom_CursorManager).
 */
class RPBomCursorManager : public RPSceneObj {
public:
    static RPBomCursorManager* sInstance; // 8043CA88

    virtual ~RPBomCursorManager(); // at 0x8

    virtual void Load();      // at 0x10 empty
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C
    virtual void Draw();      // at 0x20

    /**
     * @brief Allocates num cursors and calls their Create (called by RPBomScene with 2) [H]
     */
    void CreateCursors(s32 num);

    static void CreateInstance();
    static void DestroyInstance();

public:
    s32 mCursorNum;             // at 0x4
    s32 mActiveNum;             // at 0x8
    RPBomCursor* mpCursors[4];  // at 0xC
};

//! @}

#endif
