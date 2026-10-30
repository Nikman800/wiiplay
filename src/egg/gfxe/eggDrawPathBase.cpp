#include <egg/gfxe.h>

namespace EGG {

DrawPathBase::DrawPathBase() {}

void DrawPathBase::calc() {
    if (isEnable()) {
        setScreen(IDrawGX::getScreen());
        internalCalc();
    }
}

/**
 * STAND-IN (invented; the linker dead-strips all of it). The original evidently
 * compiled a class derived from DrawPathBase here, between calc and scnProcDraw:
 * - CW emits a used inline function right after the first function that uses it.
 *   reset() uses internalResetForDraw and the constructor uses ~DrawPathBase, so
 *   both come out before scnProcDraw, in the original order.
 * - Building a derived vtable in this file makes CW emit DrawPathBase's IScnProc
 *   thunks forward (@112@__dt__ before @112@scnProcDraw); otherwise it emits
 *   them in reverse.
 */
class DrawPathBaseStandIn : public DrawPathBase {
public:
    DrawPathBaseStandIn();
    void reset();
};

void DrawPathBaseStandIn::reset() {
    internalResetForDraw();
}

DrawPathBaseStandIn::DrawPathBaseStandIn() {}

void DrawPathBase::scnProcDraw(u16 step) {
    if (isEnable()) {
        internalDraw(step);
    }
}

} // namespace EGG
