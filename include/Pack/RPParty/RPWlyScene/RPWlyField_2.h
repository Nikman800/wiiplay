#ifndef RP_PARTY_WLY_FIELD_2_H
#define RP_PARTY_WLY_FIELD_2_H

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

// The earth model derives directly from RPSimpleModel (its vtable holds RPSimpleModel's own
// LoadResource/LoadModel/VF_0x18). Its Load/Reset slots have the same ABI as the RPWlyObject
// view used by the field owner (RPWlyField.h).
class RPWlyEarth : public RPSimpleModel {
public:
    RPWlyEarth();
    virtual ~RPWlyEarth();
    virtual void Calculate();
    virtual void Load();
    virtual void Reset();
};

#endif
