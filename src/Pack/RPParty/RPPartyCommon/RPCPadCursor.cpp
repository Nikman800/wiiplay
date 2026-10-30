#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>

#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

void RPLytObjBase::AttachArcFile(EGG::Archive* pArchive, const char* pName,
                                  RPSysLytResAccessor* pAccessor) {
    void* pBinary =
        RPSysResourceManager::GetFileFromArchive(pArchive, pName, NULL);
    pAccessor->mountArchive(pBinary, NULL);
}

// NONMATCHING-DEBT: a global empty function precedes AttachArcFile; RPLytObjBase::VF_0x18 must stay inline in the header (an out-of-line definition would move the class vtable here), so the ledger name is likely wrong for this function.
extern "C" void VF_0x18__12RPLytObjBaseFv() {}
