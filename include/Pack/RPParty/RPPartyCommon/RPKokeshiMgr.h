#ifndef RP_PARTY_COMMON_KOKESHI_MGR_H
#define RP_PARTY_COMMON_KOKESHI_MGR_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPKernel/IRPSysKokeshiCtrlDataCallback.h>
#include <Pack/RPKernel/RPSysKokeshi.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPGrpCamera;
class RPSysKokeshiCtrlMgr;
namespace EGG {
class Archive;
}
namespace nw4r {
namespace lyt {
class Pane;
}
}

/**
 * @brief Mii (Kokeshi) head shown in the menus, with its own scale and matrix
 * @details vtable 80357A70 (RPSysKokeshi slots; the IRPSysHostIOSocket "Kokeshi Head" part at
 * 0xB4). Size 0xF8, verified (Mii_KokeshiMgr_ctor_801D2BB8, Mii_KokeshiMgr_onRemoteMiiAdded).
 * Adds no virtuals: the ledger's Mii_KokeshiHead_build and Mii_KokeshiHead_registerLightTex are
 * RPSysKokeshi::LoadResource and RPSysKokeshi::ApplyLightTexture (shared with the Pnp_Spectator
 * vtable 80359980), and Mii_KokeshiHead_callDbVf20 is RPSysKokeshi::GetRightHandJointMtx.
 * Wly_Avatar (vtable 8035C7E0) derives from it.
 * @note Wii Play-only framework class (ledger Mii_KokeshiHead); the name is a project convention.
 */
class RPKokeshiHead : public RPSysKokeshi, public IRPSysHostIOSocket {
public:
    explicit RPKokeshiHead(const RPSysKokeshiGenInfo* pGenInfo);
    virtual ~RPKokeshiHead(); // at 0x8

    /**
     * @brief Sets flag bit 0, scale (1, 1, 1) and an identity matrix [H]
     */
    void ResetTransform();
    /**
     * @brief Applies mMtx and mScale to the body model, puts the face model on the head joint,
     * sets both models' visibility and enters them for drawing when flag bit 0 is set [H]
     * (ledger Mii_MiiModel_applyTransform)
     */
    void Calculate(bool visible);
    void SetTransform(const nw4r::math::MTX34& rMtx, const nw4r::math::VEC3& rScale);

    const RFLAdditionalInfo& GetAdditionalInfo() const {
        return mAdditionalInfo;
    }
    u16 GetIndex() const {
        return GetGenInfo().GetLocation().GetIndex();
    }

private:
    u32 mFlags;                 // at 0xB8 bit 0: enter the models for drawing
    nw4r::math::VEC3 mScale;    // at 0xBC
    nw4r::math::MTX34 mMtx;     // at 0xC8
};

/**
 * @brief Menu Mii manager: Kokeshi heads per data source (database, remotes), the Mii camera and
 * 3D view, player slots and remote Mii loading
 * @details vtable 80357AB4 (IRPSysHostIOSocket "KokeshiMgr", IRPSysKokeshiCtrlDataCallback part
 * at 0x4). Size 0xB8, verified (Menu_MenuScene_configure allocates 0xB8 with
 * Mii_KokeshiMgr_ctor_801D2BB8). Six sources of up to {100, 6, 10, 10, 10, 10} heads.
 * @note Wii Play-only framework class (ledger Mii_KokeshiMgr, which also holds Wii Sports
 * RPSysKokeshiManager functions, vtable 8034F490); the name is a project convention.
 */
class RPKokeshiMgr : public IRPSysHostIOSocket, public IRPSysKokeshiCtrlDataCallback {
public:
    static const int SOURCE_NUM = 6;
    static const int PLAYER_NUM = 4;

    //! A player's Mii: head, source, index in the source, type and database index
    struct Player {
        RPKokeshiHead* mpHead; // at 0x0
        s32 mSource;           // at 0x4
        u16 mIndex;            // at 0x8
        s8 mType;              // at 0xA
        u16 mMiiIndex;         // at 0xC
    };

public:
    RPKokeshiMgr(u8 viewNo, u8 sceneNo);
    virtual ~RPKokeshiMgr(); // at 0x8

    /**
     * @brief A remote's Mii data became available: builds a head for it [M]
     */
    virtual void onCreateResource(s32 chan, u8 index); // at 0x24
    virtual void onReleaseResource(s32 chan, u8 index); // at 0x28

    void Load();
    void ResetAll();
    void Calculate();
    void SetViewMode(s32 mode);
    void SortMiis();
    void AttachKokeshi(s32 source, s32 idx, s32 slot);
    void DetachKokeshi(s32 slot);
    void DetachSource(s32 source);
    s32 GetPlayerSlot(s32 player) const;
    void SetPlayerSlot(s32 player, s32 slot);
    void SetPlayerSlotByType(s32 player, s32 type);
    void ApplyPlayerHighlights();
    void FixPlayerMiiIndices();
    void FixPartyMiiIndices();
    bool IsRecordsChanged() const;
    void ClearRecordsChanged();
    void RequestRemoteMii(s32 chan);
    bool IsRemoteMiiReady(s32 chan) const;
    const wchar_t* GetMiiName(s32 source, s32 idx) const;
    u8 GetAlpha() const;
    void SetVisible(bool visible);
    void CalcPaneMtx();
    void CalcPaneScale();

    // Signatures as compiled (RPKokeshiMgr_3)
    void Load(EGG::Archive* pArchive);
    void SetViewMode(s32 mode, s32 layout);
    void AttachKokeshi(nw4r::lyt::Pane* pPane, nw4r::lyt::Pane* pScalePane, s32 source,
                       s32 idx);
    void DetachKokeshi(s32 source, s32 idx);
    Player* GetPlayer(s32 player);
    void SetPlayerSlot(s32 source, u16 idx, s32 player);
    void SetPlayerSlotByType(s8 type, u16 miiIndex, s32 player);
    bool IsInPlayers(s32 source, u16 idx, s32 num) const;
    void ApplyPlayerHighlights(s32 num);
    void ReleaseRemoteMii(s32 chan);
    bool IsRemoteMiiReady(s32 chan);
    BOOL IsRemoteLoadError(s32 chan) const;
    EGG::Matrix34f CalcPaneMtx(nw4r::lyt::Pane* pPane);
    EGG::Vector3f CalcPaneScale(const nw4r::lyt::Pane* pPane);

private:
    struct Slot {
        nw4r::lyt::Pane** mpPanes;      // at 0x0 new[n]
        nw4r::lyt::Pane** mpScalePanes; // at 0x4 new[n]
    };

    Player mPlayers[PLAYER_NUM];             // at 0x8
    bool mVisible;                           // at 0x48
    s32 mAlpha;                              // at 0x4C
    bool mRecordsChanged;                    // at 0x50
    u8 mViewNo;                              // at 0x51
    u8 mSceneNo;                             // at 0x52
public:
    u8 mHeadNum[SOURCE_NUM];                 // at 0x53
private:
    GXColor mHighlightColor;                 // at 0x59 (130, 140, 150, 0)
    f32 mOffsetY;                            // at 0x60
    f32 mScale;                              // at 0x64
    RPGrpCamera* mpCamera;                   // at 0x68
    Slot mSlots[SOURCE_NUM];                 // at 0x6C
    RPKokeshiHead** mpHeads[SOURCE_NUM];     // at 0x9C new[n] per source
    RPSysKokeshiCtrlMgr* mpCtrlMgr;          // at 0xB4 remote Mii loader
};

//! @}

#endif
