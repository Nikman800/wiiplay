#include <Pack/RPParty/RPPartyCommon/RPKokeshiMgr_2.h>

#include <Pack/RPGraphics.h>
#include <egg/math.h>
#include <revolution/MTX.h>

RPKokeshiHead::RPKokeshiHead(const RPSysKokeshiGenInfo* pGenInfo)
    : RPSysKokeshi(pGenInfo), IRPSysHostIOSocket("Kokeshi Head") {
    mFlags = 0;
}

RPKokeshiHead::~RPKokeshiHead() {}

void RPKokeshiHead::ResetTransform() {
    mFlags |= 1;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    PSMTXIdentity(mMtx.mtx);
}

void RPKokeshiHead::Calculate(bool visible) {
    if (GetBodyModel()->GetScnObj() != NULL) {
        GetBodyModel()->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mMtx);
    }

    GetBodyModel()->SetBaseScale(mScale);
    GetBodyModel()->Calc();

    EGG::Matrix34f headMtx;
    GetHeadJointMtx(&headMtx);

    if (GetNigaoeModel()->GetScnObj() != NULL) {
        GetNigaoeModel()->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &headMtx);
    }

    GetNigaoeModel()->SetBaseScale(mScale);
    GetNigaoeModel()->Calc();

    if (visible) {
        GetNigaoeModel()->SetDrawGroup(true);
        GetBodyModel()->SetDrawGroup(true);
    } else {
        GetNigaoeModel()->SetDrawGroup(false);
        GetBodyModel()->SetDrawGroup(false);
    }

    if ((mFlags & 1) != 0) {
        GetNigaoeModel()->Entry();
        GetBodyModel()->Entry();
    }
}

void RPKokeshiHead::SetTransform(const nw4r::math::MTX34& rMtx,
                                 const nw4r::math::VEC3& rScale) {
    static_cast<EGG::Matrix34f&>(mMtx) = static_cast<const EGG::Matrix34f&>(rMtx);

    mScale = rScale;
}
