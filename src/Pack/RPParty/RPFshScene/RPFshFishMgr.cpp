// Fishing fish manager (RPFshFishMgr): the 30 pond fish, the spawn pattern and the rare fish
// that appear at set times.
// Unit 0x80205A84-0x802067E4 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshFishMgr.h>

#include <Pack/RPParty/RPFshScene/RPFshBonusFish.h>
#include <Pack/RPParty/RPFshScene/RPFshFish.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshResultLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshTimer.h>
#include <Pack/RPUtility.h>

s32 RPFshFishMgr::sRareSpawnTimes[4] = {10, 30, 50, 90};
f32 RPFshFishMgr::sRareSpawnChances[4] = {25.0f, 25.0f, 25.0f, 100.0f};

RPFshFishMgr* RPFshFishMgr::sInstance;

inline s32 FshTimerHasStarted(const RPFshTimer* pTimer) {
    return static_cast<u32>(__cntlzw(static_cast<u32>(pTimer->IsNotStarted()))) >> 5;
}

inline bool FshTimerIsCounting(const RPFshTimer* pTimer) {
    bool counting;
    bool result = true;
    counting = true;
    s32 state = pTimer->GetState();
    if (state != RPFshTimer::STATE_COUNT && state != RPFshTimer::STATE_LAST_SECOND) {
        counting = false;
    }
    if (!counting && state != RPFshTimer::STATE_COUNTDOWN) {
        result = false;
    }
    return result;
}

inline RPFshFishMgr::RPFshFishMgr() {
    for (int i = 0; i < FISH_NUM; i++) {
        mpFish[i] = new RPFshFish(i, i % RPFshFish::TYPE_NUM);
    }
}

void RPFshFishMgr::CreateInstance() {
    sInstance = new RPFshFishMgr();
}

void RPFshFishMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshFishMgr::~RPFshFishMgr() {}

void RPFshFishMgr::CreateAll() {
    for (int i = 0; i < FISH_NUM; i++) {
        mpFish[i]->Create();
    }
}

void RPFshFishMgr::Reset() {
    unk10 = -1;
    mPattern = 0;

    if (RPFshPlayerMgr::GetInstance()->GetPlayerNum() > 1) {
        u32 thresholds[4] = {40, 60, 90, 100};
        u32 r = RPUtlRandom::getU32(100);
        for (s32 i = 0; i < 4; i++) {
            if (r < thresholds[i]) {
                mPattern = i;
                break;
            }
        }
    }

    mSpawnNum = 0;
    unkC = 0;

    for (int i = 0; i < FISH_NUM; i++) {
        mpFish[i]->Reset();
    }
}

void RPFshFishMgr::ExitAll() {
    for (int i = 0; i < FISH_NUM; i++) {
        mpFish[i]->Exit();
    }
}

void RPFshFishMgr::Calculate() {
    if (RPFshResultLayout::GetInstance()->IsShowing()) {
        return;
    }

    for (int i = 0; i < FISH_NUM; i++) {
        mpFish[i]->Calculate();
    }

    RPFshTimer* pTimer = RPFshTimer::GetInstance();
    if (pTimer->IsWindowEntered() && SpawnFishOfType(RPFshFish::TYPE_RARE_A)) {
        RPFshBonusFish::GetInstance()->SetNextType(RPFshFish::TYPE_RARE_A);
        mSpawnNum++;
    }

    if (FshTimerHasStarted(pTimer) && !pTimer->IsTimeUp() && FshTimerIsCounting(pTimer)) {
        for (int i = 0; i < 4; i++) {
            if (pTimer->GetFrame() == sRareSpawnTimes[i] * RPFshTimer::GetTimeLimitSec() &&
                CountFishOfType(RPFshFish::TYPE_RARE_A) == 0 &&
                RPUtlRandom::getU32(100) < sRareSpawnChances[i] &&
                CountFishOfType(RPFshFish::TYPE_RARE_B) < 1) {
                SpawnFishOfType(RPFshFish::TYPE_RARE_B);
                mSpawnNum++;
            }
        }
    }
}

void RPFshFishMgr::SetFishToSwim(s32 fishIdx) {
    for (int i = 0; i < FISH_NUM; i++) {
        s32 state = mpFish[i]->GetState();
        if (state != RPFshFish::STATE_NONE && state != RPFshFish::STATE_VANISH &&
            state != RPFshFish::STATE_ESCAPE && state != RPFshFish::STATE_JUMP) {
            mpFish[i]->ChangeState(RPFshFish::STATE_SWIM);
        }
    }

    mpFish[fishIdx]->ChangeState(RPFshFish::STATE_CATCH_VIEW);
}

void RPFshFishMgr::SpawnInitialFish() {
    s32 initialNum = sSpawnPatterns[mPattern].mInitialNum;
    for (int i = 0; i < initialNum; i++) {
        SpawnRandomFish();
    }
}

const RPFshSpawnPattern RPFshFishMgr::sSpawnPatterns[4] = {
    {5, 2, {25, 25, 25, 25, 0, 0}},
    {2, 1, {25, 25, 25, 25, 0, 0}},
    {8, 4, {25, 25, 25, 25, 0, 0}},
    {5, 4, {10, 10, 10, 70, 0, 0}},
};

void RPFshFishMgr::SpawnRandomFish() {
    if (mSpawnNum == SPAWN_MAX) {
        return;
    }

    bool retry;
    u32 total = 0;
    for (int i = 0; i < RPFshFish::TYPE_NUM; i++) {
        total += sSpawnPatterns[mPattern].mTypeRates[i];
    }
    retry = true;
    const RPFshSpawnPattern* pPattern = &sSpawnPatterns[mPattern];
    while (retry) {
        s32 r = RPUtlRandom::getU32(total);
        s32 type = 0;

        s32 sum = 0;
        for (int i = 0; i < RPFshFish::TYPE_NUM; i++) {
            if (r < sum + pPattern->mTypeRates[i]) {
                type = i;
                break;
            }
            sum += pPattern->mTypeRates[i];
        }

        if (SpawnFishOfType(type)) {
            retry = false;
            mSpawnNum++;
        }
    }
}

bool RPFshFishMgr::SpawnFishOfType(s32 type) {
    const RPFshSpawnPattern* pPattern;
    if (mSpawnNum >= SPAWN_MAX) {
        return false;
    }

    pPattern = GetSpawnPattern();
    s32 count = CountFishOfType(type);
    if (count >= pPattern->mMaxSameType) {
        return false;
    }

    s32 start = RPUtlRandom::getU32(FISH_NUM);
    for (s32 i = start; i < start + FISH_NUM; i++) {
        RPFshFish* pFish = mpFish[i % FISH_NUM];
        if (type == pFish->GetType()) {
            if ((pFish->GetState() == RPFshFish::STATE_NONE && !pFish->IsChanging()) ||
                (pFish->GetNextState() == RPFshFish::STATE_NONE && pFish->IsChanging())) {
                pFish->Spawn();
                return true;
            }
        }
    }

    return false;
}

s32 RPFshFishMgr::CountFishOfType(s32 type) const {
    s32 num = 0;
    for (int i = 0; i < FISH_NUM; i++) {
        if ((mpFish[i]->GetState() != RPFshFish::STATE_NONE ||
             (mpFish[i]->GetState() == RPFshFish::STATE_NONE && mpFish[i]->IsChanging())) &&
            type == mpFish[i]->GetType()) {
            num++;
        }
    }
    return num;
}
