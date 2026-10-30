#ifndef RP_PARTY_WLY_FIELD_4_H
#define RP_PARTY_WLY_FIELD_4_H

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

#include <nw4r/math.h>

// The spotlight field overlay derives directly from RPSimpleModel (its vtable holds
// RPSimpleModel's own LoadResource/LoadModel/VF_0x18). Its Load/Reset slots have the same ABI
// as the RPWlyObject view used by the field owner (RPWlyField.h).
class RPWlyField06 : public RPSimpleModel {
public:
    RPWlyField06();
    virtual ~RPWlyField06();
    virtual void Calculate();
    virtual void Load();
    virtual void Reset();

private:
    f32 mTexScale;                // at 0x44
    nw4r::math::MTX34 mTexMtx[2]; // at 0x48 per player
    bool mAnmStarted;             // at 0xA8
};

#endif
