#ifndef RP_PARTY_COMMON_RES_FILE_H
#define RP_PARTY_COMMON_RES_FILE_H
#include <Pack/types_pack.h>

#include <egg/core.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Relocated table resource with an entry-type filter
 * @details No vtable. Size 0x8, verified (Create allocates 0x8 from the given heap, align 4).
 * Entries are 0x58 bytes at data+0x40 (count u16 at data+0x10); an entry matches the filter
 * when its type byte (entry+0x4) is 0 or equals mFilter.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPResFile {
public:
    /**
     * @brief Allocates the wrapper (mFilter 0) and relocates the data in place [H]
     */
    static RPResFile* Create(void* pData, EGG::Heap* pHeap);
    /**
     * @brief Turns the data's offsets into pointers, once (flag bit 0) [H]
     */
    void Relocate();

    /**
     * @brief Counts the entries matching mFilter (ledger RP_ResTable_countFilteredEntries) [H]
     */
    int CountFilteredEntries() const;
    /**
     * @brief The idx-th entry matching mFilter, or NULL (ledger RP_ResTable_getFilteredEntry) [H]
     */
    void* GetFilteredEntry(u16 idx) const;

private:
    RPResFile(void* pData) : mpData(pData), mFilter(0) {}

    // Inlined copy of GetFilteredEntry used by Relocate (RPResFile_3)
    inline void* FindFilteredEntry(u16 idx) const;

private:
    void* mpData; // at 0x0
    u16 mFilter;  // at 0x4
};

/**
 * @brief Holder of the RPResFile loaded from the scene's common archive
 * @details No vtable. Size 0x4 [L] (only mpFile is accessed).
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPResTable {
public:
    /**
     * @brief Loads pResName from the scene common archive into an RPResFile on the current
     * heap [H]
     */
    explicit RPResTable(const char* pResName);
    u16 CountEntriesOfType(u32 type) const;            // [M] type compared as a whole word
    void* FindNthEntryOfType(u32 type, u16 idx) const; // [M] type compared as a whole word

private:
    RPResFile* mpFile; // at 0x0
};

//! @}

#endif
