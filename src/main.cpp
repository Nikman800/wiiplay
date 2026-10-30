#include <Pack/RPSystem.h>
// Ported from doldecomp/ogws (CC0) src/main.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <revolution/OS.h>

// Stubbed for release
void OSReport(const char* /* msg */, ...) {
    ;
}

#if defined(__MWERKS__)
void main(int /* argc */, char** /* argv */) {
#else
int main(int /* argc */, char** /* argv */) {
#endif
    RPSysSystem::initialize();
    RPSysSystem::create();
    RP_GET_INSTANCE(RPSysSystem)->setup();
    RP_GET_INSTANCE(RPSysSystem)->mainLoop();
}
