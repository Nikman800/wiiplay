// Tanks empty task (RPTnkEmptyTask).
// Unit 0x80268A68-0x80268B64 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkTask.h>

RPTnkEmptyTask* RPTnkEmptyTask::sInstance;

void RPTnkEmptyTask::CreateInstance() {
    sInstance = new RPTnkEmptyTask();
}

void RPTnkEmptyTask::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkEmptyTask::~RPTnkEmptyTask() {}

void RPTnkEmptyTask::Create() {}

void RPTnkEmptyTask::Load() {}

void RPTnkEmptyTask::Reset() {}

void RPTnkEmptyTask::Calculate() {}

void RPTnkEmptyTask::Exit() {}
