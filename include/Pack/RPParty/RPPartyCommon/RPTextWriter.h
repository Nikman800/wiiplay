#ifndef RP_PARTY_COMMON_TEXT_WRITER_H
#define RP_PARTY_COMMON_TEXT_WRITER_H
#include <Pack/types_pack.h>

#include <nw4r/ut.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Wide-character screen text writer using the system fonts
 * @details vtable 8034F678 (pointer last; nw4r::ut::TextWriterBase has no virtuals). Size 0x6C
 * [H] (constructed on the stack by the Duc/Cow/Bom screen passes and the debug log). The
 * constructor points nw4r::ut::TextWriterBase<wchar_t>'s format buffer (8043B998, size 8043AACC
 * = 0x100) at RPSysWideTextWriter::spTextBuffer (8043C4E8) and sets the default tag processor
 * (8043C4F8).
 * @note Name is a project convention. Very likely Wii Play's own RPSysWideTextWriter: its code
 * sits in the same unit right before Initialize__19RPSysWideTextWriterFv and uses that class's
 * buffer. Wii Play's version is not the Wii Sports singleton in RPKernel/RPSysWideTextWriter.h
 * (public constructor, extra BOOL, Begin/End), so it is kept as a separate class here.
 */
class RPTextWriter : public nw4r::ut::WideTextWriter {
public:
    RPTextWriter();

    /**
     * @brief Begin with the system font fontIdx (-1 = first font) [H]
     */
    void Begin(s32 fontIdx);
    /**
     * @brief Sets the font, GX state and an identity position matrix (Y flipped when the
     * screen is not in its default orientation) [H]
     */
    void Begin(const nw4r::ut::Font* pFont);
    void End();
    /**
     * @brief Prints the formatted text at (x, y) (y negated when the screen is not in its
     * default orientation), expanding message tags [H]
     * @param arg Passed through to RPSysTagProcessor::PreProcessEx
     */
    void Printf(f32 x, f32 y, const wchar_t* pFmt, int arg, ...);

private:
    BOOL mIsInBegin; // at 0x64

public:
    virtual ~RPTextWriter(); // at 0x8
};

//! @}

#endif
