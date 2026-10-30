#ifndef RP_PARTY_COMMON_COMMON_H
#define RP_PARTY_COMMON_COMMON_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpDrawObject.h>
#include <Pack/RPKernel/IRPSysHostIOSocket.h>

#include <egg/math.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPFaceCtrl;
class RPGrpCamera;
class RPGrpScreen;
class RPLeadCursor;
class RPMedalResult;
class RPPauseText;
class RPResultMenu;
class RPScreenFade;
class RPSysKokeshi;
class RPSysMessage;
class RPVsWinner;

/**
 * @brief Parameters every minigame passes to RPCommon::CreateInstance
 * @details No vtable. Size 0x8 (copied as two words into RPCommon + 0x10). SetDefault gives
 * game 6 and one player.
 * @note Wii Play-only framework class (ledger Party_CommonParam); the name is a project
 * convention.
 */
class RPCommonParam {
public:
    // Wii Play: out of line (Party_CommonParam_setDefault, 801E7CB4): game 6, one player
    RPCommonParam();

    void SetDefault() {
        mGame = 6;
        mPlayerNum = 1;
    }

public:
    s32 mGame;      // at 0x0 game index 0-8 (message index 0-5, 7-9)
    s32 mPlayerNum; // at 0x4
};

/**
 * @brief Result-screen services shared by every minigame (singleton sInstance)
 * @details vtable 80358290 (IRPSysHostIOSocket, IRPGrpDrawObject part at 0x4 with its vtable
 * at 0x8). Size 0x54, verified (Party_Common_create). Every minigame creates it with its
 * RPCommonParam, then loads, resets, calculates and destroys it with the scene. Owns the
 * single-player medal result, the result menu, the two-player winner screen, the off-screen
 * pointer guide, the pause text and the screen fade, plus a 2D/3D view (camera and screen)
 * and one Mii avatar with blinking per player. UserDraw draws the result layouts on views
 * 0x1A (layer 1A) and 0x1C (layer 1C) during draw step 5.
 * vtable layout (CodeWarrior): IRPSysHostIOSocket part 0x8-0x10, IRPGrpDrawObject part at
 * 0x14 (thunks -4), UserDraw primary slot at 0x28.
 * @note Wii Play-only framework class (ledger Party_Common); the name is a project
 * convention.
 */
class RPCommon : public IRPSysHostIOSocket, public IRPGrpDrawObject {
public:
    static void CreateInstance(const RPCommonParam& rParam);
    /**
     * @brief Model resources the common objects need (801E7850) [L]
     */
    static u16 GetResourceNum();
    static void DestroyInstance();
    /**
     * @brief Renderer view of a result layout layer: layer + 0x1A (ledger Party_addLayerBase1A)
     */
    static u8 GetLayoutViewNo(s32 layer);
    /**
     * @brief Draw layer of the result effects, 7 (ledger Party_getMedalDrawLayer)
     */
    static u8 GetEffectDrawLayer();
    /**
     * @brief A pressed on the active controller; the player is unused (ledger Party_isPadAPressed)
     */
    static bool IsAPressed(s32 player);
    /**
     * @brief A or B pressed by any of the first playerNum players (ledger
     * Party_isAnyPlayerPressingAB)
     */
    static bool IsAnyABPressed(s32 playerNum);
    /**
     * @brief Message index and plural form of a game's record (ledger Party_getGameMsgIndex)
     */
    static void GetGameMsgIndex(s32 num, s32 game, s32* pGameIdx, s32* pPlural);
    static RPCommon* GetInstance() {
        return sInstance;
    }

    // Wii Play: RPSysPauseMgr opens the pause menu only while this is set [M]
    bool IsPauseEnable() const {
        return unk30;
    }

    virtual ~RPCommon(); // at 0x8

    /**
     * @brief IRPGrpDrawObject override (ledger Party_Common_draw) [H]
     */
    virtual void UserDraw(); // at 0x28

    void LoadResources();
    void Reset();
    void Calculate();
    void GetGameMsgIndex(s32 num, s32* pGameIdx, s32* pPlural) const;
    void PlayAvatarAnm(s32 player, u16 anm, f32 frame);
    /**
     * @brief Places, scales and calculates a player's avatar body and head (ledger
     * Party_Common_setAvatarScale)
     */
    void SetAvatarScale(s32 player, f32 scale, const nw4r::math::MTX34& rBodyMtx);
    /**
     * @brief Screen position to world position with the common camera (ledger
     * Party_Common_screenToWorld)
     */
    EGG::Vector3f ScreenToWorld(const nw4r::math::VEC2& rScreenPos) const;
    /**
     * @brief Game unlocked by clearing this one, 9 after the last (ledger Party_getNextGameId)
     */
    s32 GetNextGame() const;

    s32 GetGame() const {
        return mParam.mGame;
    }
    s32 GetPlayerNum() const {
        return mParam.mPlayerNum;
    }
    /**
     * @brief Medal saved for this game by the first player (ledger Party_getSavedMedal 801E7800)
     */
    s32 GetSavedMedal() const;

    RPMedalResult* GetMedalResult() const {
        return mpMedalResult;
    }
    RPResultMenu* GetResultMenu() const {
        return mpResultMenu;
    }
    RPVsWinner* GetVsWinner() const {
        return mpVsWinner;
    }
    RPLeadCursor* GetLeadCursor() const {
        return mpLeadCursor;
    }
    RPPauseText* GetPauseText() const {
        return mpPauseText;
    }
    RPScreenFade* GetScreenFade() const {
        return mpScreenFade;
    }
    RPSysKokeshi* GetAvatar(s32 player) const {
        return mpAvatars[player];
    }
    RPSysMessage* GetMessage() const {
        return mpMessage;
    }

private:
    explicit RPCommon(const RPCommonParam& rParam);

public:
    u32 unkC;                                      // at 0xC cleared by Reset [L]
    RPCommonParam mParam;                          // at 0x10
    RPMedalResult* mpMedalResult;                  // at 0x18
    RPResultMenu* mpResultMenu;                    // at 0x1C
    RPVsWinner* mpVsWinner;                        // at 0x20
    RPLeadCursor* mpLeadCursor;                    // at 0x24
    RPPauseText* mpPauseText;                      // at 0x28
    RPScreenFade* mpScreenFade;                    // at 0x2C
    bool unk30;                                    // at 0x30 1 after Reset, 0 in the record screen
    RPGrpCamera* mpCamera;                         // at 0x34
    RPGrpScreen* mpScreen;                         // at 0x38
    RPSysKokeshi* mpAvatars[RP_PARTY_MAX_PLAYERS]; // at 0x3C
    RPFaceCtrl* mpFaceCtrls[RP_PARTY_MAX_PLAYERS]; // at 0x44
    RPSysMessage* mpMessage;                       // at 0x4C party_message.bmg
    bool mEntryAvatars;                            // at 0x50 avatar models entered for drawing

    static RPCommon* sInstance; // 8043C6D8
};

//! @}

#endif
