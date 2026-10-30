#ifndef RP_PARTY_PNP_HUD_H
#define RP_PARTY_PNP_HUD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

//! @addtogroup rp_pnp
//! @{

// Forward declarations
class RPPnpBalloonLyt;
class RPPnpScoreLyt;
class RPPnpServerLyt;
class RPPnpArrowLyt;
class RPPnpCountLyt;
class RPPnpResultLyt;
class RPPnpStartMsgLyt;
class RPPnpHelpMsgLyt;
class RPPnpFaultMsgLyt;
class RPPnpServeMsgLyt;
class RPPnpPointMsgLyt;

/**
 * @brief Table Tennis HUD: the eleven pingpong.arc layouts (singleton Pnp_hudInstance)
 * @details vtable 803599F8 (destructor only). Size 0x30, verified (Pnp_createHud allocates 0x30
 * and each layout with its own size; no constructors run). The score layout is drawn on view
 * 3, the others on view 1, during draw step 5; the server layout only with two players. The
 * destructor only clears the pointers.
 */
class RPPnpHud {
public:
    static void CreateInstance();
    static void DestroyInstance();

    RPPnpHud() {
        mpBalloonLyt = new RPPnpBalloonLyt;
        mpScoreLyt = new RPPnpScoreLyt;
        mpServerLyt = new RPPnpServerLyt;
        mpArrowLyt = new RPPnpArrowLyt;
        mpCountLyt = new RPPnpCountLyt;
        mpResultLyt = new RPPnpResultLyt;
        mpStartMsgLyt = new RPPnpStartMsgLyt;
        mpHelpMsgLyt = new RPPnpHelpMsgLyt;
        mpFaultMsgLyt = new RPPnpFaultMsgLyt;
        mpServeMsgLyt = new RPPnpServeMsgLyt;
        mpPointMsgLyt = new RPPnpPointMsgLyt;
    }
    virtual ~RPPnpHud(); // at 0x8

    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    static RPPnpHud* GetInstance() {
        return sInstance;
    }

    RPPnpBalloonLyt* GetBalloonLyt() const { return mpBalloonLyt; }
    RPPnpScoreLyt* GetScoreLyt() const { return mpScoreLyt; }
    RPPnpServerLyt* GetServerLyt() const { return mpServerLyt; }
    RPPnpArrowLyt* GetArrowLyt() const { return mpArrowLyt; }
    RPPnpCountLyt* GetCountLyt() const { return mpCountLyt; }
    RPPnpResultLyt* GetResultLyt() const { return mpResultLyt; }
    RPPnpStartMsgLyt* GetStartMsgLyt() const { return mpStartMsgLyt; }
    RPPnpHelpMsgLyt* GetHelpMsgLyt() const { return mpHelpMsgLyt; }
    RPPnpFaultMsgLyt* GetFaultMsgLyt() const { return mpFaultMsgLyt; }
    RPPnpServeMsgLyt* GetServeMsgLyt() const { return mpServeMsgLyt; }
    RPPnpPointMsgLyt* GetPointMsgLyt() const { return mpPointMsgLyt; }

private:
    RPPnpBalloonLyt* mpBalloonLyt;   // at 0x4
    RPPnpScoreLyt* mpScoreLyt;       // at 0x8
    RPPnpServerLyt* mpServerLyt;     // at 0xC
    RPPnpArrowLyt* mpArrowLyt;       // at 0x10
    RPPnpCountLyt* mpCountLyt;       // at 0x14
    RPPnpResultLyt* mpResultLyt;     // at 0x18
    RPPnpStartMsgLyt* mpStartMsgLyt; // at 0x1C
    RPPnpHelpMsgLyt* mpHelpMsgLyt;   // at 0x20
    RPPnpFaultMsgLyt* mpFaultMsgLyt; // at 0x24
    RPPnpServeMsgLyt* mpServeMsgLyt; // at 0x28
    RPPnpPointMsgLyt* mpPointMsgLyt; // at 0x2C

    static RPPnpHud* sInstance; // 8043C828 (Pnp_hudInstance)
};

//! @}

#endif
