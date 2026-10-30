
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysLytBounding.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".
#include <Pack/RPKernel.h>

#include <egg/math.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>

/**
 * @brief Tests whether a point falls within this bounding pane
 *
 * @param rInfo Pane draw info
 * @param rPoint Point to test
 */
bool RPSysLytBounding::isInsideBounding(const nw4r::lyt::DrawInfo& rInfo,
                                        const EGG::Vector2f& rPoint) {
    bool inside = false;

    EGG::Vector3f bx(mGlbMtx._00, mGlbMtx._10, mGlbMtx._20);
    EGG::Vector3f by(mGlbMtx._01, mGlbMtx._11, mGlbMtx._21);
    EGG::Vector3f bw(mGlbMtx._03, mGlbMtx._13, mGlbMtx._23);

    f32 lx = bx.length();
    f32 ly = by.length();

    if (lx < EGG::Mathf::epsilon() || ly < EGG::Mathf::epsilon()) {
        inside = false;
    } else {
        nw4r::ut::Rect rect;
        // Wii Play: the pane rect is a separate local, scaled into rect
        nw4r::ut::Rect paneRect = GetPaneRect(rInfo);

        bool inRect = false;

        rect.left = paneRect.left * lx;
        rect.right = paneRect.right * lx;
        rect.top = paneRect.top * ly;
        rect.bottom = paneRect.bottom * ly;

        // Wii Play: sp14 is filled in member by member
        EGG::Vector3f sp14;
        sp14.x = rPoint.x - bw.x;
        sp14.y = rPoint.y - bw.y;
        sp14.z = 0.0f;

        f32 dx = nw4r::math::VEC3Dot(&sp14, &bx);
        f32 dy = nw4r::math::VEC3Dot(&sp14, &by);

        EGG::Vector3f sp08(dx / lx, dy / ly, 0.0f);

        if (rect.left <= sp08.x && sp08.x <= rect.right &&
            rect.bottom <= sp08.y && sp08.y <= rect.top) {

            inRect = true;
        }

        if (inRect) {
            inside = true;
        }
    }

    return inside;
}
