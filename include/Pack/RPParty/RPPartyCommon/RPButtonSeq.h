#ifndef RP_PARTY_COMMON_BUTTON_SEQ_H
#define RP_PARTY_COMMON_BUTTON_SEQ_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Detects a button sequence (zero-terminated u16 button masks) on one controller
 * @details No vtable. Size 0xC, verified (RPButtonSeqHolder allocates one). Each frame with a
 * button pressed advances on a match and restarts on a miss.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPButtonSeq {
public:
    RPButtonSeq(s32 channel, const u16* pSeq);

    void Update();
    /**
     * @brief The current element is the terminator [H]
     */
    bool IsComplete() const;

private:
    s32 mChannel;      // at 0x0
    const u16* mpSeq;  // at 0x4
    s32 mIndex;        // at 0x8
};

/**
 * @brief Owns one RPButtonSeq on channel 0 with sequence table 80318818
 * @details vtable 80357738. Size 0x8, verified (Lyt_CommonMessage_setup).
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPButtonSeqHolder {
public:
    RPButtonSeqHolder();

    void Update();
    bool IsComplete() const;

private:
    RPButtonSeq* mpSeq; // at 0x0

public:
    /**
     * @brief Does not delete mpSeq [H]
     */
    virtual ~RPButtonSeqHolder(); // at 0x8

private:
    static const u16 sSequence[]; // 80318818
};

//! @}

#endif
