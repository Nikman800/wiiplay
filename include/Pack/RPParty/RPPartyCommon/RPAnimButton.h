#ifndef RP_PARTY_COMMON_ANIM_BUTTON_H
#define RP_PARTY_COMMON_ANIM_BUTTON_H
#include <Pack/types_pack.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief Optional start-frame override per button animation
 * @details No vtable. Size 0x40 [H] (8 {flag, frame} entries, cleared together).
 * @note Wii Play-only framework class (ledger Lyt_FrameOverrideTable); the name is a project
 * convention.
 */
class RPFrameOverrideTable {
public:
    struct Entry {
        bool mEnabled; // at 0x0
        s32 mFrame;    // at 0x4
    };

public:
    RPFrameOverrideTable() {
        Clear();
    }

    void Clear();
    bool Has(s32 anim) const;
    /**
     * @return Override frame, or -1 when not set
     */
    s32 Get(s32 anim) const;

private:
    Entry mEntries[8]; // at 0x0
};

/**
 * @brief Layout button driven by up to eight animations (hover, press, ...)
 * @details vtable 803574A0 (five empty virtuals, same shape as RPLytObjBase but not derived from
 * it: the constructor does not create a resource accessor). Size 0xBC, verified
 * (Menu_PlayerSettingPanel_createButtons). playAnim stops all eight, then plays the requested
 * one; animations 2 and 6 are played in reverse from their last frame.
 * @note Wii Play-only framework class (ledger Lyt_AnimButton); the name is a project convention.
 */
class RPAnimButton {
public:
    static const int ANIM_MAX = 8;

public:
    RPAnimButton();

    virtual void VF_0x8();    // at 0x8
    virtual void VF_0xC();    // at 0xC
    virtual void VF_0x10();   // at 0x10
    virtual void VF_0x14();   // at 0x14
    virtual void VF_0x18();   // at 0x18

    // Out of line in the binary (RPAnimButton.cpp), called from other units.
    void SetAnim(s32 idx, RPSysLytAnmObj* pAnmObj);
    void SetLayout(nw4r::lyt::Pane* pRootPane);
    void SetPane(nw4r::lyt::Pane* pPane);
    nw4r::lyt::Pane* GetPane() const;
    void SetDefaultAnim(s32 anim) {
        mDefaultAnim = anim;
    }
    void SetRefreshFlag(u8 flag) {
        unkB4 = flag;
    }
    void SetInheritFrame(bool inherit) {
        mInheritFrame = inherit;
    }

    s32 GetCurrentAnimLength() const;
    bool PlayAnim(s32 anim, RPSysLayout* pLayout);
    void PlayDefaultAnim(RPSysLayout* pLayout);
    bool HitTest(const nw4r::math::VEC2& rPos, RPSysLayout* pLayout) const;

private:
    nw4r::lyt::Pane* mpPane;              // at 0x4 hit-test bounding pane
    nw4r::lyt::Pane* mpAnimPane;          // at 0x8 pane the animations are bound to
    s32 mCurrentAnim;                     // at 0xC -1 = none
    RPSysLytAnmObj* mpAnmObjs[ANIM_MAX];  // at 0x10
    s32 mDefaultAnim;                     // at 0x30 2 by default
    RPFrameOverrideTable mStopFrames;     // at 0x34 frame to stop at (anim plays to a fixed frame)
    RPFrameOverrideTable mStartFrames;    // at 0x74 frame to start from
    u8 unkB4;                             // at 0xB4 1: refresh the pane after binding [M]
    bool mTexturePatternOnly;             // at 0xB5 bind texture patterns instead of the whole anim
    bool mInheritFrame;                   // at 0xB6 continue from the paired anim's frame
    s32 mDefaultFrame;                    // at 0xB8
};

/**
 * @brief Table of up to 100 {id, RPAnimButton} pairs looked up by id
 * @details vtable 80357508 (one slot). Size 0x324, verified
 * (Menu_PlayerSettingPanel_createButtons).
 * @note Wii Play-only framework class (ledger Lyt_AnimButtonTable); the name is a project
 * convention.
 */
class RPAnimButtonTable {
public:
    static const int ENTRY_MAX = 100;

    struct Entry {
        s32 mId;                // at 0x0
        RPAnimButton* mpButton; // at 0x4
    };

public:
    RPAnimButtonTable();
    virtual void VF_0x8(); // at 0x8 empty (RPAnimButton_3.cpp)

    void Add(s32 id, RPAnimButton* pButton);
    s32 GetAnimLengthById(s32 id) const;
    nw4r::lyt::Pane* GetPaneById(s32 id) const;
    /**
     * @return Id of the first button containing the point, or -1
     */
    s32 HitTest(const nw4r::math::VEC2& rPos, RPSysLayout* pLayout) const;
    void PlayAnimById(s32 id, s32 anim, RPSysLayout* pLayout);

private:
    Entry mEntries[ENTRY_MAX]; // at 0x4
};

//! @}

#endif
