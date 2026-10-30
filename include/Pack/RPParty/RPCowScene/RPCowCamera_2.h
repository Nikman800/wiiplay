#ifndef RP_PARTY_COW_CAMERA_2_H
#define RP_PARTY_COW_CAMERA_2_H

#include <Pack/types_pack.h>
#include <egg/math.h>

class RPGrpCamera;

//! @addtogroup rp_cow
//! @{

class RPCowCamera2 {
public:
    static void InitStatics();
    static void OnTimeUp(void* pGame, s32 player);
    static void OnCourseComplete(void* pGame, s32 player);
    static void DtorThunk(void* pThis);
    static f32 CalcSquareDistance(void* pThis, const EGG::Vector3f* pPos);
    static bool IsInView(void* pThis, const EGG::Vector3f* pPos);
    static void SetModelVisible(void* pThis, bool visible);
    static void StateOverview(void* pThis);
    static void StateFollowEase(void* pThis);
    static void StateFollowSnap(void* pThis);
    static void StateFollowNear(void* pThis);
    static void StateFollow(void* pThis);
    static void StateFinish(void* pThis);
    static void StateResult(void* pThis);
    static void StateResultOrbit(void* pThis);
    static void StateIntro(void* pThis);
    static void StateIntroBlend(void* pThis);
    static void StateReady(void* pThis);
    static void StateStart(void* pThis);
    static void StartIntro(void* pThis);
    static void SetupIntro(void* pThis);
    static bool IsIntroDone(void* pThis);
    static void SetupReady(void* pThis);
    static void StartStart(void* pThis);
    static bool IsPullbackDone(void* pThis);
    static void SetFollowNearState(void* pThis);
    static void SetFollowState(void* pThis);
    static void StartFinish(void* pThis);
    static void StartResult(void* pThis);
    static void Create(void* pThis);
    static void Destroy(void* pThis);
    static void Init(void* pThis);
    static void Reset(void* pThis);
    static void Exit(void* pThis);
    static void Calc(void* pThis);
    static void EntryDraw(void* pThis);
    static void DrawPass(void* pThis, u8 pass);
    static void Ctor(void* pThis);
};

class RPCowCamera {
public:
    char pad0[0x2C];
    u8 mViewA;
    char pad2D[0x7C - 0x2D];
    RPGrpCamera* mpDrawCamera;

    EGG::Vector3f GetBackDir() const;
    EGG::Vector3f GetEyePos() const;
    EGG::Vector3f ProjectToScreen(const EGG::Vector3f& rPos) const;
    EGG::Vector3f ProjectToScreenClamped(const EGG::Vector3f& rPos, f32 depth) const;
};

//! @}

#endif
