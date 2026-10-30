#include <egg/math.h>

namespace EGG {

class Projection {
public:
    static f32 calcViewDepth(const Vector3f& rCamPos, const Vector3f& rLookDir,
                             const Vector3f& rPos);
};

f32 Projection::calcViewDepth(const Vector3f& rCamPos,
                              const Vector3f& rLookDir, const Vector3f& rPos) {
    return rLookDir.dot(rPos - rCamPos);
}

} // namespace EGG
