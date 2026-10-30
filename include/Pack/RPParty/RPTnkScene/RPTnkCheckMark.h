#ifndef RP_PARTY_TNK_CHECK_MARK_H
#define RP_PARTY_TNK_CHECK_MARK_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPTnkScene/RPTnkActor.h>
#include <Pack/RPParty/RPTnkScene/RPTnkActorPool.h>

//! @addtogroup rp_tnk
//! @{

/**
 * @brief "X" mark left where a tank was destroyed
 * @details vtable 8035C49C. Size 0xD0, verified (RPTnkCheckMarkPool::CreateObject). Models
 * tnk_check_blue / _red / _white from G3D/tnk_check.brres, chosen by mColor.
 */
class RPTnkCheckMark : public RPTnkActor {
public:
    typedef void (RPTnkCheckMark::*StateFunc)();

public:
    RPTnkCheckMark();
    virtual ~RPTnkCheckMark(); // at 0x8

    virtual void Create();       // at 0xC
    virtual void Load();         // at 0x10
    virtual void Calculate();    // at 0x18
    virtual void CalcModelMtx(); // at 0x24 draws the model of mColor

    virtual void OnSpawn();                  // at 0x64 orientation, state 0
    virtual void ApplySpawnParam(u32 param); // at 0x68 colour
    virtual void ApplyVelocity();            // at 0x6C

    /**
     * @brief Only state: stops the mark (velocity 0) [H]
     */
    void StateStop();

private:
    u32 mColor;                 // at 0xB0 0 blue, 1 red, 2 white [M]
    s32 mState;                 // at 0xB4
    RPGrpModel* mpModels[3];    // at 0xB8
    StateFunc mStateFuncs[1];   // at 0xC4
};

/**
 * @brief Pool of check marks (singleton Tnk_checkMarkPoolInstance)
 * @details vtable 8035C438. Size 0x18, verified. Created by RPTnkTankManager::CreateTanks.
 */
class RPTnkCheckMarkPool : public RPTnkActorPool {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkCheckMarkPool(); // at 0x8

    virtual RPTnkActor* CreateObject(); // at 0x24
    virtual void Destroy();             // at 0x2C

    static RPTnkCheckMarkPool* sInstance; // Tnk_checkMarkPoolInstance
};

//! @}

#endif
