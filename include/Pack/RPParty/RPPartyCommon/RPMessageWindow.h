#ifndef RP_PARTY_COMMON_MESSAGE_WINDOW_H
#define RP_PARTY_COMMON_MESSAGE_WINDOW_H
#include <Pack/types_pack.h>

#include <nw4r/lyt.h>

#include <egg/core.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPAButtonPrompt;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSysLytTextBox;
class RPSysMessage;

/**
 * @brief Speech-bubble message window (messageWindow.arc: messageWindowB_00 and
 * messageWindowW_00 layouts) with a typewriter text effect and an A-button prompt
 * @details vtable 8034F6E0 (destructor only). Size 0x284, verified
 * (Lyt_MessageWindow_createInstance). Singleton at 8043C4F0, drawn by the system drawer.
 * mFlags bits: 1 visible, 2 opening, 4 open, 8 text in, 0x10 text shown, 0x20 text out, 0x40 / 0x80
 * closing. A queued text (mNextText / mpNextMessage) replaces the current one after the text-out fade.
 * @note Wii Play-only framework class (ledger Lyt_MessageWindow); the name is a project convention.
 */
class RPMessageWindow {
public:
    static RPMessageWindow* CreateInstance(EGG::Heap* pHeap);

    explicit RPMessageWindow(EGG::Heap* pHeap);
    virtual ~RPMessageWindow(); // at 0x8

    void Load();
    void Reset();
    void Calculate();
    void Draw();

    static RPMessageWindow* GetInstance() {
        return sInstance;
    }

private:
    // Wii Play: a const read; Calculate's opening-complete path reloads mFlags through it
    u32 GetFlags() const {
        return mFlags;
    }

    /**
     * @brief Starts closing: fades the text out when it is shown, otherwise ends the wait
     * after one more frame (inlined into Calculate's auto-close) [M]
     */
    bool Close() {
        if (mFlags & 1) {
            if (mFlags & 8) {
                mFlags = 0;
                mFlags |= 0x81;
                return true;
            }
            if (!unk27E) {
                mAutoCloseFrames = 1;
                return true;
            }
        }
        return false;
    }

    static RPMessageWindow* sInstance; // ledger Lyt_messageWindowInstance


    static const int WINDOW_NUM = 2;
    static const int TEXT_LEN = 0x80;

    EGG::Heap* mpHeap;                            // at 0x4
    RPSysLytResAccessor* mpResAccessor;           // at 0x8
    RPSysLayout* mpLayouts[WINDOW_NUM];           // at 0xC B, W
    RPSysLytAnmObj* mpAnmObjs[WINDOW_NUM][2];     // at 0x14 in / out per layout
    nw4r::lyt::Pane* mpTexWindPanes[WINDOW_NUM];  // at 0x24 N_texWind_00
    nw4r::lyt::Pane* mpPosPanes[WINDOW_NUM][3];   // at 0x2C N_twPosition_00..02
    RPSysLytTextBox* mpTextBoxes[WINDOW_NUM];     // at 0x44 T_message_00
    u32 mFlags;                                   // at 0x4C
    s32 mWindow;                                  // at 0x50 layout in use
    s32 unk54;                                    // at 0x54 1 after reset
    u16 mAutoCloseFrames;                         // at 0x58 0 = wait for input [M]
    u16 mAutoCloseTimer;                          // at 0x5A
    u16 unk5C;                                    // at 0x5C 120 after reset
    u16 mFadeTimer;                               // at 0x5E
    u16 mTextTimer;                               // at 0x60 text fade, 10 frames
    wchar_t mText[TEXT_LEN];                      // at 0x62
    wchar_t mNextText[TEXT_LEN + 1];              // at 0x162
    RPSysMessage* mpMessage;                      // at 0x264 message shown with setMessage (NULL: none)
    u32 mGroup;                                   // at 0x268
    u32 mIndex;                                   // at 0x26C
    RPSysMessage* mpNextMessage;                  // at 0x270 queued message
    u32 mNextGroup;                               // at 0x274
    u32 mNextIndex;                               // at 0x278
    u8 mTextAlpha[WINDOW_NUM];                    // at 0x27C text box alpha from the layout
    bool unk27E;                                  // at 0x27E 1 after reset: A prompt off [M]
    RPAButtonPrompt* mpAButtonPrompt;             // at 0x280
};

//! @}

#endif
