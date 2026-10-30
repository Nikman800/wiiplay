#include <Pack/RPParty/RPBomScene/RPBomScore.h>

#include <Pack/RPParty/RPBomScene/RPBomCursor.h>
#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>
#include <Pack/RPParty/RPBomScene/RPBomLyt.h>
#include <Pack/RPParty/RPBomScene/RPBomQuestionParam.h>

#include <egg/gfxe/eggScreen.h>

#pragma opt_unroll_loops off


static inline void SpawnScorePopup(nw4r::math::VEC2 screenPos, s32 points, s32 player, BOOL doubled) {
    RPBomLayoutMgr::sInstance->SpawnScorePopup(&screenPos, points, player, doubled);
}

void RPBomScore::ClearTotals() {
    s32 count = 0;
    do {
        mTotals[count] = 0;
        count++;
    } while (count < mPlayerNum);
}

void RPBomScore::AddScore(s32 player, s32 points) {
    s32* pScore = &mScores[player];
    *pScore = *pScore + points;
    s32 score = *pScore;
    score = nw4r::ut::Max(score, 0L);
    score = nw4r::ut::Min(score, 9999L);
    *pScore = score;
}

void RPBomScore::AddTotal(s32 player, s32 points) {
    s32* pTotal = &mTotals[player];
    s32 total = *pTotal + points;
    *pTotal = total;
    total = nw4r::ut::Max(total, 0L);
    total = nw4r::ut::Min(total, 99999L);
    *pTotal = total;
}

// NONMATCHING-DEBT: ResetAll, ResetCombos and Reset's second loop walk an s32 pointer advanced
// from `this` (word offsets of mScores/mTotals/mHits/mCombos, unk4C/unkCC/unk14C/unk1CC). Indexed
// loops (mCombos[count] = 0) give the same code with the counter and the pointer in swapped
// registers (the original has the counter first, in r5); loop forms, counter types, element
// pointers declared first or after, and accessor inlines were tried.
void RPBomScore::ResetAll() {
    s32 count = 0;
    s32* pScores = reinterpret_cast<s32*>(this);
    do {
        pScores[1] = 0;
        count++;
        pScores++;
    } while (count < mPlayerNum);

    count = 0;
    s32* pTotals = reinterpret_cast<s32*>(this);
    do {
        pTotals[5] = 0;
        count++;
        pTotals++;
    } while (count < mPlayerNum);

    count = 0;
    s32* pHits = reinterpret_cast<s32*>(this);
    do {
        pHits[9] = 0;
        count++;
        pHits++;
    } while (count < mPlayerNum);

    count = 0;
    s32* pCombos = reinterpret_cast<s32*>(this);
    do {
        pCombos[13] = 0;
        count++;
        pCombos++;
    } while (count < mPlayerNum);
}

void RPBomScore::ResetCombos() {
    s32 count = 0;
    s32* pValue = reinterpret_cast<s32*>(this);
    do {
        pValue[13] = 0;
        count++;
        pValue++;
    } while (count < mPlayerNum);
}

void RPBomScore::AddBubblePoints(s32 player, const nw4r::math::VEC2* pScreenPos, BOOL doubled) {
    s32* pCombo = &mCombos[player];
    s32 points = RPBomQuestionParam::sInstance->mpFile->mComboPoints[*pCombo];
    s32 added = points;
    if (doubled) {
        added *= 2;
    }
    AddScore(player, added);
    AddTotal(player, points);
    ++mHits[player];
    RPBomGameManager::sInstance->TryAdvanceQuestion();

    if (mScores[player] == 9999) {
        RPBomGameManager::sInstance->SetForcedMiss();
    }

    u16 width = EGG::Screen::GetSizeXMax() / 2;
    u16 height = EGG::Screen::GetSizeYMax() / 2;
    f32 x = pScreenPos->x / static_cast<f32>(width);
    f32 y = pScreenPos->y / static_cast<f32>(height);
    nw4r::math::VEC2 pos(x, y);
    SpawnScorePopup(pos, points, player, doubled);

    s32 combo = *pCombo + 1;
    *pCombo = combo;
    combo = nw4r::ut::Max(combo, 0L);
    s32 maxCombo = GetComboNum() - 1;
    if (maxCombo < combo) {
        combo = maxCombo;
    }
    *pCombo = combo;
}

void RPBomScore::ApplyMissPenalty(s32 player) {
    s32* pScore = mScores + player;
    s32 penalty = (RPBomGameManager::sInstance->mLevel + 1) * -5;
    if (*pScore < -penalty) {
        penalty = -*pScore;
    }
    s32 newScore = mScores[player] - -penalty;
    mScores[player] = newScore;
    newScore = nw4r::ut::Max(newScore, 0L);
    newScore = nw4r::ut::Min(newScore, 9999L);
    mScores[player] = newScore;
    RPBomLayoutMgr::sInstance->ShowScoreMove(player, -penalty);
}

s32 RPBomScore::GetWinner() const {
    s32 first = mScores[1];
    s32 second = mScores[0];
    if (second > first) {
        return 0;
    }
    if (second < first) {
        return 1;
    }
    return -1;
}

// The original's .sdata2 has this constant after AddBubblePoints' literal pool and before
// Reset's: the functions above are emitted (in reverse order) before it, the ones below in
// definition order.
#pragma defer_codegen off

const s32 RPBomScore::sComboNum = 10;

void RPBomScore::Reset(s32 playerNum) {
    mPlayerNum = playerNum;

    s32 count = 0;
    do {
        mScores[count] = 0;
        mTotals[count] = 0;
        mHits[count] = 0;
        mCombos[count] = 0;
        count++;
    } while (count < mPlayerNum);

    count = 0;
    s32* pScores = reinterpret_cast<s32*>(this);
    s32* pVecs = reinterpret_cast<s32*>(this);
    do {
        pScores[19] = 0;
        pScores[83] = 0;
        pScores[51] = 0;
        *reinterpret_cast<f32*>(&pVecs[115]) = 0.0f;
        *reinterpret_cast<f32*>(&pVecs[116]) = 0.0f;
        count++;
        pScores++;
        pVecs += 2;
    } while (count < 32);

    unk48 = 0;
}

RPBomScore::~RPBomScore() {
}

void RPBomScore::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

void RPBomScore::CreateInstance() {
    sInstance = new RPBomScore;
}
