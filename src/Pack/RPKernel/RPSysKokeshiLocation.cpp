#include <Pack/RPKernel.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysKokeshiLocation.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <RVLFaceLib.h>


/**
 * @brief Constructor
 */
RPSysKokeshiLocation::RPSysKokeshiLocation()
    : mDataSource(RFLDataSource_Official), mIndex(0), mpMiddleDB(NULL) {}

/**
 * @brief Destructor
 */
RPSysKokeshiLocation::~RPSysKokeshiLocation() {}
