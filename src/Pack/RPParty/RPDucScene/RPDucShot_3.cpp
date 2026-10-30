// Shooting Range pointer history (RPDucShotPointer): pushes the remote's latest acceleration.
// Unit 0x8024FD3C-0x8024FF80 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucShot.h>

#include <egg/core.h>

RPDucShotPointer::RPDucShotPointer(s32 channel, s32 capacity) : RPPosHistory(channel, capacity) {}

RPDucShotPointer::~RPDucShotPointer() {}

void RPDucShotPointer::PullFromController() {
    EGG::CoreStatus status;
    status = *EGG::CoreControllerMgr::instance()->getNthController(mChannel)->getCoreStatus(0);
    EGG::Vector3f acceleration;
    acceleration.x = status.acc.x;
    acceleration.y = status.acc.y;
    acceleration.z = status.acc.z;
    Push(acceleration);
}
