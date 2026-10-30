// Ported from doldecomp/ogws (CC0) src/Pack/RPGraphics/RPGrpDrawPathManager.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".
// Wii Play: the draw path classes are declared in this order: their weak vtables come out in
// reverse declaration order (XluSnap first), their weak destructors (declared in the headers) in
// declaration order after ~RPGrpDrawPathManager, then RPGrpDrawPath::On/Off
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpDrawPathSV.h>
#include <Pack/RPGraphics/RPGrpDrawPathHDR.h>
#include <Pack/RPGraphics/RPGrpDrawPathDOF.h>
#include <Pack/RPGraphics/RPGrpDrawPathBloom.h>
#include <Pack/RPGraphics/RPGrpDrawPathXluSnap.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPPartyCommon/RPScnProcDrawHook.h>

// Wii Play: nothing in this file is auto-inlined (the Construct functions and the constructor
// are called, not inlined)
#pragma auto_inline off

// Wii Play: scene renderer timing of each proc
const u32 RPGrpDrawPathManager::PROC_TIMINGS[] = {
    EGG::ScnRenderer::cTiming_Opa4th, EGG::ScnRenderer::cTiming_Xlu1st,
    EGG::ScnRenderer::cTiming_Opa1st, EGG::ScnRenderer::cTiming_Opa2nd,
    EGG::ScnRenderer::cTiming_Opa3rd, EGG::ScnRenderer::cTiming_Xlu2nd,
};

RPGrpDrawPathManager*
RPGrpDrawPathManager::Construct(u32 pathFlags, RPGrpModelScene* pModelScene) {

    return new RPGrpDrawPathManager(pathFlags, pModelScene);
}

template <>
RPGrpDrawPathShadowTexture*
RPGrpDrawPath::Construct<RPGrpDrawPathShadowTexture>(
    EGG::ScnRenderer* pRenderer) {

    return new RPGrpDrawPathShadowTexture(pRenderer);
}

template <>
RPGrpDrawPathLightMap*
RPGrpDrawPath::Construct<RPGrpDrawPathLightMap>(EGG::ScnRenderer* pRenderer) {

    return new RPGrpDrawPathLightMap(pRenderer);
}

template <>
RPGrpDrawPathXluSnap*
RPGrpDrawPath::Construct<RPGrpDrawPathXluSnap>(EGG::ScnRenderer* pRenderer) {

    return new RPGrpDrawPathXluSnap(pRenderer);
}

template <>
RPGrpDrawPathBloom*
RPGrpDrawPath::Construct<RPGrpDrawPathBloom>(EGG::ScnRenderer* pRenderer) {

    return new RPGrpDrawPathBloom(pRenderer);
}

template <>
RPGrpDrawPathDOF*
RPGrpDrawPath::Construct<RPGrpDrawPathDOF>(EGG::ScnRenderer* pRenderer) {

    return new RPGrpDrawPathDOF(pRenderer);
}

template <>
RPGrpDrawPathHDR*
RPGrpDrawPath::Construct<RPGrpDrawPathHDR>(EGG::ScnRenderer* pRenderer) {

    return new RPGrpDrawPathHDR(pRenderer);
}

template <>
RPGrpDrawPathSV*
RPGrpDrawPath::Construct<RPGrpDrawPathSV>(EGG::ScnRenderer* pRenderer) {

    return new RPGrpDrawPathSV(pRenderer);
}

RPGrpDrawPathManager::RPGrpDrawPathManager(u32 pathFlags,
                                           RPGrpModelScene* pModelScene)
    : unkC(1),
      mpModelScene(pModelScene),
      mpDrawHook(NULL) {

    EGG::ScnRenderer* pRenderer = pModelScene->GetScnRenderer();

    mpDrawPathLightMap = NULL;
    mpDrawPathShadowTexture = NULL;
    mpDrawPathXluSnap = NULL;
    mpDrawPathSV = NULL;
    mpDrawPathHDR = NULL;
    mpDrawPathBloom = NULL;
    mpDrawPathDOF = NULL;

    if (pathFlags & EFlag_DrawPathSV) {
        mpDrawPathSV = RPGrpDrawPath::Construct<RPGrpDrawPathSV>(pRenderer);
    }

    if (pathFlags & EFlag_DrawPathHDR) {
        mpDrawPathHDR = RPGrpDrawPath::Construct<RPGrpDrawPathHDR>(pRenderer);
    }

    if (pathFlags & EFlag_DrawPathDOF) {
        mpDrawPathDOF = RPGrpDrawPath::Construct<RPGrpDrawPathDOF>(pRenderer);
    }

    if (pathFlags & EFlag_DrawPathBloom) {
        mpDrawPathBloom =
            RPGrpDrawPath::Construct<RPGrpDrawPathBloom>(pRenderer);

        // Wii Play: the shadow volume path is told that bloom is on
        if (pRenderer->getDrawPathBase(EGG::ScnRenderer::cDrawPath_SV) !=
            NULL) {
            static_cast<EGG::DrawPathShadowVolume*>(
                pRenderer->getDrawPathBase(EGG::ScnRenderer::cDrawPath_SV))
                ->OnSVFlag(2);
        }
    }

    if (pathFlags & EFlag_DrawPathXluSnap) {
        mpDrawPathXluSnap =
            RPGrpDrawPath::Construct<RPGrpDrawPathXluSnap>(pRenderer);
    }

    if (pathFlags & EFlag_DrawPathLightMap) {
        mpDrawPathLightMap =
            RPGrpDrawPath::Construct<RPGrpDrawPathLightMap>(pRenderer);
    }

    if (pathFlags & EFlag_DrawPathShadowTexture) {
        mpDrawPathShadowTexture =
            RPGrpDrawPath::Construct<RPGrpDrawPathShadowTexture>(pRenderer);
    }

    createScnProc(ARRAY_SIZE(PROC_TIMINGS), NULL);

    for (int i = 0; i < (int)ARRAY_SIZE(PROC_TIMINGS); i++) {
        u16 prioMax = pRenderer->getLocalPriorityMax();
        pRenderer->setLocalPriorityScnProc(this, PROC_TIMINGS[i], prioMax, i);
    }
}

void RPGrpDrawPathManager::scnProcDraw(u16 idx) {
    if (mpDrawHook == NULL) {
        return;
    }

    // Wii Play: step 4 is drawn only while the shadow volume path is enabled
    if (idx == 4) {
        if (mpDrawPathSV == NULL || !mpDrawPathSV->IsEnable()) {
            return;
        }
    }

    mpDrawHook->Draw(idx);
}
