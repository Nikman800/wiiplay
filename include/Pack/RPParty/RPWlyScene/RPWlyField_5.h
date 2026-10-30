#ifndef RP_PARTY_WLY_FIELD_5_H
#define RP_PARTY_WLY_FIELD_5_H

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

// Screen wipe at the start of each stage (wtd_wipe_a). StartClose is defined out of line in
// RPWlyField_5.cpp (the binary has it as a normal function, the first of the unit).
class RPWlyWipe : public RPSimpleModel {
public:
    RPWlyWipe();
    virtual ~RPWlyWipe();
    virtual void Calculate();

    void Create();
    void Init();
    void Reset();
    void Destroy();
    void StartClose();

private:
    bool mCloseRequested; // at 0x44
public:
    bool mDone;           // at 0x45
};

#endif
