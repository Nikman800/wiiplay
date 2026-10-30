// Second Billiards camera (RPBilAuxCamera, 3D view 1): only resets to its default view.
// Unit 0x802CAF50-0x802CB0A4. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilCamera.h>

#include <egg/core/eggController.h>

RPBilAuxCamera::~RPBilAuxCamera() {}

RPBilAuxCamera::RPBilAuxCamera() {}

void RPBilAuxCamera::Reset() {
    RPInterpCamera::Reset();

    EGG::Vector3f dir(1.0f, 0.0f, 0.0f);
    RPBilCamera::ComputeDefaultView(this, dir);
    CalcMatrix();
}

void RPBilAuxCamera::Calculate() {
    EGG::CoreControllerMgr::instance()->func_800A6F50(0);
    RPInterpCamera::Calculate();
}
