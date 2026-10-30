#include <Pack/RPKernel.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysMiddleDBGenInfo.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <RVLFaceLib.h>

/**
 * @brief Constructor
 */
RPSysMiddleDBGenInfo::RPSysMiddleDBGenInfo()
    : mType(RFLMiddleDBType_Random),
      mSize(0),
      mGenderMask(RFLSex_All),
      mAgeMask(RFLAge_All),
      mRaceMask(RFLRace_All),
      mpMiiManager(NULL) {}

/**
 * @brief Destructor
 */
RPSysMiddleDBGenInfo::~RPSysMiddleDBGenInfo() {}
