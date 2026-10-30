
#include <Pack/RPGraphics/RPGrpDrawPathLightMap.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpTexture.h>

#include <egg/prim/eggAssert.h>

bool RPGrpDrawPathLightMap::IsEnable() const {
    return mpLightTextureManager->isFlag4();
}

void RPGrpDrawPathLightMap::Off() {
    mpLightTextureManager->offFlag4();
}

void RPGrpDrawPathLightMap::On() {
    mpLightTextureManager->onFlag4();
}

// Refer to the real implicit destructor before On is emitted. The linker drops
// this helper and keeps the destructor in the original function order.
#pragma dont_inline on
static void DestroyLightMap(RPGrpDrawPathLightMap* pLightMap) {
    pLightMap->RPGrpDrawPathLightMap::~RPGrpDrawPathLightMap();
}
#pragma dont_inline reset

RPGrpDrawPathLightMap::RPGrpDrawPathLightMap(EGG::ScnRenderer* pRenderer)
    : RPGrpDrawPath(pRenderer),
      mpLightTextureManager(pRenderer->getLightManager()->GetLightTextureManager()) {}

void RPGrpDrawPathLightMap::ReplaceModelTexture(u16 index, RPGrpModel* pModel) {
    if (pModel->GetKind() != RPGrpModel::Kind_RFL) {
        mpLightTextureManager->replaceModelTexture(index, pModel->GetModelEx());
        return;
    }

    EGG::LightTextureManager* pLightTextures = GetLightTextureManager();
    RPGrpTexture texture(pLightTextures->getTexture(index));

    pLightTextures = GetLightTextureManager();
    pModel->ReplaceTexture(pLightTextures->getTexture(index)->getName(),
                           texture, true);
}
