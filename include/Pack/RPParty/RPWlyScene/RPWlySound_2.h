#ifndef RP_PARTY_WLY_SOUND_2_H
#define RP_PARTY_WLY_SOUND_2_H

#include <Pack/RPAudio/RPSndObject2D.h>

// RPWlySound.h does not declare RPWlySoundObj's inline constructor yet (RPWlySound_2.cpp
// inlines it into RPWlyGameMgr's constructor). This adds the declaration after DuckBgm.
// TODO(header): declare RPWlySoundObj() in RPWlySound.h and drop this.
#ifndef RP_PARTY_WLY_SOUND_H
#define DuckBgm() \
    DuckBgm();    \
    RPWlySoundObj()
#include <Pack/RPParty/RPWlyScene/RPWlySound.h>
#undef DuckBgm
#endif

#endif
