// Tanks check mark pool (RPTnkCheckMarkPool).
// Unit 0x80274E84-0x80274FE0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkCheckMark.h>

RPTnkCheckMarkPool* RPTnkCheckMarkPool::sInstance ALIGN(8);

void RPTnkCheckMarkPool::Destroy() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkActor* RPTnkCheckMarkPool::CreateObject() {
    return new RPTnkCheckMark();
}

void RPTnkCheckMarkPool::CreateInstance() {
    sInstance = new RPTnkCheckMarkPool();
}

void RPTnkCheckMarkPool::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkCheckMarkPool::~RPTnkCheckMarkPool() {}
