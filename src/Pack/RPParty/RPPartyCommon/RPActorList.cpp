#include <Pack/RPParty/RPPartyCommon/RPActorList.h>

#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>

#include <egg/math.h>

// Returns the current actor and advances the iterator
static inline RPActor* NextActor(RPActor*& rpIt) {
    RPActor* pActor = rpIt;
    if (rpIt != NULL) {
        rpIt = rpIt->mpNext;
    }
    return pActor;
}

// Swaps two actors' places in the list
static inline void SwapActors(RPActorListHead* pList, RPActor* pA, RPActor* pB) {
    RPActor* pNext = pA->mpNext;
    RPActor* pPrev = pA->mpPrev;
    if (pNext == pB) {
        pA->mpNext = pB->mpNext;
        pA->mpPrev = pB;
        pB->mpNext = pA;
        pB->mpPrev = pPrev;
    } else if (pPrev == pB) {
        pA->mpNext = pB;
        pA->mpPrev = pB->mpPrev;
        pB->mpNext = pNext;
        pB->mpPrev = pA;
    } else {
        pA->mpNext = pB->mpNext;
        pA->mpPrev = pB->mpPrev;
        pB->mpNext = pNext;
        pB->mpPrev = pPrev;
    }
    if (pA->mpNext != NULL) {
        pA->mpNext->mpPrev = pA;
    } else {
        pList->mpTail = pA;
    }
    if (pA->mpPrev != NULL) {
        pA->mpPrev->mpNext = pA;
    } else {
        pList->mpHead = pA;
    }
    if (pB->mpNext != NULL) {
        pB->mpNext->mpPrev = pB;
    } else {
        pList->mpTail = pB;
    }
    if (pB->mpPrev != NULL) {
        pB->mpPrev->mpNext = pB;
    } else {
        pList->mpHead = pB;
    }
}

void RPActor::UpdateModelFrame(RPGrpModel* pModel) {
    pModel->UpdateFrame();
}

void RPActor::CalcModel(RPGrpModel* pModel, f32 scale, const nw4r::math::MTX34* pMtx) {
    f32 s = 0.01f;
    if (scale > s) {
        s = scale;
    }
    if (pModel->GetModelEx()->getScnObj() != NULL) {
        pModel->GetModelEx()->getScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, pMtx);
    }
    pModel->SetBaseScale(s, s, s);
    pModel->Calc();
}

void RPActor::EntryModel(RPGrpModel* pModel) {
    pModel->Entry();
}

void RPActorList::DrawScreen(u8 pass) {}

RPActorList::RPActorList()
    : mCapacity(0),
      mpTableA(NULL),
      mpTableNum(NULL) {}

RPActorList::~RPActorList() {
    if (mpTableA != NULL) {
        delete mpTableA;
    }
    if (mpTableNum != NULL) {
        delete mpTableNum;
    }
}

RPGrpCamera* RPActorList::CreateView3D(u8 viewNo, RPGrpScreen* pScreen) {
    RPGrpCamera* pCamera = new RPGrpCamera();
    RPGrpRenderer::GetCurrent()->CreateView3D(viewNo, pCamera, pScreen);
    return pCamera;
}

void RPActorList::CreateView2D(u8 viewNo, RPGrpScreen* pScreen) {
    RPGrpRenderer::GetCurrent()->CreateView2D(viewNo, pScreen);
}

void RPActorList::CreateResourceList(u16 num) {
    RPGrpModelResManager::GetCurrent()->CreateResourceList(num);
}

void RPActorList::AllocTables(u16 capacity) {
    mCapacity = capacity;
    mpTableA = new RPActor*[capacity];
    mpTableNum = new s32[capacity];
    for (int i = 0; i < capacity; i++) {
        mpTableA[i] = NULL;
        mpTableNum[i] = 0;
    }
}

void RPActorList::LoadResourceAll() {
    RPActor* pIt = mpHead;
    while (pIt != NULL) {
        RPActor* pActor = NextActor(pIt);
        pActor->LoadResource();
    }
}

void RPActorList::SortAndActivate() {
    RPActor* pIt = mpHead;
    while (pIt != NULL) {
        RPActor* pActor = NextActor(pIt);
        RPActor* pTarget;
        if (pIt != NULL && pActor != NULL && pIt->mSortKey < pActor->mSortKey) {
            pTarget = pIt;
            SwapActors(this, pIt, pActor);
            pIt = pActor;
        } else {
            pTarget = pActor;
        }
        if (pTarget->GetFlags().mActivate) {
            pTarget->GetFlags().mActive = true;
            pTarget->OnActivate();
        }
    }
}

void RPActorList::CalcActive() {
    RPActor* pIt = mpHead;
    while (pIt != NULL) {
        RPActor* pActor = NextActor(pIt);
        if (pActor->GetFlags().mActive && pActor->GetFlags().mCalc) {
            pActor->Calculate();
        }
    }
}

void RPActorList::DrawScreenAll(u8 pass) {
    RPActor* pIt = mpHead;
    while (pIt != NULL) {
        RPActor* pActor = NextActor(pIt);
        if (pActor->GetFlags().mActive) {
            pActor->DrawScreen(pass);
        }
    }
}

void RPActorList::DrawPassAll(u8 pass) {
    RPActor* pIt = mpHead;
    while (pIt != NULL) {
        RPActor* pActor = NextActor(pIt);
        if (pActor->GetFlags().mActive) {
            pActor->DrawPass(pass);
        }
    }
}

void RPActorList::StartActor(RPActor* pActor) {
    if (pActor == NULL) {
        return;
    }
    pActor->GetFlags().mStop = true;
    pActor->GetFlags().mActive = false;
    pActor->GetFlags().mActivate = true;
    pActor->GetFlags().mCalc = true;
    pActor->GetFlags().mStarted = true;
    pActor->OnStart();
}

void RPActorList::StopFlaggedActors() {
    RPActor* pIt = mpHead;
    while (pIt != NULL) {
        RPActor* pActor = NextActor(pIt);
        if (pActor->GetFlags().mStop) {
            StopActor(pActor);
        }
    }
}

void RPActorList::StopActor(RPActor* pActor) {
    if (pActor == NULL) {
        return;
    }
    pActor->GetFlags().mStop = false;
    pActor->GetFlags().mActive = false;
    pActor->GetFlags().mActivate = false;
    pActor->GetFlags().mCalc = false;
    pActor->GetFlags().mStarted = false;
    pActor->OnStop();
}
