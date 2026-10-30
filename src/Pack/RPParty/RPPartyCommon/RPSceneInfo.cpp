#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPKcl.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPVec3Range.h>

#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPSystem/RPSysResourceManager.h>


extern "C" void RP_Cls803572E0_createInstance();

RPCls803572E0* RPCls803572E0::sInstance;

void RP_Cls803572E0_createInstance() {
    RPCls803572E0::sInstance = new RPCls803572E0;
}

void RPCls803572E0::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPCls803572E0::~RPCls803572E0() {}

void RPCls803572E0::LoadMessage(const char* pName) {
    void* pResource = RPSysResourceManager::GetMessageResource(pName);
    mpMessage = new RPSysMessage(pResource, NULL);
}

RPKclHolder::RPKclHolder() {
    mpKcl = new RPKcl;
}

void RPKclHolder::SetResource(void* pResource) {
    mpKcl->SetResource(pResource);
}

EGG::Vector3f RPVec3Range::GetSum() const {
    return mMin + mMax;
}

EGG::Vector3f RPKclHolder::CalcPushOut(const nw4r::math::VEC3& rPos, f32 radius) const {
    RPKclHit hits[0x20];

    s32 hitCount = mpKcl->SearchSphereOctree(hits, 0x20, rPos, radius);
    if (hitCount > 0x20) {
        hitCount = 0x20;
    }

    RPVec3Range bounds;

    RPKclHit* pHit = hits;
    for (; hitCount > 0; --hitCount, ++pHit) {
        RPKclHit& rHit = *pHit;
        EGG::Vector3f offset(rHit.normal.x, rHit.normal.y, rHit.normal.z);
        offset *= rHit.distance;
        bounds.Extend(offset);
    }

    return bounds.GetSum();
}
