#ifndef RP_PARTY_WLY_AVATAR_MGR_2_H
#define RP_PARTY_WLY_AVATAR_MGR_2_H
#include <Pack/types_pack.h>

// RPWlyAvatarMgr_2.cpp: RPWlyStage (declared in RPWlyAvatarMgr.h) and RPWlyClearPhase
// (RPWlyPhase.h). The game and layout singletons keep their state private; this unit reads it
// directly, as the phases do (RPWlyPhase.cpp).

#define private public
#include <Pack/RPParty/RPWlyScene/RPWlyAvatar.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyCursor.h>
#include <Pack/RPParty/RPWlyScene/RPWlyField.h>
#include <Pack/RPParty/RPWlyScene/RPWlyField_3.h>
#include <Pack/RPParty/RPWlyScene/RPWlyField_5.h>
#include <Pack/RPUtility/RPUtlRandom.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLyt.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#undef private

#include <Pack/RPParty/RPWlyScene/RPWlyGameConfig.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLevelTable.h>
#include <Pack/RPParty/RPWlyScene/RPWlyPhase.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>

#include <egg/gfxe/eggScreen.h>

#include <nw4r/math.h>

#include <cmath>
#include <math.h>
#include <cstdio>
#include <stdlib.h>

// RPWlyScene.cpp: out-of-line copies of the progress kept in RPSceneInfo (real names unknown)

// RPWlyCursor.cpp

// RPWlyGameMgr (not decompiled yet; RPWlyGameMgr.h declares them without these parameters)

#endif
