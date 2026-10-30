#include <Pack/RPSystem.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPSystem/RPSysQueuedScene.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

// Wii Play: DestroyInstance is not in the binary (unused, dead-stripped)
RP_SINGLETON_IMPL_EX_NO_DESTROY(RPSysQueuedScene);

// Wii Play: only inlined into CreateInstance
inline RPSysQueuedScene::RPSysQueuedScene(EGG::Heap* pHeap) : mpParentHeap(pHeap) {
    mNextSceneID = -1;
}

// Wii Play: reset() is not in the binary (unused, dead-stripped).

RPSysQueuedScene::~RPSysQueuedScene() {}
