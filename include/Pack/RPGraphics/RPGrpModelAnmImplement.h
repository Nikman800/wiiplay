#ifndef RP_GRAPHICS_MODEL_ANM_IMPLEMENT_H
#define RP_GRAPHICS_MODEL_ANM_IMPLEMENT_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>

#include <nw4r/g3d.h>

//! @addtogroup rp_graphics
//! @{

/******************************************************************************
 *
 * RPGrpModelAnmImplement
 *
 ******************************************************************************/

/**
 * @brief Model animation implementation (Wii Play: only Kind_G3D is used)
 *
 * Wii Play: a class template; RPGrpModelAnmImplement.cpp explicitly specializes most members for
 * Kind_G3D and leaves six as template definitions. Those are implicitly instantiated after the
 * file's normal functions and its vtable, in the order they are declared here (InternalGetAnmObj,
 * SetAnmObj, Init, the destructor, both RegistFromResFile), which puts the vtable between the
 * jump tables of the normal functions and theirs, as in the binary.
 */
template <RPGrpModel::Kind K>
class RPGrpModelAnmImplement : public RPGrpModelAnm {
public:
    // Wii Play: these six are template definitions in RPGrpModelAnmImplement.cpp, instantiated in
    // this declaration order (see above); the virtual slots come from RPGrpModelAnm.
    nw4r::g3d::AnmObj* InternalGetAnmObj(Anm anm, AnmIdx idx) const;

private:
    virtual void SetAnmObj(Anm anm,
                           nw4r::g3d::AnmObj** ppAnmObjs) override; // at 0x80

public:
    void Init();

private:
    virtual ~RPGrpModelAnmImplement() override; // at 0x7C

public:
    virtual void RegistFromResFile(Anm anm, AnmIdx idx, RPGrpHandle handle,
                                   u32 id) override; // at 0x14
    virtual void RegistFromResFile(Anm anm, AnmIdx idx, RPGrpHandle handle,
                                   const char* pName, u32 arg4 = 0) override; // at 0x10

    // Wii Play: defined out of line at the top of RPGrpModelImplement.cpp (a normal function: it
    // comes after the constructors there, not after its caller CreateAnm as a weak copy would)
    static RPGrpModelAnm* Construct(RPGrpModel* pModel) DECOMP_DONT_INLINE;

    virtual void Configure() override; // at 0x8

    virtual void Regist(Anm anm, AnmIdx idx, RPGrpHandle handle,
                        u32 arg3 = 0) override; // at 0xC

    virtual s16 SetBlendAnm(Anm anm, u8, AnmIdx idx) override; // at 0x18
    virtual s16 RemoveBlendAnm(Anm anm, u8, u8) override;      // at 0x1C

    virtual void SetBlendWeight(Anm anm, u8, f32, u8) override; // at 0x20
    virtual s16 GetBlendAnm(Anm, u8, u8) override;              // at 0x24
    virtual f32 GetBlendWeight(Anm, u8, u8) override;           // at 0x28

    virtual void ReBindChrAnmNode(u16, u16, BindType type);           // at 0x2C
    virtual void ReleaseChrAnmNode(u16, u16, BindType type) override; // at 0x30

    // Wii Play: StartNoBlend/StartBlend (0x34, 0x38) are RPGrpModelAnm's

    virtual bool Cancel(Anm anm, AnmIdx idx) override; // at 0x3C
    virtual bool Start(Anm anm, AnmIdx idx,
                       f32 frame = 0.0f) override; // at 0x40

    virtual f32 VF_0x44(UNKTYPE) override;
    virtual f32 VF_0x48(UNKTYPE) override;

    virtual int GetNumFrame(Anm anm, AnmIdx idx) const override; // at 0x4C

    virtual AnmPlayPolicy
    GetAnmPlayPolicy(Anm anm,
                     AnmIdx idx) const override; // at 0x50

    virtual void SetFrame(Anm anm, AnmIdx idx, f32 frame) override; // at 0x54
    virtual f32 GetFrame(Anm anm, AnmIdx idx) const override;       // at 0x58

    virtual void SetUpdateRate(Anm anm, AnmIdx idx,
                               f32 rate) override;                 // at 0x5C
    virtual f32 GetUpdateRate(Anm anm, AnmIdx idx) const override; // at 0x60

    virtual void SetEnableQuaternionBlend(bool enable) override; // at 0x64

    virtual void SetEnableChrAnm(bool enable) override; // at 0x68
    virtual void UpdateFrame() override;                // at 0x6C

    virtual void VF_0x70(UNKTYPE) override; // at 0x70
    virtual void VF_0x74(UNKTYPE) override; // at 0x74

    virtual nw4r::g3d::AnmObj* GetAnmObj(Anm anm,
                                         AnmIdx idx) const override; // at 0x78

    void InternalSetAnmObj(Anm anm);

    bool IsPlayingG3D(Anm anm, u16 idx) const {
        return mpAnmFlags[anm][idx] & AnmFlag_PlayingG3D;
    }

private:
    struct ChrBlendInfo {
        nw4r::g3d::AnmObjChrBlend* pAnmObj; // at 0x0
        s16 toAnmIdx[8];                    // at 0x4
    };

    struct ShpBlendInfo {
        nw4r::g3d::AnmObjShpBlend* pAnmObj; // at 0x0
        s16 toAnmIdx[8];                    // at 0x4
    };

private:
    explicit RPGrpModelAnmImplement(RPGrpModel* pModel);
    virtual void InternalCalc() override;                           // at 0x84

    void RegistAnmChr(AnmIdx idx, const nw4r::g3d::ResAnmChr chr);

    void RegistAnmClr(AnmIdx idx, const nw4r::g3d::ResAnmClr clr);

    void RegistAnmTexPat(AnmIdx idx, const nw4r::g3d::ResAnmTexPat pat);

    void RegistAnmTexSrt(AnmIdx idx, const nw4r::g3d::ResAnmTexSrt srt);

    void RegistAnmVis(AnmIdx idx, const nw4r::g3d::ResAnmVis vis);

    void RegistAnmShp(AnmIdx idx, const nw4r::g3d::ResAnmShp shp);

private:
    //! Animation object type conversion table
    // Wii Play: six entries (Anm_Chr..Anm_Shp)
    static const nw4r::g3d::ScnMdlSimple::AnmObjType TO_G3D_TYPE[];
    //! Animation play policy conversion table
    static const AnmPlayPolicy FROM_G3D_POLICY[];

    //! Scene model that this animation controls
    nw4r::g3d::ScnMdlSimple* mpScnMdlSimple; // at 0x6C

    nw4r::g3d::AnmObjChrRes** mppAnmObjChrRes;       // at 0x70
    nw4r::g3d::AnmObjMatClrRes** mppAnmObjMatClrRes; // at 0x74
    nw4r::g3d::AnmObjTexPatRes** mppAnmObjTexPatRes; // at 0x78
    nw4r::g3d::AnmObjTexSrtRes** mppAnmObjTexSrtRes; // at 0x7C
    nw4r::g3d::AnmObjVisRes** mppAnmObjVisRes;       // at 0x80
    nw4r::g3d::AnmObjShpRes** mppAnmObjShpRes;       // at 0x84

    nw4r::g3d::AnmObjVisRes** unk88;
    nw4r::g3d::AnmObjVisRes** unk8C;
    u32 unk90[Anm_Max];

    //! Map from RP animation index to G3D index
    s8* mpToIndexG3D[Anm_Max]; // at 0xB0

    nw4r::g3d::AnmObjMatClrOverride* mpAnmObjMatClrOverride; // at 0xD0
    nw4r::g3d::AnmObjTexPatOverride* mpAnmObjTexPatOverride; // at 0xD4
    nw4r::g3d::AnmObjTexSrtOverride* mpAnmObjTexSrtOverride; // at 0xD8
    nw4r::g3d::AnmObjVisOR* mpAnmObjVisOR;                   // at 0xDC

    ChrBlendInfo* mpChrBlendInfo; // at 0xE0
    ShpBlendInfo* mpShpBlendInfo; // at 0xE4
};

/******************************************************************************
 *
 * RPGrpModelAnmG3D
 *
 ******************************************************************************/

// Shorthand name for specialization
typedef RPGrpModelAnmImplement<RPGrpModel::Kind_G3D> RPGrpModelAnmG3D;

//! @}

#endif
