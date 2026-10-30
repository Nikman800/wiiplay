#ifndef RP_UTILITY_LOAD_SAVE_DATA_SCENE_H
#define RP_UTILITY_LOAD_SAVE_DATA_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem/RPSysScene.h>

// Forward declarations
class RPSysSaveDataMgr;
class RPSysSystemWinMgr;

/**
 * @brief Base class for scenes which need to load the save file
 * @details Wii Play: identified by behaviour [M] (ledger Lyt_CommonMessage, code 801C1E28-801C28D8):
 * a save-data state machine with 17 states, each with an enter and a calc virtual, that checks,
 * creates and deletes the save files and shows the system window. No vtable of its own exists
 * in Wii Play (only the derived RPCommonMessage, vtable 80358040, is instantiated), so CalcState4,
 * the only state slot the derived class overrides, is modelled as pure. Size 0x54 (derived 0x68
 * verified).
 */
class RPUtlLoadSaveDataScene : public RPSysScene {
public:
    // Wii Play: empty out of line (801C28B4, 801C28B0, 801C2770, 801C2774); RPCommonMessage
    // calls them qualified
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    virtual void Exit();         // at 0x54
    virtual void UserDraw();     // at 0x5C

    virtual void EnterState0(); // at 0x60
    virtual void EnterState1(); // at 0x64
    virtual void EnterState2(); // at 0x68
    virtual void EnterState3(); // at 0x6C
    virtual void EnterState4(); // at 0x70
    virtual void EnterState5(); // at 0x74
    virtual void EnterState6(); // at 0x78
    virtual void EnterState7(); // at 0x7C
    virtual void EnterState8(); // at 0x80
    virtual void EnterState9(); // at 0x84
    virtual void EnterState10(); // at 0x88
    virtual void EnterState11(); // at 0x8C
    virtual void EnterState12(); // at 0x90
    virtual void EnterState13(); // at 0x94
    virtual void EnterState14(); // at 0x98
    virtual void EnterState15(); // at 0x9C
    virtual void EnterState16(); // at 0xA0

    virtual void CalcState0(); // at 0xA4
    virtual void CalcState1(); // at 0xA8
    virtual void CalcState2(); // at 0xAC
    virtual void CalcState3(); // at 0xB0
    virtual void CalcState4() = 0; // at 0xB4
    virtual void CalcState5(); // at 0xB8
    virtual void CalcState6(); // at 0xBC
    virtual void CalcState7(); // at 0xC0
    virtual void CalcState8(); // at 0xC4
    virtual void CalcState9(); // at 0xC8
    virtual void CalcState10(); // at 0xCC
    virtual void CalcState11(); // at 0xD0
    virtual void CalcState12(); // at 0xD4
    virtual void CalcState13(); // at 0xD8
    virtual void CalcState14(); // at 0xDC
    virtual void CalcState15(); // at 0xE0
    virtual void CalcState16(); // at 0xE4

protected:
    /**
     * @brief Binds the save-data and system-window managers and enables saving (Wii Play)
     */
    void BindSystems();
    /**
     * @brief Calls the calc virtual of the current state (Wii Play)
     */
    void CalcCurrentState();
    /**
     * @brief Calls EnterState0 (Wii Play, ledger Lyt_CommonMessage_callEnterState0, 801C2760)
     */
    void StartSaveCheck();

protected:
    s32 mSaveState;                   // at 0x44 Wii Play: current state (0..16; derived uses 17+)
    const wchar_t* unk48;             // at 0x48 Wii Play: common message 0x15/0, the save-check window argument
    RPSysSaveDataMgr* mpSaveDataMgr;  // at 0x4C Wii Play
    RPSysSystemWinMgr* mpSystemWinMgr; // at 0x50 Wii Play
};

#endif
