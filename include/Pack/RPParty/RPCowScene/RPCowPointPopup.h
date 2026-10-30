#ifndef RP_PARTY_COW_POINT_POPUP_H
#define RP_PARTY_COW_POINT_POPUP_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPCowScene/RPCowFieldObj.h>
#include <Pack/RPParty/RPPartyCommon/RPActor.h>

#include <egg/math.h>
#include <nw4r/lyt.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPCollisionShape;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytTextBox;

/**
 * @brief Score popup over a knocked object (cowPoint_00.brlyt: L, S, XL, XS sizes and
 * "perfect")
 * @details vtable 8035D658. Size 0xD4, verified (array stride in the RPCowGame constructor,
 * 32 popups, table slot 17). Spawn picks a free popup, projects the hit position through the
 * player's camera; in two-player games a smaller copy, scaled by the distance, is shown on the
 * other player's screen.
 * @note Name is a project convention (ledger Cow_PointPopup).
 */
class RPCowPointPopup : public RPActor {
public:
    RPCowPointPopup();
    virtual ~RPCowPointPopup(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void Calculate();         // at 0x54 ledger entryDraw
    virtual void DrawScreen(u8 pass); // at 0x58
    /**
     * @brief IRPActorEnable::IsEnabled override (ledger onCollide): true for another popup on
     * the same screen [H]
     */
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60

    /**
     * @brief Ledger Cow_spawnPointPopup (called by RPCowPlayer::AddScore) [H]
     */
    static void Spawn(EGG::Vector3f pos, s32 player, s32 points, s32 perfect);

private:
    RPSysLayout* mpLayout;           // at 0x24
    RPSysLytAnmObj* mpAnmObjs[5];    // at 0x28 pointL, pointS, pointXL, pointXS, PerfectS
    s32 mAnmIdx;                     // at 0x3C current animation
    nw4r::lyt::Pane* mpPanes[15];    // at 0x40 N_Point*/N_tPoint* per size, N_perfectS
    s32 mPaneIdx;                    // at 0x7C pane the animation is bound to
    RPSysLytTextBox* mpTextBoxes[10]; // at 0x80
    RPCollisionShape* mpCollision;   // at 0xA8
    f32 mRiseSpeed;                  // at 0xAC [M] -0.2 x scale
    s32 mScreen;                     // at 0xB0 screen (camera) the popup is drawn on
    s32 mPlayer;                     // at 0xB4 scoring player
    s32 mPoints;                     // at 0xB8
    s32 mPerfect;                    // at 0xBC
    s32 mTimer;                      // at 0xC0 35 frames
    f32 mScale;                      // at 0xC4 size x distance scale
    f32 unkC8;                       // at 0xC8 [L] 0.0
    f32 unkCC;                       // at 0xCC [L] 1.0
    f32 unkD0;                       // at 0xD0 [L] 1.0

    static const f32 sRadiusNormal;  // 16
    static const f32 sRadiusPerfect; // 24
    static const f32 sScaleNormal;   // 1
    static const f32 sScalePerfect;  // 2
    static const f32 sOfsYNormal;    // 12
    static const f32 sRiseNormal;    // 4
    static const f32 sUnkCCNormal;   // 1
    static const f32 sUnkD0Normal;   // 1
    static const f32 sOfsYPerfect;   // 14
    static const f32 sRisePerfect;   // 4
    static const f32 sUnkCCPerfect;  // 1
    static const f32 sUnkD0Perfect;  // 1
    static const f32 sFarDist;       // 800
    static const f32 sNearDist;      // 0 (.sbss2)
};

//! @}

#endif
