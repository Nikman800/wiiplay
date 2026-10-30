#define RP_ACTOR_DEFINE_VF
#include <Pack/RPParty/RPPartyCommon/RPActor_2.h>

#pragma defer_codegen off

RPActor::~RPActor() {}

RPActor::RPActor() : mPriority(0), unk1A(0), mFlags(0) {}

void RPActor::VF_0x3C() {}

void RPActor::VF_0x40() {}

u16 RPActor::GetUnk1A() const {
    return unk1A;
}

u16 RPActor::GetPriority() const {
    return mPriority;
}
