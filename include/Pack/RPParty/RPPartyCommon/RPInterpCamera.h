#ifndef RP_PARTY_COMMON_INTERP_CAMERA_H
#define RP_PARTY_COMMON_INTERP_CAMERA_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/RPGrpCamera.h>

#include <egg/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Eased 0..1 timer: frame counter, duration and easing curve
 * @details Size 0xC, verified (RPInterpCamera allocates one). The ledger splits it in two:
 * Init/IsEnd are RP_Range_*, Start/Step/GetValue are Fsh_Ease_* (801B9E44..801B9F5C).
 * Ease types: 0 linear, 1 sin(t*pi/2), 2 t^2, 3 sqrt(t), 4 (1 - cos(t*pi)) / 2.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPRange {
public:
    // Wii Play: out of line (RP_Range_ctor, 801B9F70; RPFshFish's constructor calls it)
    RPRange();

    /**
     * @brief mFrame == mDuration [H]
     */
    bool IsEnd() const;

    /**
     * @brief Sets the ease type and duration and restarts [H] (out of line: Fishing calls it)
     */
    void Start(s32 easeType, s32 duration);

    /**
     * @brief mFrame++ up to mDuration [H] (out of line: Fishing calls it)
     */
    void Step();

    f32 GetValue();

    /**
     * @brief The same easing curves as GetValue for an explicit t (RP_calcEase) [H]
     */
    static f32 CalcEase(f32 t, s32 easeType);

private:
    s32 mFrame;    // at 0x0
    s32 mDuration; // at 0x4
    s32 mEaseType; // at 0x8
};

/**
 * @brief RPGrpCamera that eases position, target and up from one view to another
 * @details vtable 803573E0 (at 0x174). Size 0x200, verified (Bil_AuxCamera allocates 0x200 and
 * adds nothing). Start() saves the current view as mFrom, takes the new view as mTo and starts
 * mpEase; Calculate() interpolates while mActive. Fsh_Camera and the Bil cameras derive from
 * it.
 * @note Wii Play-only framework class; the name is a project convention. The ledger's
 * RP_Camera (clearLock) is this class's Reset.
 */
class RPInterpCamera : public RPGrpCamera {
public:
    RPInterpCamera();
    // Wii Play: inline (RPFshCamera's destructor destroys the arrays itself)
    virtual ~RPInterpCamera() {} // at 0x8

    /**
     * @brief Clears mActive [H]
     */
    virtual void Reset(); // at 0x10
    virtual void Calculate(); // at 0x14

    bool IsActive() const {
        return mActive;
    }

    void Start(s32 duration, const EGG::Vector3f& rPos, const EGG::Vector3f& rTarget,
               const EGG::Vector3f& rUp, s32 easeType);

    /**
     * @brief Up vector for a view from rPos to rTarget (world up unless looking straight
     * up/down) (ledger Fsh_calcUpVector, 801BDB38) [M]
     */
    static EGG::Vector3f CalcUpVector(const EGG::Vector3f& rPos, const EGG::Vector3f& rTarget);

protected:
    RPRange* mpEase;  // at 0x178
    /**
     * @brief Views as pos, target, up, dir (normalised target - pos), side (up x dir)
     */
    EGG::Vector3f mFrom[5]; // at 0x17C
    EGG::Vector3f mTo[5];   // at 0x1B8
    f32 mFromDist;    // at 0x1F4
    f32 mToDist;      // at 0x1F8
    bool mActive;     // at 0x1FC
};

//! @}

#endif
