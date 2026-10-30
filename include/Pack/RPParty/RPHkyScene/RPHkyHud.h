#ifndef RP_PARTY_HKY_HUD_H
#define RP_PARTY_HKY_HUD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

//! @addtogroup rp_hky
//! @{

// Forward declarations
class RPHkyStartMsgLyt;
class RPHkyBalloonLyt;
class RPHkyTimeLyt;
class RPHkyScoreLyt;
class RPHkyGoalMsgLyt;
class RPHkyTimeUpMsgLyt;
class RPHkyMsg03Lyt;

/**
 * @brief Laser Hockey HUD: the seven airHockey.arc layouts (singleton Hky_hudInstance)
 * @details vtable 803594E0 (destructor only). Size 0x20, verified (Hky_createHud allocates 0x20
 * and each layout with its own size; no constructors run). The layouts are drawn on view 1
 * (score layout on view 2) during draw step 5. The destructor only clears the pointers.
 */
class RPHkyHud {
public:
    static void CreateInstance();
    static void DestroyInstance();

    RPHkyHud() {
        mpStartMsgLyt = new RPHkyStartMsgLyt;
        mpBalloonLyt = new RPHkyBalloonLyt;
        mpTimeLyt = new RPHkyTimeLyt;
        mpScoreLyt = new RPHkyScoreLyt;
        mpGoalMsgLyt = new RPHkyGoalMsgLyt;
        mpTimeUpMsgLyt = new RPHkyTimeUpMsgLyt;
        mpMsg03Lyt = new RPHkyMsg03Lyt;
    }
    virtual ~RPHkyHud(); // at 0x8

    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    static RPHkyHud* GetInstance() {
        return sInstance;
    }

    RPHkyStartMsgLyt* GetStartMsgLyt() const { return mpStartMsgLyt; }
    RPHkyBalloonLyt* GetBalloonLyt() const { return mpBalloonLyt; }
    RPHkyTimeLyt* GetTimeLyt() const { return mpTimeLyt; }
    RPHkyScoreLyt* GetScoreLyt() const { return mpScoreLyt; }
    RPHkyGoalMsgLyt* GetGoalMsgLyt() const { return mpGoalMsgLyt; }
    RPHkyTimeUpMsgLyt* GetTimeUpMsgLyt() const { return mpTimeUpMsgLyt; }
    RPHkyMsg03Lyt* GetMsg03Lyt() const { return mpMsg03Lyt; }

private:
    RPHkyStartMsgLyt* mpStartMsgLyt;   // at 0x4
    RPHkyBalloonLyt* mpBalloonLyt;     // at 0x8
    RPHkyTimeLyt* mpTimeLyt;           // at 0xC
    RPHkyScoreLyt* mpScoreLyt;         // at 0x10
    RPHkyGoalMsgLyt* mpGoalMsgLyt;     // at 0x14
    RPHkyTimeUpMsgLyt* mpTimeUpMsgLyt; // at 0x18
    RPHkyMsg03Lyt* mpMsg03Lyt;         // at 0x1C

    static RPHkyHud* sInstance; // 8043C7F0 (Hky_hudInstance)
};

//! @}

#endif
