#ifndef RP_PARTY_TNK_FIELD_H
#define RP_PARTY_TNK_FIELD_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPGrpCaptureTexture;
class RPTnkActor;
class RPTnkBlockPool;
class RPTnkStageMap;
class RPTnkStepTimer;

/**
 * @brief Wall segment for the XZ circle tests (field border and block edges)
 * @details Size 0x20. First field wall from InitWalls: pos (w/2 + 8, 0, -d/2), normal
 * (0, 0, 1), length w + 16.
 */
struct RPTnkWall {
    nw4r::math::VEC3 mPos;    // at 0x0 segment start
    nw4r::math::VEC3 mNormal; // at 0xC
    f32 mLength;              // at 0x18
    u32 unk1C;                // at 0x1C [L] zero
};

/**
 * @brief Field floor models (tnk_field_16_9 / _4_3 and the "_out" border model)
 * @details vtable 8035B1C0 (IRPGrpModelCallback part at 0x48). Size 0x78, verified
 * (RPTnkField::Create). The floor's "caterpillar" texture is replaced by the field's capture
 * texture, so tread marks drawn into the capture show on the floor.
 */
/**
 * @brief Circle (radius) against one wall segment on the XZ plane: on a hit, the wall normal,
 * the penetration depth and the wall's unk1C (ledger Tnk_testCircleVsWall, RPTnkField_2.cpp)
 */
BOOL RPTnkTestCircleVsWall(f32 radius, nw4r::math::VEC3* pNormal, f32* pDepth, u32* pWallId,
                           const nw4r::math::VEC3* pPos, RPTnkWall* pWall);

class RPTnkFieldModel : public RPModelObj, public IRPGrpModelCallback {
public:
    RPTnkFieldModel() DECOMP_DONT_INLINE;
    virtual ~RPTnkFieldModel(); // at 0x8

    virtual void Create();       // at 0xC
    virtual void Load();         // at 0x10
    virtual void Reset();        // at 0x14 zero position, unit scale, identity rotation
    /**
     * @brief Sets the matrix of both models and updates/enters them [H]
     */
    virtual void CalcModelMtx(); // at 0x24

    /**
     * @brief IRPGrpModelCallback::EditWorldMtx override (empty)
     * @details Appends a primary slot (at 0x74) and a thunk in the callback part (slot 25).
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0x74

    void SetGridSize(f32 width, f32 depth, s32 cols, s32 rows) DECOMP_DONT_INLINE;

private:
    f32 mWidth;               // at 0x4C
    f32 mDepth;               // at 0x50
    f32 mCellWidth;           // at 0x54
    f32 mCellDepth;           // at 0x58
    s32 mCols;                // at 0x5C
    s32 mRows;                // at 0x60
    char unk64[0x6C - 0x64];
    RPGrpModel* mpFloorModel; // at 0x6C
    RPGrpModel* mpOutModel;   // at 0x70
    u32 unk74;                // at 0x74 [L] never accessed
};

/**
 * @brief Tanks! field: size and grid, border walls, blocks, stage maps and the tread-mark
 * capture texture (singleton Tnk_fieldInstance)
 * @details vtable 8035B198. Size 0xB4, verified (Tnk_Field_createInstance). Grid 16 x 17 cells
 * on 4:3 (560 x 595) or 22 x 17 on 16:9 (770 x 595); the cell size is also written to
 * Tnk_cellWidth / Tnk_cellDepth.
 */
class RPTnkField : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkField(); // at 0x8

    /**
     * @brief Creates the field model, block pool (256), stage map, capture texture and step
     * timer [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C destroys the block pool, stage map and step timer

    void InitWalls();
    void SetWorldScale(f32 scale);
    f32 GetWorldScale() const { return mWorldScale; }
    // Wii Play: RPTnkTank::InitOrientation (diagonals of the field)
    f32 GetAspect() const { return mDepth / mWidth; }
    void UpdateModels();
    /**
     * @brief Pushes an actor out of the four border walls, then out of the blocks; true when
     * anything was hit [H]
     */
    BOOL CollideActor(RPTnkActor* pActor, s32 mode);
    BOOL IsBlockAtPos(nw4r::math::VEC3 pos) const;

    static bool IsInRange(f32 value, f32 half) {
        bool inside = false;
        if (-half <= value && half >= value) {
            inside = true;
        }
        return inside;
    }
    // RPTnkWeaponManager::LayMine / FireShell [H]
    bool IsInside(nw4r::math::VEC3 pos) const {
        if (!IsInRange(pos.x, mWidth * 0.5f)) {
            return false;
        }
        return IsInRange(pos.z, mDepth * 0.5f);
    }

    static RPTnkField* sInstance; // Tnk_fieldInstance

    void DrawPlainQuad();
    void DrawClearQuad();
    void DrawCaptureTexture(s32 steps);
    void CaptureTexture();

    RPGrpCaptureTexture* GetCaptureTexture() const { return mpCaptureTex; }
    RPTnkBlockPool* GetBlockPool() const { return mpBlockPool; }
    s32 GetCols() const { return mCols; }
    s32 GetRows() const { return mRows; }
    /**
     * @brief World position of map cell (x, y), the grid centred on the origin [H]
     */
    nw4r::math::VEC3 GetCellPos(s32 x, s32 y) const {
        nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
        f32 cellDepth;
        f32 cellWidth;
        f32 originX;
        f32 originZ;
        cellWidth = mCellWidth;
        cellDepth = mCellDepth;
        originX = -0.5f * ((mCols - 1) * cellWidth);
        originZ = -0.5f * ((mRows - 1) * cellDepth);
        pos.x = cellWidth * x + originX;
        pos.z = cellDepth * y + originZ;
        return pos;
    }

    static f32 sCellWidth; // Tnk_cellWidth (35), set by Create
    static f32 sCellDepth; // Tnk_cellDepth (35)

private:
    f32 mWidth;                        // at 0x4
    f32 mDepth;                        // at 0x8
    s32 mCols;                         // at 0xC
    s32 mRows;                         // at 0x10
    f32 mCellWidth;                    // at 0x14
    f32 mCellDepth;                    // at 0x18
    RPTnkWall mWalls[4];               // at 0x1C
    RPTnkFieldModel* mpModel;          // at 0x9C
    RPTnkBlockPool* mpBlockPool;       // at 0xA0
    RPTnkStageMap* mpStageMap;         // at 0xA4
    f32 mWorldScale;                   // at 0xA8 reset to 1, Scene::Reset sets 0.7466
    RPGrpCaptureTexture* mpCaptureTex; // at 0xAC 0x10 bytes (ledger Gfx_CapTexture)
    RPTnkStepTimer* mpStepTimer;       // at 0xB0
};

//! @}

#endif
