#define IRPActorEnable IRPActorEnableUnit
#include "Pack/RPParty/RPPartyCommon/RPActor.h"
#undef IRPActorEnable

class __declspec(novtable) IRPActorEnable {
public:
    virtual bool IsEnabled(IRPActorPriority* pOther);
};

bool IRPActorEnable::IsEnabled(IRPActorPriority* pOther) {
    return true;
}
