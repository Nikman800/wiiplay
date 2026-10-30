#ifndef RP_PARTY_WLY_FIELD_3_H
#define RP_PARTY_WLY_FIELD_3_H

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

// The ground model derives directly from RPSimpleModel. Its Load/Reset slots
// have the same ABI as the RPWlyObject view used by the field owner.
class RPWlyGround : public RPSimpleModel {
public:
    RPWlyGround();
    virtual ~RPWlyGround();
    virtual void Calculate();
    virtual void Load();
    virtual void Reset();

private:
    s32 mFrame;
    f32 unk48;
    f32 unk4C;
    s32 unk50;
};

#endif
