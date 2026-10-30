#include <Pack/RPGraphics.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPGraphics/RPGrpModelMisc.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <egg/core.h>

/**
 * @brief File extension for RPGrpModelMisc binaries
 */
template <> const char* EGG::IBinary<RPGrpModelMisc>::spExtension = "pmmisc";

/**
 * @brief Gets the binary signature of this object
 */
const char* RPGrpModelMisc::GetBinaryType() const {
    return "MMSC";
}
