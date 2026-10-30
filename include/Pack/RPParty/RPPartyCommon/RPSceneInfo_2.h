#ifndef RP_PARTY_COMMON_SCENE_INFO_2_H
#define RP_PARTY_COMMON_SCENE_INFO_2_H

#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>

class RPHkyGame;

extern "C" void RP_createSceneInfo();
extern "C" void RP_getPairFromField20(EGG::Vector2f* pOut, const RPHkyGame* pGame);
extern "C" void RP_getPairFromField1C(EGG::Vector2f* pOut, const RPHkyGame* pGame);

#endif
