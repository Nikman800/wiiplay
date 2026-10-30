#ifndef RP_PARTY_DUC_FIELD_H
#define RP_PARTY_DUC_FIELD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPDucScene/RPDucSound.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

//! @addtogroup rp_duc
//! @{

/**
 * @brief Shooting Range ground model (G3D/sht_field.brres, texture SRT animation)
 * @details vtable 80359C08. Size 0x48, verified. No members of its own.
 * @note Name is a project convention (ledger Duc_ShtField).
 */
class RPDucShtField : public RPModelObj {
public:
    RPDucShtField();
    virtual ~RPDucShtField(); // at 0x8

    virtual void Create(); // at 0xC
    virtual void Load();   // at 0x10
    virtual void Reset();  // at 0x14
};

/**
 * @brief Sky/cloud model (G3D/sht_cloud.brres, scrolling texture animation)
 * @details vtable 8035ACB8. Size 0x48, verified. No members of its own.
 * @note Name is a project convention (ledger Duc_ShtCloud).
 */
class RPDucShtCloud : public RPModelObj {
public:
    RPDucShtCloud();
    virtual ~RPDucShtCloud(); // at 0x8

    virtual void Create(); // at 0xC
    virtual void Load();   // at 0x10
    virtual void Reset();  // at 0x14
};

/**
 * @brief Grass tuft that shakes when shot (G3D/sht_grass.brres, wait/shake animations)
 * @details vtable 80359AA8. Size 0x54, verified.
 * @note Name is a project convention (ledger Duc_ShtGrass).
 */
class RPDucShtGrass : public RPModelObj {
public:
    RPDucShtGrass();
    virtual ~RPDucShtGrass(); // at 0x8

    /**
     * @brief Creates mpSndActor [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18

    void StartShake();
    void SpawnHitEffect();
    void PlaySound(s32 type);

private:
    friend class RPDucShot;
    friend class RPDucField;

    f32 mAnmEndFrame;          // at 0x48 last frame of the current animation
    u16 mAnmState;            // at 0x4C 1 while shaking
    bool mHasSecret;           // at 0x4E [M] one random tuft per row gets a different sound
    RPDucSndActor* mpSndActor; // at 0x50
};

/**
 * @brief Owns the ground, the clouds and the 27 grass tufts (singleton Duc_fieldInstance)
 * @details vtable 80359BE0. Size 0x78, verified. Rows of grass: 15 at z 60 (x = 13i - 91),
 * 6 at z 72 and 6 at z 84 (x from rodata tables).
 * @note Name is a project convention (ledger Duc_Field).
 */
class RPDucField : public RPSceneObj {
    friend class RPDucShot;

public:
    virtual ~RPDucField(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18

    static void CreateInstance();
    static void DestroyInstance();

    void CalcModelMtx();

    static RPDucField* sInstance;

private:
    RPDucShtField* mpGround;      // at 0x4
    RPDucShtCloud* mpCloud;       // at 0x8
    RPDucShtGrass* mpGrass[27];   // at 0xC
};

//! @}

#endif
