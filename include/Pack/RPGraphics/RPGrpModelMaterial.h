#ifndef RP_GRAPHICS_MODEL_MATERIAL_H
#define RP_GRAPHICS_MODEL_MATERIAL_H
#include <Pack/types_pack.h>

#include <nw4r/g3d.h>

#include <revolution/GX.h>

//! @addtogroup rp_graphics
//! @{

// Forward declarations
class RPGrpModel;

/**
 * @brief Model material setting
 */
class RPGrpModelMaterial {
public:
    enum ESyncFlag {
        ESyncFlag_ResGenMode = 1 << 0,
        ESyncFlag_ResMatTev = 1 << 4,
        // Wii Play
        ESyncFlag_ChanCtrl = 1 << 1,
        ESyncFlag_ResMatPix = 1 << 3,
        ESyncFlag_EffectMtx = 1 << 9,
        ESyncFlag_ChanColor = 1 << 11,
        ESyncFlag_TevSwap = 1 << 12,
    };

    enum ETevColor {
        ETevColor_Reg0,
        ETevColor_Reg1,
        ETevColor_Reg2,

        ETevColor_Max
    };

    enum ETevKColor {
        ETevKColor_Reg0,
        ETevKColor_Reg1,
        ETevKColor_Reg2,
        ETevKColor_Reg3,

        ETevKColor_Max
    };

    struct ChanCtrl {
        u8 enable;    // at 0x0
        u8 lightMask; // at 0x1
        u8 ambSrc;    // at 0x2
        u8 matSrc;    // at 0x3
        u8 diffuseFn; // at 0x4
        u8 attnFn;    // at 0x5
        u8 unk6[2];   // Wii Play: size 8 (copied whole, never written)
    };

    /**
     * @brief Material/ambient colours of one channel pair (Wii Play)
     */
    struct ChanColor {
        GXColor amb; // at 0x0
        GXColor mat; // at 0x4
    };

    /**
     * @brief Pixel engine settings (Wii Play)
     */
    struct PixMode {
        u8 comp0;          // at 0x0
        u8 comp1;          // at 0x1
        u8 ref0;           // at 0x2
        u8 ref1;           // at 0x3
        u8 alphaOp;        // at 0x4
        bool zTest;        // at 0x5
        bool zUpdate;      // at 0x6
        u8 zComp;          // at 0x7
        u8 blendMode;      // at 0x8
        u8 srcFactor;      // at 0x9
        u8 dstFactor;      // at 0xA
        u8 logicOp;        // at 0xB
        bool dstAlphaTest; // at 0xC
        u8 dstAlpha;       // at 0xD
        u8 padE[2];
    };

private:
    u32 mSyncFlags; // at 0x0
    u16 unk4;
    u8 unk6;
    u8 mNumChans;     // at 0x7
    u8 mNumTexGens;   // at 0x8
    u8 mNumTevStages; // at 0x9
    u8 mNumIndStages; // at 0xA
    u8 mCullMode;     // at 0xB
    char unkC[4];
    ChanCtrl* mpChanCtrls[GX_ALPHA1 - GX_COLOR0 + 1]; // at 0x10
    GXColor mTevKColors[ETevKColor_Max];              // at 0x20
    GXColorS10 mTevColors[ETevColor_Max];             // at 0x30
    // Wii Play: members below filled in from the constructor and Calc
    PixMode mPixMode;                                    // at 0x48
    nw4r::math::MTX34* mpEffectMtx[8];                   // at 0x58 the resource's effect matrices
    u8* mpTevStageBuf;                                   // at 0x78
    ChanColor* mpChanColors[2];                          // at 0x7C
    u8 mSwapTable[4][4];                                 // at 0x84
    char unk94[4];
    nw4r::g3d::ResMat mResMat;                           // at 0x98
    nw4r::g3d::ScnMdl::CopiedMatAccess mCopiedMatAccess; // at 0x9C

    // Wii Play: a ScnMdl's copied material with the resource as fallback
    nw4r::g3d::ResGenMode GetGenMode();
    nw4r::g3d::ResMatChan GetMatChan();
    nw4r::g3d::ResMatPix GetMatPix();
    nw4r::g3d::ResMatTevColor GetMatTevColor();
    nw4r::g3d::ResTev GetTev();
    nw4r::g3d::ResTexSrt GetTexSrt();
    void EndEditMatChan();

    bool GetChanCtrl(u8 chan, ChanCtrl* pCtrl);
    bool GetChanColor(u8 idx, ChanColor* pColor);
    void GetPixMode(PixMode* pMode);

    void ReadTevColors();
    void ReadTevSwap();
    void ReadEffectMtx();

    void SyncGenMode();
    void SyncChanCtrl(u8 chan, const ChanCtrl& rCtrl);
    void SyncChanColor();
    void SyncPixMode();
    void SyncTevColor();
    void SyncTevSwap();
    void SyncEffectMtx();

public:
    RPGrpModelMaterial(RPGrpModel* pModel, u16 idx);
    virtual ~RPGrpModelMaterial(); // at 0x8

    void Calc();

    // Wii Play: RPWlySpotlight writes the spotlight's texture matrices in place
    nw4r::math::MTX34* GetEffectMtx(int idx) const {
        return mpEffectMtx[idx];
    }
    void SetSyncFlags(u32 flags) {
        mSyncFlags |= flags;
    }

    u8 GetCullMode() const {
        return mCullMode;
    }
    void SetCullMode(GXCullMode cullMode) {
        mCullMode = cullMode;
    }

    void SetTevKColor(ETevKColor idx, GXColor color) {
        mTevKColors[idx] = color;
    }
    void SetTevKColor(ETevKColor idx, u8 r, u8 g, u8 b) {
        mTevKColors[idx].r = r;
        mTevKColors[idx].g = g;
        mTevKColors[idx].b = b;
    }

    GXColorS10& GetTevColor(ETevColor idx) {
        return mTevColors[idx];
    }
    void SetTevColor(ETevColor idx, GXColorS10 color) {
        mTevColors[idx] = color;
    }
    void SetTevColor(int idx, const GXColor& color) {
        // Wii Play: addressed through the TEV color block at 0x20
        // (RPSysKokeshiManager::SetMatColor)
        struct TevColorBlock {
            GXColor kcolor[ETevKColor_Max];  // at 0x0
            GXColorS10 color[ETevColor_Max]; // at 0x10
        };
        TevColorBlock* pBlock = reinterpret_cast<TevColorBlock*>(mTevKColors);

        pBlock->color[idx].r = color.r;
        pBlock->color[idx].g = color.g;
        pBlock->color[idx].b = color.b;
        pBlock->color[idx].a = color.a;
    }
    void SetTevColor(ETevColor idx, u8 r, u8 g, u8 b, u8 a) {
        mTevColors[idx].r = r;
        mTevColors[idx].g = g;
        mTevColors[idx].b = b;
        mTevColors[idx].a = a;
    }
    void SetTevColor(ETevColor idx, u8 r, u8 g, u8 b) {
        mTevColors[idx].r = r;
        mTevColors[idx].g = g;
        mTevColors[idx].b = b;
    }
    void SetTevColorAlpha(ETevColor idx, u8 alpha) {
        mTevColors[idx].a = alpha;
    }
};

//! @}

#endif
