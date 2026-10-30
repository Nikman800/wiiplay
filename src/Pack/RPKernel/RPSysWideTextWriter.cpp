#include <Pack/RPKernel.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysWideTextWriter.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

/**
 * @brief Work buffer for string formatting
 */
wchar_t* RPSysWideTextWriter::spTextBuffer = NULL;

/**
 * @brief Creates the text buffer
 */
void RPSysWideTextWriter::Initialize() {
    spTextBuffer = new wchar_t[TEXT_BUFFER_SIZE];
}
