#include <Pack/RPParty/RPBilScene/RPBilCamera_4.h>

#include <Pack/RPUtility/RPUtlRandom.h>
#include <egg/core/eggController.h>

f32 RPBilPocketCam::sPocketCamParamA = 0.1f;
f32 RPBilPocketCam::sPocketCamDist = 50.0f;

RPBilPocketCam::~RPBilPocketCam() {}

RPBilPocketCam::RPBilPocketCam(s32 index) : RPInterpCamera(), mIndex(index) {}

void RPBilPocketCam::Reset() {
    RPInterpCamera::Reset();

    EGG::Vector3f position(1.0f, 0.0f, 0.0f);
    SetPosition(position);
    SetTarget(EGG::Vector3f::zero);
    EGG::Vector3f up(0.0f, 1.0f, 0.0f);
    SetUp(up);
    CalcMatrix();
}

void RPBilPocketCam::Calculate() {
    EGG::CoreControllerMgr::instance()->func_800A6F50(0);
    RPInterpCamera::Calculate();
}

#pragma fp_contract off
inline EGG::Vector3f& PocketCamCopyVector(EGG::Vector3f& out,
                                        const nw4r::math::VEC3& value) {
    out.set(value.x, value.y, value.z);
    return out;
}
void RPBilPocketCam::LookAtPocket(s32 pocket) {
    EGG::Vector3f dir(0.0f, -1.0f, 1.0f);
    dir.normalise();

    f32 range = 2.0f * sPocketCamParamA;
    f32 amplitude = sPocketCamParamA;
    dir.y += range * RPUtlRandom::getF32() - amplitude;
    dir.normalise();

    RPBilCamera* camera = RPBilCameraMgr::sInstance->GetCamera();
    EGG::Vector3f cameraDir;
    PocketCamCopyVector(cameraDir, camera->GetTargetRef() - camera->GetPosition());
    if (cameraDir.z < 0.0f) {
        dir.z = -dir.z;
    }

    f32 distance = sPocketCamDist;
    SetTarget(RPBilTable::sInstance->GetPocketPos(pocket));
    EGG::Vector3f position;
    SetPosition(PocketCamCopyVector(position, GetTargetRef() - dir * distance));

    EGG::Vector3f up(0.0f, 1.0f, 0.0f);
    EGG::Vector3f side = up.cross(dir);
    side.normalise();
    up = dir.cross(side);
    SetUp(up);
}
#pragma fp_contract on
