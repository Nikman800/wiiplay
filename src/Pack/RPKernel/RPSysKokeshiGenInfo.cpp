#include <Pack/RPKernel.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysKokeshiGenInfo.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <RVLFaceLib.h>

/**
 * @brief Constructor
 */
RPSysKokeshiGenInfo::RPSysKokeshiGenInfo() {
    mGenType = RPSysKokeshiManager::GenType_None;
    mPlayerNo = 0;

    mBodyType = 0;
    mHandType = RPSysKokeshiManager::HandType_Sphere;

    mViewNo = 1;
    mTypeOption = 0;
    mBufferOption = 0;

    mIconWidth = 128;
    mIconHeight = 128;

    mResolution = RFLResolution_128;
    mExprFlag = RFLExpFlag_Normal;

    mIconMipMap = false;
    mIconBGType = RFLIconBG_Favorite;
    mIconBGColor.r = mIconBGColor.g = mIconBGColor.b = mIconBGColor.a = 0;
    mIconView = RPSysKokeshiManager::IconView_Front;
    mIconExpression = RFLExp_Normal;
    mIconEdge = false;
    mIconEdgeColor.r = mIconEdgeColor.g = mIconEdgeColor.b = mIconEdgeColor.a =
        0;
    mIconEdgeWidth = 0;
}

/**
 * @brief Destructor
 */
RPSysKokeshiGenInfo::~RPSysKokeshiGenInfo() {}
