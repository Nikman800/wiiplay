#ifndef RP_PARTY_COMMON_MODEL_OBJ_H
#define RP_PARTY_COMMON_MODEL_OBJ_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPPartyCommon/RPAxes.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief RPSceneObj owning one RPGrpModel, placed by position, scale and rotation
 * @details vtable 803571D0. Size 0x48 [H]: the constructor initialises every member up to
 * 0x48 and derived objects (Bom_StartBg 0x9C, Bom_Border 0x158) add their own after it.
 * The animation virtuals forward to the model's RPGrpModelAnm (mpModel->mpModelAnm) through
 * the RPGrpModelAnm slot of the same meaning.
 * @note Wii Play-only framework class (ledger RP_ModelObj); the name is a project convention.
 * The ledger's RP_ModelActor_applyTransform (slot 0x24) is this class's CalcModelMtx.
 */
class RPModelObj : public RPSceneObj {
public:
    RPModelObj();
    virtual ~RPModelObj(); // at 0x8

    // Defined out of line in RPModelObj.cpp (global symbols)
    virtual void Calculate(); // at 0x18
    virtual void Draw();      // at 0x20

    /**
     * @brief Builds the model matrix from mRot (each column scaled by mScale) and mPos, then
     * UpdateFrame/Calc/Entry on the model [H]
     */
    virtual void CalcModelMtx(); // at 0x24

    /**
     * @brief Finds or loads the model resource through RPResSlots (kind 1), then constructs
     * mpModel on view 1 with draw group sDrawGroup [H]
     */
    virtual void LoadModel(const char* pResName, u16 slot,
                           u32 option); // at 0x28

    /**
     * @brief Constructs mpModel from an already loaded resource handle [H]
     */
    virtual void CreateModel(RPGrpHandle handle); // at 0x2C

    /**
     * @brief RPGrpModelAnm::CreateBuffer on the model [H]
     */
    virtual void CreateAnmBuffer(RPGrpModelAnm::Anm anm, u16 num); // at 0x30

    /**
     * @brief Finds or loads an animation resource (resource kind 3/5/7/6 for anm 0/1/2/3)
     * and registers it through RegistAnm [H]
     */
    virtual void LoadAnm(const char* pResName, u16 slot,
                         RPGrpModelAnm::Anm anm, u16 idx); // at 0x34

    // Forwarders to mpModel->mpModelAnm [H]
    virtual void RegistAnm(RPGrpModelAnm::Anm anm, u16 idx,
                           RPGrpHandle handle); // at 0x38 (RPGrpModelAnm::Regist)
    virtual bool StartAnm(RPGrpModelAnm::Anm anm, u16 idx,
                          f32 frame); // at 0x3C (RPGrpModelAnm::Start)
    virtual bool StartAnmBlend(RPGrpModelAnm::Anm anm, u16 idx, f32 weight,
                               f32 frame); // at 0x40 (RPGrpModelAnm::StartBlend)
    virtual bool CancelAnm(RPGrpModelAnm::Anm anm,
                           u16 idx); // at 0x44 (RPGrpModelAnm::Cancel)
    virtual void SetAnmUpdateRate(RPGrpModelAnm::Anm anm, u16 idx,
                                  f32 rate); // at 0x48 (RPGrpModelAnm::SetUpdateRate)
    virtual f32 GetAnmFrame(RPGrpModelAnm::Anm anm,
                            u16 idx); // at 0x4C (RPGrpModelAnm::GetFrame)
    /**
     * @brief RPGrpModelAnm::GetNumFrame converted to f32 [H]
     */
    virtual f32 GetAnmNumFrame(RPGrpModelAnm::Anm anm, u16 idx); // at 0x50

    /**
     * @brief Stores unk40 and sets bit 0 of the model's flags (RPGrpModel::mFlags) [M]
     */
    virtual void SetUnk40(u8 value); // at 0x54

    /**
     * @brief Sets sDrawGroup, the draw group given to every model created afterwards [H]
     */
    static void SetDrawGroup(u32 drawGroup) {
        sDrawGroup = drawGroup;
    }

    const RPAxes& GetRot() const { return mRot; }

protected:
    nw4r::math::VEC3 mPos;   // at 0x4
public:
    nw4r::math::VEC3 mScale; // at 0x10
protected:
    RPAxes mRot;             // at 0x1C rows x, y, z
    u8 unk40;                // at 0x40 [L]
    RPGrpModel* mpModel;     // at 0x44

    // Not defined: the binary's draw group is the global RP_modelObjDrawGroup (8043C610,
    // RPModelObj.cpp), set by RP_ModelObj_setDrawGroup; SetDrawGroup above is unused.
    static u32 sDrawGroup;
};

//! @}

#endif
