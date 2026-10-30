#ifndef RP_PARTY_COW_SOUND_H
#define RP_PARTY_COW_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndSceneSound.h>

//! @addtogroup rp_cow
//! @{

/**
 * @brief Scene sound object of Charge! (vtable 8035CE08, built inline in the RPCowGame
 * constructor). Size 0x18, verified.
 */
class RPCowSceneSound : public RPSndSceneSound {
public:
    virtual ~RPCowSceneSound(); // at 0x8

    /**
     * @brief With 0, sets the four players' 3D sound parameters in RPSndAudioMgr (mode 0x20,
     * 1800 / 100 / 900) [M]
     */
    virtual void initSound3D(s32 arg); // at 0xC
};

/**
 * @brief Sound actor of a course object (ledger Cow_ObjSound, vtable 8035D170, built inline in
 * RPCowFieldObj::VF_0x3C)
 * @details RPSndObject (TRPSndObject<4>, vtable 8035C6BC, 0x84 bytes) plus a sound mode and a
 * parameter; overrides startSound(u32, u32) and holdSound(u32, u32) (ledger
 * Cow_PlayerSound_playSound / Cow_PlayerSound_holdSound, shared with RPCowPlayerSound).
 * Size 0x90, verified.
 */
class RPCowObjSound : public RPSndObject {
private:
    /**
     * @brief FAKEMATCH: stand-in for unknown code of the original between the TRPSndObject part
     * and RPCowObjSound's vtable stores (compare RPSndGameObjectHelper)
     * @details Its destructor's dead store, together with `#pragma opt_dead_code off` around
     * RPCowFieldObj::VF_0x3C, survives until scheduling and gives the original's registers
     * (__vt__36TRPSndAudioActorBaseWithHandles<4,4>+0x24 in r0, `li r0, 2` after its store);
     * nothing of it reaches the object code (its weak destructor is unreferenced and stripped).
     */
    struct CtorScope {
        int unk0;
        ~CtorScope() {
            unk0 = 0;
        }
    };

public:
    // Inlined into the player sound's constructor (RPCowPlayer::VF_0x3C) [H]
    RPCowObjSound(nw4r::snd::SoundArchivePlayer& rPlayer)
        : RPSndObject(rPlayer, *static_cast<nw4r::snd::Sound3DManager*>(NULL)),
          mMode(2),
          unk88(0),
          unk8C(0.0f) {}

    // Inlined into RPCowFieldObj::VF_0x3C; fetches the player itself (as RPFshFishSnd does,
    // which decides the callee-saved registers) [M]
    RPCowObjSound()
        : RPSndObject((CtorScope(), RPSndAudioMgr::instance()->getPlayer()),
                      *static_cast<nw4r::snd::Sound3DManager*>(NULL)),
          mMode(2),
          unk88(0),
          unk8C(0.0f) {}

    // Pan mode (RPCowFieldObj::SetSoundMode) [H]
    void SetObjMode(s32 mode) {
        mMode = mode;
    }

    // Wii Play: declared before the destructor, so startSound is the key function and the vtable
    // (8035D170) and the destructor thunk come out in RPCowSound_2.cpp (the destructor itself is
    // defined with the player sound in RPCowPlayer_6.cpp). Overrides keep their base slots.
    // mMode 0: handle 0; 1: handles 0 and 1 panned left/right; 2: handle 0 left; 3: handle 1 right
    virtual nw4r::snd::SoundHandle* startSound(u32 id, u32 handle = 0); // at 0x5C
    virtual nw4r::snd::SoundHandle* holdSound(u32 id, u32 handle = 0);  // at 0x7C

    virtual ~RPCowObjSound(); // at 0x40

private:
    // Wii Play: TRPSndObject<4>::startSound / holdSound(u32, u32), expanded inline in the
    // overrides (the template members are not inlined); defined in RPCowSound_2.cpp
    nw4r::snd::SoundHandle* StartSoundBase(u32 id, u32 handle);
    nw4r::snd::SoundHandle* HoldSoundBase(u32 id, u32 handle);

protected:
    s32 mMode; // at 0x84 2 from the constructors; read by holdSound (RPCowFieldObj::SetSoundMode)
    u32 unk88; // at 0x88 [L] 0 from the constructors
    f32 unk8C; // at 0x8C [L]
};


//! @}

#endif

// The player's sound actor is declared after RPCowPlayer in RPCowPlayer_6.cpp's translation
// unit: CodeWarrior emits vtables in reverse class declaration order, and the original .data
// has RPCowPlayerSound's vtable (8035CF30) before RPCowPlayer's (8035D00C). That file defines
// RP_COW_PLAYER_SOUND_LATE, includes everything, then includes this header again with it
// undefined.
#if !defined(RP_COW_PLAYER_SOUND_LATE) && !defined(RP_PARTY_COW_PLAYER_SOUND_DECLARED)
#define RP_PARTY_COW_PLAYER_SOUND_DECLARED

//! @addtogroup rp_cow
//! @{

/**
 * @brief The player's sound actor (ledger Cow_PlayerSound, vtable 8035CF30, built inline in
 * RPCowPlayer::VF_0x3C)
 * @details RPCowObjSound [M] (same members up to 0x90 and the same startSound / holdSound
 * overrides) plus a bool and an f32 (-1); also overrides set3DPosition (ledger
 * Cow_PlayerSound_setParam). Size 0x98, verified.
 */
class RPCowPlayerSound : public RPCowObjSound {
public:
    // Inlined into RPCowPlayer::VF_0x3C [H]
    RPCowPlayerSound(nw4r::snd::SoundArchivePlayer& rPlayer)
        : RPCowObjSound(rPlayer), unk90(false), unk94(-1.0f) {}

    // Set by RPCowPlayer::LoadResource (pan mode and remote speaker mask) [M]
    void SetMode(s32 mode) {
        mMode = mode;
    }
    void SetRemoteFlag(u32 flag) {
        mRemoteFlag = flag;
    }

    virtual ~RPCowPlayerSound(); // at 0x40

    virtual void set3DPosition(const nw4r::math::VEC3& rPos); // at 0x48

    bool PlayHit(u32 id, f32 speed);           // RPCowSound_2.cpp
    bool HoldSpeedSound(f32 speed);            // RPCowSound_2.cpp
    bool HoldRunStepSound(s32 type, f32 speed); // RPCowSound_2.cpp

private:
    bool unk90; // at 0x90 [L] false from the constructor
    f32 unk94;  // at 0x94 [L] -1 from the constructor, written by set3DPosition
};

//! @}

#endif
