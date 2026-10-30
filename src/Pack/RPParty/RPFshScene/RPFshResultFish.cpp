// Fishing result-screen fish (RPFshResultFish): a fish model placed under a screen position,
// facing the camera.
// Functions are defined in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshResultFish.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPFshScene/RPFshCamera.h>

#include <egg/gfxe.h>
#include <egg/math.h>

static inline void AssignVector(EGG::Vector3f& dst, const nw4r::math::VEC3& src) {
    f32 z = src.z;
    f32 y = src.y;
    f32 x = src.x;
    dst.x = x;
    dst.y = y;
    dst.z = z;
}

static inline void AssignPosition(nw4r::math::VEC3& dst, const EGG::Vector3f& src) {
    dst.x = src.x;
    dst.y = src.y;
    dst.z = src.z;
}

static inline void AssignScaledVector(EGG::Vector3f& dst, const nw4r::math::VEC3& src) {
    // Preserve the argument evaluation used by the scaled camera basis vectors.
    dst.set(src.x, src.y, src.z);
}

RPFshResultFish::~RPFshResultFish() {}

RPFshResultFish::RPFshResultFish(s32 type) {
    mType = type;
    mAxisZ = EGG::Vector3f(-1.0f, 0.0f, 0.0f);
    mAxisY = EGG::Vector3f(0.0f, 1.0f, 0.0f);
    mAxisX = EGG::Vector3f(0.0f, 0.0f, 1.0f);
}

void RPFshResultFish::Create() {
    const char* modelNames[6] = {"fsh_fish_A",     "fsh_fish_B",     "fsh_fish_C",
                                 "fsh_fish_sukaA", "fsh_fish_rareA", "fsh_fish_rareB"};
    // Wii Play: the (handle, int) overload is called with the model name
    mpModel = RPGrpModel::Construct(mType + 8, modelNames[mType], 1, 0, 0);
    mpModel->SetDrawScene(2);
}

void RPFshResultFish::Reset() {
    unk64 = 0;
    unk68 = 0;
    unk6C = false;
}

void RPFshResultFish::Exit() {}

void RPFshResultFish::Calculate() {
    if (unk6C) {
        unk64 = unk68;
        unk6C = false;
    }

    if (unk64 != 0) {
        RPFshCamera* pCamera = RPFshCameraMgr::GetInstance()->GetCamera();
        nw4r::math::VEC3 direction = pCamera->GetPosition() - pCamera->GetTargetRef();
        mAxisX.x = direction.x;
        mAxisX.y = direction.y;
        mAxisX.z = direction.z;
        mAxisX.normalise();

        const nw4r::math::VEC3& rRight = pCamera->GetRight();
        mAxisZ = EGG::Vector3f(-rRight.x, -rRight.y, -rRight.z);
        mAxisY = mAxisZ.cross(mAxisX);

        CalcModelMtx();
    }
}

void RPFshResultFish::SetScreenPos(const nw4r::math::VEC2& rPos) {
    mScreenPos = rPos;

    RPFshCamera* pCamera = RPFshCameraMgr::GetInstance()->GetCamera();
    EGG::Vector3f dir, base, right, up;
    AssignVector(dir, pCamera->GetTargetRef() - pCamera->GetPosition());
    dir.normalise();

    AssignVector(base, pCamera->GetPosition() + dir * 100.0f);
    AssignScaledVector(right, pCamera->GetRight() * EGG::Screen::GetSizeXMax());
    AssignScaledVector(up, pCamera->GetUp() * EGG::Screen::GetSizeYMax());

    AssignPosition(mPos, base + right * rPos.x + up * rPos.y);
}
