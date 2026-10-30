#ifndef RP_PARTY_COMMON_CLS_803572E0_H
#define RP_PARTY_COMMON_CLS_803572E0_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Singleton created by the Fsh, Cow and Bil scenes [L purpose]
 * @details vtable 803572E0 (destructor only). Size 0x8, verified (CreateInstance allocates 0x8
 * and only sets the vtable pointer). Instance at 8043C628.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPSysMessage;

class RPCls803572E0 {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPCls803572E0(); // at 0x8

    // Wii Play: Fishing reads its message data through it [M]
    static RPCls803572E0* GetInstance() {
        return sInstance;
    }
    RPSysMessage* GetMessage() const {
        return mpMessage;
    }
    /**
     * @brief Loads the named message file into mpMessage (ledger RP_loadMsgRes, 801B9544) [M]
     */
    void LoadMessage(const char* pName);

private:
    RPSysMessage* mpMessage; // at 0x4 [L] never initialised by CreateInstance

public:
    static RPCls803572E0* sInstance;
};

//! @}

#endif
