#ifndef RP_PARTY_COMMON_LYT_OBJ_BASE_H
#define RP_PARTY_COMMON_LYT_OBJ_BASE_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
namespace EGG {
class Archive;
}
class RPSysLytResAccessor;

/**
 * @brief Base of the menu/game layout objects (one layout archive each)
 * @details vtable 803572F0 (abstract: Load is pure). Size 0x8 [H] (derived classes start their
 * members at 0x8; the Billiards layouts allocated with 0x14/0x24 add members). The constructor
 * creates the resource accessor on the default heap. There is no virtual destructor here: classes
 * that are deleted (RPFrameDown) add theirs after VF_0x18.
 * @note Wii Play-only framework class (ledger Lyt_LytObjBase); the name is a project convention.
 */
class RPLytObjBase {
public:
    RPLytObjBase();

    /**
     * @brief Mounts the named layout archive of pArchive on pAccessor (ledger Lyt_attachArcFile) [H]
     */
    static void AttachArcFile(EGG::Archive* pArchive, const char* pName,
                              RPSysLytResAccessor* pAccessor);

    /**
     * @brief Mounts the layout archive (a file inside pArchive) and builds the layout
     */
    virtual void Load(EGG::Archive* pArchive) = 0; // at 0x8
    virtual void Reset() {}                        // at 0xC
    virtual void Calculate() {}                    // at 0x10
    virtual void Draw() {}                         // at 0x14
    virtual void VF_0x18() {}                      // at 0x18

protected:
    RPSysLytResAccessor* mpResAccessor; // at 0x4
};

//! @}

#endif
