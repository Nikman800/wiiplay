#ifndef RP_PARTY_BOM_SCORE_H
#define RP_PARTY_BOM_SCORE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

/**
 * @brief Scores, question points and combos per player (singleton RPBomScore::sInstance)
 * @details vtable 8035EF90 (no overrides but the destructor). Size 0x2CC, verified
 * (Bom_Score_createInstance). A matched bubble adds RPBomQuestionParamFile::mComboPoints of
 * the player's combo (doubled for the other player's bubble in two-player mode) to mScores
 * (max 9999) and mTotals (max 99999, spent by RPBomGameManager::TryAdvanceQuestion); a miss
 * costs 5 points per level.
 * @note Name is a project convention (ledger Bom_Score).
 */
class RPBomScore : public RPSceneObj {
public:
    static RPBomScore* sInstance; // 8043CAC0

    virtual ~RPBomScore(); // at 0x8

    void Reset(s32 playerNum);
    void ResetAll();
    void ClearTotals();
    void ResetCombos();
    void AddScore(s32 player, s32 points);
    void AddTotal(s32 player, s32 points);
    void AddBubblePoints(s32 player, const nw4r::math::VEC2* pScreenPos, BOOL doubled);
    void ApplyMissPenalty(s32 player);
    /**
     * @brief 0 or 1, -1 for a draw [H]
     */
    s32 GetWinner() const;

    static void CreateInstance();
    static void DestroyInstance();

    static s32 GetComboNum() {
        return sComboNum;
    }

    static const s32 sComboNum; // 80441788 (combos 0-9)

public:
    s32 mScores[4];        // at 0x4
    s32 mTotals[4];        // at 0x14 question points
    s32 mHits[4];          // at 0x24
    s32 mCombos[4];        // at 0x34 0-9
    s16 mPlayerNum;        // at 0x44
    u32 unk48;             // at 0x48 [L] cleared only
    u32 unk4C[32];         // at 0x4C [L] cleared only
    u32 unkCC[32];         // at 0xCC [L] cleared only
    u32 unk14C[32];        // at 0x14C [L] cleared only
    f32 unk1CC[32][2];     // at 0x1CC [L] cleared only
};

//! @}

#endif
