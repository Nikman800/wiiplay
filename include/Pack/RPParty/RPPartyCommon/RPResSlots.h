#ifndef RP_PARTY_COMMON_RES_SLOTS_H
#define RP_PARTY_COMMON_RES_SLOTS_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPGrpModelResManager;

/**
 * @brief Scene-wide table of loaded model/animation resources
 * @details vtable 803571A8. Size 0x10, verified (RP_ResSlots_createInstance). Singleton
 * sInstance (8043C608), created by the Duc, Tnk and Bom scenes.
 * Sys_ResSlots_findOrLoad looks resources up here and returns their RPGrpModelResManager handle.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPResSlots : public RPSceneObj {
public:
    /**
     * @brief One loaded resource [M]
     */
    struct Slot {
        void* pFile;           // at 0x0 [L] the resource file from the common archive
        const char* pResName;  // at 0x4 [L]
        u32 kind;              // at 0x8 [L]
        u32 handle;            // at 0xC [L]
    };

public:
    static void CreateInstance();
    static void DestroyInstance();
    static RPResSlots* GetInstance() {
        return sInstance;
    }

    RPResSlots() : mpResManager(NULL), mSlotNum(0), mpSlots(NULL) {}
    virtual ~RPResSlots(); // at 0x8

    /**
     * @brief Creates the model resource list and a zeroed slot table of the given size [H]
     */
    void Init(u16 slotNum);

public:
    RPGrpModelResManager* mpResManager; // at 0x4
    u16 mSlotNum;                       // at 0x8
    Slot* mpSlots;                      // at 0xC
private:

    static RPResSlots* sInstance;
};

//! @}

#endif
