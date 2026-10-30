#ifndef RP_PARTY_BIL_TABLE_H
#define RP_PARTY_BIL_TABLE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>

#include <egg/math.h>

#include <nw4r/math.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
class RPGrpModel;
class RPGrpShadowTexture;
class RPJointMtxBuffer;
class RPKcl;

class RPBilTable;
// Explicit ABI entry for the by-value GetPocketPos return; the public API stays below.
extern "C" void GetPocketPos__10RPBilTableCFl(nw4r::math::VEC3* pOut, RPBilTable* pTable,
                                             s32 pocket);

/**
 * @brief Billiards table: model, collision, pocket positions and the aiming guide line
 * (singleton RPBilTable::sInstance)
 * @details vtable 8035DCA8. Size 0xB8, verified (Bil_Table_create). Load reads table.kcl into
 * mpKcl, builds the bad_table model (mpModel) and the bad_mask model, reads the joints
 * ball0_range, pocket_corner and pocket_side into the three positions and creates the table's
 * 0x200-byte shadow texture. Pockets 0-3 are the corners (signs of the corner position), 1 and
 * 4 the side pockets.
 */
// Wii Play: derives from RPActorBase directly (RPBilActor's implicit destructor would add a
// null check to the destructor)
class RPBilTable : public RPActorBase {
public:
    static RPBilTable* sInstance; // 8043C9B8

    RPBilTable();
    virtual ~RPBilTable(); // at 0x8

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84

    /**
     * @brief IRPGrpDrawObject::UserDraw override: the guide line (and debug lines) [M]
     */
    virtual void UserDraw(); // at 0x88

    /**
     * @brief Position of the given pocket (ledger Bil_Table_getPocketPos) [M]
     */
    EGG::Vector3f GetPocketPos(s32 pocket) const;
    // Phase 3 form, still used by RPBilCamera_4.cpp; the binary's function returns by value
    // (GetPocketPos(s32) above) and this one is not defined
    void GetPocketPos(nw4r::math::VEC3* pPos, s32 pocket) const;
    /**
     * @brief Direction into the given pocket (ledger Bil_getPocketDirection; called without
     * this) [M]
     */
    static EGG::Vector3f GetPocketDirection(s32 pocket);
    /**
     * @brief Whether a ball of the given radius at the position lies inside ball0_range
     * (around sCenter) [H]
     */
    bool IsInsideBallRange(f32 radius, const EGG::Vector3f& rPos) const;
    void HideGuideLine();
    /**
     * @brief Shows the guide line from rStart along rDir (y flattened), clipped at the
     * cushions [H]
     */
    void SetGuideLine(const EGG::Vector3f& rStart, const EGG::Vector3f& rDir);
    /**
     * @brief Sphere sweep from pSegment[0] to pSegment[1] against table.kcl, in steps of 0.5
     * (a point test when radius is 0) (Bil_Table_sweepHitsKcl) [M]
     */
    bool SweepHitsKcl(const EGG::Vector3f* pSegment, f32 radius);

    /**
     * @brief Height of the table surface (ledger Bil_getTableHeight; called without this) [M]
     */
    static f32 GetHeight();
    /**
     * @brief Origin of the ball rack, (50, 0, 0) (ledger Bil_initRackOrigin) [M]
     */
    static EGG::Vector3f GetRackOrigin();
    /**
     * @brief 3.57, read by the ball manager's table collisions (ledger Bil_getConst3_57) [L]
     */
    static f32 GetCushionParam();

    RPKcl* GetKcl() const {
        return mpKcl;
    }
    RPGrpShadowTexture* GetShadowTex() const {
        return mpShadowTex;
    }
    /**
     * @brief sCenter (ledger Bil_getZeroVec) [M]
     */
    static const EGG::Vector3f& GetCenter();

    bool GetUnk80() const {
        return unk80;
    }
    void SetUnk80(bool value) {
        unk80 = value;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    friend void GetPocketPos__10RPBilTableCFl(nw4r::math::VEC3*, RPBilTable*, s32);

    bool mShowGuideLine;                // at 0x60
    EGG::Vector3f mGuideStart;          // at 0x64 y replaced by -0.3
    EGG::Vector3f mGuideDir;            // at 0x70 normalised, y = 0
    f32 mGuideLength;                   // at 0x7C distance to the nearer cushion
    bool unk80;                         // at 0x80 [L] cleared by Reset and UserDraw
    RPJointMtxBuffer* mpJointMtx;       // at 0x84 joints of the table model
    nw4r::math::VEC3 mBallRange;        // at 0x88 ball0_range (half extents x / z)
    nw4r::math::VEC3 mPocketCorner;     // at 0x94 pocket_corner
    nw4r::math::VEC3 mPocketSide;       // at 0xA0 pocket_side
    RPGrpShadowTexture* mpShadowTex;    // at 0xAC
    RPKcl* mpKcl;                       // at 0xB0 table.kcl
    RPGrpModel* mpMaskModel;            // at 0xB4 bad_mask

    /**
     * @brief Table centre, a copy of EGG::Vector3f::zero (ledger Bil_zeroVec, 8043A944) [M]
     */
    static EGG::Vector3f sCenter;

public:
    static u8 sGuideLineWidth;
    static f32 sGuideLineY;
    static GXColor sGuideLineColors;
};

//! @}

#endif
