#ifndef RP_PARTY_TNK_CURSOR_H
#define RP_PARTY_TNK_CURSOR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPAxes.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPCursorTrail;
class RPSysEffect;

/**
 * @brief A player's aiming cursor: pointer position projected onto the field, drawn with
 * effects (main, tail, line to the tank)
 * @details vtable 8035B61C. Size 0x7C, verified (RPTnkCursorManager::CreateCursors). Effects
 * Tnk_cursor_1p_main/_tail/_line or the 2p ones, created for the player index only.
 */
class RPTnkCursor : public RPSceneObj {
public:
    RPTnkCursor();
    virtual ~RPTnkCursor(); // at 0x8

    virtual void Create();    // at 0xC creates the pointer trail (channel 0, 10 samples)
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Draw();      // at 0x20

    virtual void RestartMainEffect();       // at 0x24
    virtual void RestartTailEffect();       // at 0x28
    virtual void RestartLineEffect();       // at 0x2C
    /**
     * @brief 0 hidden (also used by the scene's pause callback) [M]
     */
    virtual void SetDisplayMode(s32 mode);  // at 0x30

    // Wii Play: the original stores and switches on a signed 32-bit player index.
    void CreateParts(s32 player);
    void CalcWorldPos();
    void UpdatePointerPos();
    void UpdateScreenPosAndMode();
    void UpdateLineEffect();

    const nw4r::math::VEC3& GetPos() const { return mPos; }

private:
    // Moves the own player's tail and main effects to the screen position
    void UpdateEffects();

    nw4r::math::VEC3 mPos;          // at 0x4 world position on the field
    nw4r::math::VEC3 mScale;        // at 0x10
    RPAxes mRot;                    // at 0x1C
    RPCursorTrail* mpTrail;         // at 0x40
    f32 mScreenHalfWidth;           // at 0x44
    nw4r::math::VEC2 mPointerPos;   // at 0x48 reset to (-+0.4, 0.15)
public:
    nw4r::math::VEC2 mScreenPos;    // at 0x50
private:
    bool mOutOfScreen;              // at 0x58 [M]
    s32 mPlayer;                    // at 0x5C
    s32 mDisplayMode;               // at 0x60
    RPSysEffect* mpMainEffects[2];  // at 0x64 1p, 2p (only the own one is created)
    RPSysEffect* mpTailEffects[2];  // at 0x6C
    RPSysEffect* mpLineEffects[2];  // at 0x74
};

/**
 * @brief Owns the players' cursors (singleton RPTnkCursorManager::sInstance)
 * @details vtable 8035B5C0. Size 0x1C, verified (Tnk_CursorManager_createInstance).
 */
class RPTnkCursorManager : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkCursorManager(); // at 0x8

    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14 active cursors by player count
    virtual void Calculate(); // at 0x18 active cursors
    virtual void Exit();      // at 0x1C every cursor
    virtual void Draw();      // at 0x20 active cursors

    void CreateCursors(s32 num);
    /**
     * @brief SetDisplayMode on every cursor, last first (scene pause callback) [H]
     */
    void SetDisplayModeAll(s32 mode);

    RPTnkCursor* GetCursor(s32 idx) const { return mpCursors[idx]; }

    static RPTnkCursorManager* sInstance; // 8043C8F0

private:
    s32 mCursorNum;          // at 0x4
    s32 mActiveNum;          // at 0x8
public:
    RPTnkCursor* mpCursors[4]; // at 0xC
};

//! @}

#endif
