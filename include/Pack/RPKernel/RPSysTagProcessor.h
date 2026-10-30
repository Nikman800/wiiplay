#ifndef RP_KERNEL_TAG_PROCESSOR_H
#define RP_KERNEL_TAG_PROCESSOR_H
#include <Pack/types_pack.h>

#include <Pack/RPSingleton.h>

#include <nw4r/ut.h>

//! @addtogroup rp_kernel
//! @{

/**
 * @brief Wide-char text tag processor
 * @details Wii Play: vtable 8034F6F0, size 0x4 (Lyt_createTagProcessor allocates 4 and stores the
 * instance at 8043C4F8; ledger Lyt_TagProcessor). Process handles ruby and colour tags
 * (colour table 80438C90); CalcRect is an empty stub.
 */
class RPSysTagProcessor : public nw4r::ut::WideTagProcessor {
    RP_SINGLETON_DECL(RPSysTagProcessor);

public:
    // Wii Play: overrides seen in vtable 8034F6F0
    virtual Operation Process(u16 ch, nw4r::ut::PrintContext<wchar_t>* pCtx); // at 0xC
    virtual Operation CalcRect(nw4r::ut::Rect* pRect, u16 ch,
                               nw4r::ut::PrintContext<wchar_t>* pCtx); // at 0x10

    void PreProcessEx(wchar_t const*, wchar_t*, u32, int*, int, va_list);

    void PreProcessEx(wchar_t const*, wchar_t*, u32, int*, int, ...);
};

//! @}

#endif
