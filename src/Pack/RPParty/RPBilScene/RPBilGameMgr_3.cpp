// Billiards music holder (RPBilMusic, singleton RPBilMusic::sInstance): create, destroy and the destructor.
// Unit 0x802D036C-0x802D0434 (dtk/game_units.tsv). The destructor (0x802D032C, ledger
// Bil_ResultLyt_dtor) and the vtable (0x8035E528) currently sit in RPBilResult_2's ranges but
// belong here. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilGameMgr.h>

RPBilMusic* RPBilMusic::sInstance;

inline RPBilMusic::RPBilMusic() {
    mpState = new RPBilMusicState();
}

void RPBilMusic::CreateInstance() {
    sInstance = new RPBilMusic();
}

void RPBilMusic::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilMusic::~RPBilMusic() {}
