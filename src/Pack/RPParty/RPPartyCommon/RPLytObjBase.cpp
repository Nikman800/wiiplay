#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>

#include <Pack/RPKernel/RPSysLytResAccessor.h>

void RPLytObjBase::Load(EGG::Archive*) {}

RPLytObjBase::RPLytObjBase() {
    mpResAccessor = RPSysLytResAccessor::create(NULL);
}
