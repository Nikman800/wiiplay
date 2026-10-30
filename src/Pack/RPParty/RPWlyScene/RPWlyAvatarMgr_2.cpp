// Find Mii stage: avatar placement patterns, their motion, target bookkeeping and hover picks
// (RPWlyStage), plus the stage clear phase (RPWlyClearPhase).
// Unit 0x80280070-0x8028CC58 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr_2.h>
#include <Pack/RPParty/RPWlyScene/RPWlyCursor.h>

inline RPWlyAvatar* Wly_GetAvatar(s32 idx) { RPWlyAvatar* avatar = RPWlyAvatarMgr::sInstance->mpAvatars[idx]; return avatar; }

inline f32 Wly_randF32Value() { f32 value = static_cast<f32>(static_cast<u16>(RPUtlRandom::RANDF_MAX & (RPUtlRandom::sRandom.getRandU32() >> RPUtlRandom::RANDF_SHIFT))); return value / static_cast<f32>(RPUtlRandom::RANDF_MAX + 1); }
inline u32 Wly_randU32Value(u32 max) { return max * Wly_randF32Value(); }

inline s32 Wly_GetAvatarNum() { s32 num = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; return num; }

inline RPWlyAvatar* Wly_GetAvatarDirect(s32 idx) { return RPWlyAvatarMgr::sInstance->mpAvatars[idx]; }

inline f32 Wly_GetAvatarAngle(RPWlyAvatar* p) { return p->mSwaySpeedZ; }

inline f32 Wly_Constant(f32 value) { f32 result = value; return result; }

inline RPGrpCamera* Wly_GetCamera() { return RPWlyGameMgr::sInstance->mpCamera; }

inline f32 Wly_Subtract(f32 a, f32 b) { return a - b; }

// Read by RPWlyLevelTable.cpp (field type 3); a named object, first in this file's .sdata2.
extern const f32 Wly_fieldType3Unk20;
const f32 Wly_fieldType3Unk20 = 6.25f;

f32 RPWlyStage::sAvatarSpacing[2] = {5.0f, 0.0f};


RPWlyStage::RPWlyStage() {
    mActive = false;
    mTargetIdx = -1;

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 8 &&
        RPSysKokeshiManager::GetOfficialDBValidNum() == 0) {
        RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType = 0;
    }

    unkC = 0;

    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 1:
    case 10:
    case 15:
    case 18:
    case 22:
        mTargetNum = 2;
        break;
    case 2:
    case 11:
    case 16:
    case 19:
        mTargetNum = 3;
        break;
    case 3:
        mTargetNum = 4;
        break;
    case 4:
        mTargetNum = 5;
        break;
    default:
        mTargetNum = 1;
        break;
    }

    mFoundNum = 0;
    for (s32 i = 0; i < 5; i++) {
        mFound[i] = false;
    }

    for (s32 i = 0; i < 100; i++) {
        mHover[i].mHovered = false;
        mHover[i].mTimer = 0;
        mHover[i].mPlayer = 0;
    }

    mMotionTimer = 0;
    mMotionValue = 1.0f;
    mClampX = 0.0f;
    mClampZ = 0.0f;

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1) {
        RPWlyStage::sAvatarSpacing[0] = 7.5f;
    } else {
        RPWlyStage::sAvatarSpacing[0] = 5.0f;
    }
}

// Rectangular formation: cols x rows (one row for 4 x 1), centred on the origin. The counts are
// parameters, not template arguments: their float conversions fold only after inlining, which
// keeps `speed * width` and `(rows - 1) / 2.0f - r` as written.
inline void Wly_Formation_GRID(RPWlyStage* stage, s32 cols, s32 rows) {
    f32 speed = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpeed;
    f32 depth = RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20;
    // Declaration order preserves the counter registers in the larger grids.
    s32 c;
    s32 r;
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            const f32 width = cols - 1;
            stage->mPositions[r * cols + c].x = speed * c - speed * width / 2.0f;
            stage->mPositions[r * cols + c].y = 0.0f;
            stage->mPositions[r * cols + c].z = depth * ((rows - 1) / 2.0f - r);
        }
    }
    const f32 width = cols - 1;
    const f32 height = rows - 1;
    stage->mClampX = speed * width / 2.0f;
    stage->mClampZ = depth * height / 2.0f;
}

// Row start plus column through an inline call: keeps `rowStart + c` from being reassociated.
inline s32 Wly_SlotIndex(s32 rowStart, s32 c) { return rowStart + c; }
// Staggered formation (field type 3): even rows hold cols avatars, odd rows cols + 1.
inline void Wly_Formation_STAGGER(RPWlyStage* stage, s32 cols, s32 rows) {
    f32 speed = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpeed;
    f32 depth = RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20;
    for (s32 r = 0; r < rows; r++) {
        if (r % 2) {
            for (s32 c = 0; c < cols + 1; c++) {
                stage->mPositions[Wly_SlotIndex(r / 2 + r * cols, c)].x = speed * c - speed * cols / 2.0f;
                stage->mPositions[Wly_SlotIndex(r / 2 + r * cols, c)].y = 0.0f;
                stage->mPositions[Wly_SlotIndex(r / 2 + r * cols, c)].z = depth * ((rows - 1) / 2.0f - r);
            }
        } else {
            for (s32 c = 0; c < cols; c++) {
                stage->mPositions[Wly_SlotIndex(r / 2 + r * cols, c)].x = speed * c - speed * (cols - 1) / 2.0f;
                stage->mPositions[Wly_SlotIndex(r / 2 + r * cols, c)].y = 0.0f;
                stage->mPositions[Wly_SlotIndex(r / 2 + r * cols, c)].z = depth * ((rows - 1) / 2.0f - r);
            }
        }
    }
    f32 width = cols;
    f32 height = rows - 1;
    stage->mClampX = speed * width / 2.0f;
    stage->mClampZ = depth * height / 2.0f;
}


void RPWlyStage::PlaceAvatarsFormation() {
    s32 num = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;
    s32 order[100];

    for (s32 i = 0; i < num; i++) {
        order[i] = i;
    }

    for (s32 i = 0; i < num - 1; i++) {
        s32 tries = 0;
        s32 j;
        s32 val;
    retry:
        j = i + RPUtlRandom::getU32(num - i);
        val = order[j];
        if (i < mTargetNum && tries < 1000) {
            for (s32 k = i - 1; k >= 0; k--) {
                if (abs(order[k] - val) < 1.5f) {
                    tries++;
                    goto retry;
                }
            }
        }
        order[j] = order[i];
        order[i] = val;
    }

    for (s32 i = 0; i < 100; i++) {
        mPositions[i].x = RPSimpleModel::sVecZero.x;
        mPositions[i].y = RPSimpleModel::sVecZero.y;
        mPositions[i].z = RPSimpleModel::sVecZero.z;
    }

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType != 3) {
        if (num <= 9) {
            switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
            case 6:
            case 7:
                if (num <= 4) {
                    Wly_Formation_GRID(this, 4, 1);
                } else if (num <= 8) {
                    Wly_Formation_GRID(this, 4, 2);
                } else {
                    Wly_Formation_GRID(this, 4, 3);
                }
                break;
            default:
                if (num == 8) {
                    Wly_Formation_GRID(this, 4, 2);
                } else if (num <= 4) {
                    Wly_Formation_GRID(this, 2, 2);
                } else {
                    Wly_Formation_GRID(this, 3, 3);
                }
                break;
            }
        } else if (num <= 16) {
            Wly_Formation_GRID(this, 4, 4);
        } else if (num <= 25) {
            Wly_Formation_GRID(this, 5, 5);
        } else if (num <= 36) {
            Wly_Formation_GRID(this, 6, 6);
        } else if (num <= 49) {
            Wly_Formation_GRID(this, 7, 7);
        } else if (num <= 64) {
            Wly_Formation_GRID(this, 8, 8);
        } else if (num <= 81) {
            Wly_Formation_GRID(this, 9, 9);
        } else {
            Wly_Formation_GRID(this, 10, 10);
        }
    } else {
        switch (num) {
        case 7:
            Wly_Formation_STAGGER(this, 2, 3);
            break;
        case 10:
            Wly_Formation_STAGGER(this, 3, 3);
            break;
        case 13:
            Wly_Formation_STAGGER(this, 4, 3);
            break;
        case 16:
            Wly_Formation_STAGGER(this, 5, 3);
            break;
        case 18:
            Wly_Formation_STAGGER(this, 4, 4);
            break;
        case 22:
            Wly_Formation_STAGGER(this, 5, 4);
            break;
        case 27:
            Wly_Formation_STAGGER(this, 5, 5);
            break;
        case 32:
            Wly_Formation_STAGGER(this, 6, 5);
            break;
        case 37:
            Wly_Formation_STAGGER(this, 7, 5);
            break;
        case 45:
            Wly_Formation_STAGGER(this, 7, 6);
            break;
        case 51:
            Wly_Formation_STAGGER(this, 8, 6);
            break;
        case 59:
            Wly_Formation_STAGGER(this, 8, 7);
            break;
        default:
            if (num <= 27) {
                Wly_Formation_STAGGER(this, 5, 5);
            } else if (num <= 52) {
                Wly_Formation_STAGGER(this, 7, 7);
            } else if (num <= 85) {
                Wly_Formation_STAGGER(this, 9, 9);
            } else {
                Wly_Formation_STAGGER(this, 11, 11);
            }
            break;
        }

        f32 shift = -mPositions[0].z;
        for (s32 i = 0; i < 100; i++) {
            mPositions[i].z += shift;
            mPositions[i].y = 2.5f * -mPositions[i].z / 6.25f;
        }
    }

    for (s32 i = 0; i < num; i++) {
        RPWlyAvatar* pAvatar = RPWlyAvatarMgr::sInstance->mpAvatars[i];
        nw4r::math::VEC3 pos = mPositions[order[i]];
        pAvatar->mPos.x = pos.x;
        pAvatar->mPos.y = pos.y;
        pAvatar->mPos.z = pos.z;
    }
}

// The position is passed by value (a stack copy), as in the original.
inline void SetAvatarPos(RPWlyAvatar* pAvatar, nw4r::math::VEC3 pos) {
    pAvatar->mPos.x = pos.x;
    pAvatar->mPos.y = pos.y;
    pAvatar->mPos.z = pos.z;
}

// Argument evaluation order keeps the position operand in the original float register.
inline f32 Wly_RandomSubtractReverse(f32 b, f32 a) { return a - b; }

void RPWlyStage::PlaceAvatarsRandom() {
    mClampX = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpeed;
    mClampZ = RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20;
    f32 rangeX = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpeed - sAvatarSpacing[0];
    f32 rangeZ = RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20 - sAvatarSpacing[0];
    s32 num = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;

    for (s32 i = 0; i < num; i++) {
        RPWlyAvatar* pZero = Wly_GetAvatar(i);
        nw4r::math::VEC3 zero = RPSimpleModel::sVecZero;
        SetAvatarPos(pZero, zero);
    }

    for (s32 i = 0; i < num; i++) {
        RPWlyAvatar* pAvatar = Wly_GetAvatar(i);
        s32 tries = 0;
    retry:
        f32 spacing = sAvatarSpacing[0];
        nw4r::math::VEC3 pos = pAvatar->mPos;
        if (tries < 1000) {
            pos.x = 2.0f * rangeX * RPUtlRandom::getF32() - rangeX;
            pos.z = 2.0f * rangeZ * RPUtlRandom::getF32() - rangeZ;
            if (mTargetNum != 1 && i < mTargetNum && tries < 500) {
                f32 r = RPUtlRandom::getF32();
                pos.x = 2.0f * rangeX * r / mTargetNum - rangeX * (1.0f - 2.0f / mTargetNum * i);
            }
            for (s32 j = 0; j < num; j++) {
                if (i != j) {
                    nw4r::math::VEC3 other = Wly_GetAvatar(j)->mPos;
                    f32 dz = Wly_RandomSubtractReverse(pos.z, other.z);
                    f32 dx = Wly_RandomSubtractReverse(pos.x, other.x);
                    f64 absX = __fabs(dx);
                    if ((f32)absX < spacing && (f32)__fabs(dz) < sAvatarSpacing[0] * 3.0f) {
                        tries++;
                        goto retry;
                    }
                }
            }
        } else {
            pos.x = 10.0f * rangeX;
            pos.z = 10.0f * rangeZ;
            pAvatar->mVisible = false;
            pAvatar->unkDC = 0;
        }
        SetAvatarPos(pAvatar, pos);
    }
}

void RPWlyStage::PlaceAvatarsSpaced() {
    mClampX = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpeed;
    mClampZ = RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20;
    f32 rangeX = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpeed;
    f32 rangeY = RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20;
    s32 num = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;

    for (s32 i = 0; i < num; i++) {
        RPWlyAvatar* pZero = Wly_GetAvatar(i);
        nw4r::math::VEC3 zero = RPSimpleModel::sVecZero;
        SetAvatarPos(pZero, zero);
    }

    for (s32 i = 0; i < num; i++) {
        RPWlyAvatar* pAvatar = Wly_GetAvatar(i);
        s32 tries = 0;
    retry:
        nw4r::math::VEC3 pos = pAvatar->mPos;
        if (tries < 1000) {
            pos.x = 2.0f * rangeX * RPUtlRandom::getF32() - rangeX;
            pos.y = 2.0f * rangeY * RPUtlRandom::getF32() - rangeY;
            for (s32 j = 0; j < num; j++) {
                if (i != j) {
                    nw4r::math::VEC3 other = Wly_GetAvatar(j)->mPos;
                    f32 dx = other.x - pos.x;
                    f32 dy = other.y - pos.y;
                    if (dx * dx + dy * dy < sAvatarSpacing[0] * 3.0f * sAvatarSpacing[0] * 3.0f) {
                        tries++;
                        goto retry;
                    }
                }
            }
        } else {
            pos.x = 10.0f * rangeX;
            pos.y = 10.0f * rangeY;
            pAvatar->mVisible = false;
            pAvatar->unkDC = 0;
        }
        SetAvatarPos(pAvatar, pos);
    }
}


// Starts a body animation at a random frame
inline void Wly_StartRandomFrame(RPWlyAvatar* pAvatar, u16 idx) {
    RPGrpModelAnm* pAnm = pAvatar->mpBodyModel->GetModelAnm();
    f32 r = RPUtlRandom::getF32();
    f32 numFrame = pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, idx);
    f32 frame = numFrame * r;
    pAnm->Start(RPGrpModelAnm::Anm_Chr, idx, 0.0f);
    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, frame);
}

void RPWlyStage::PlacePattern0() {
    mTargetIdx = 0;
    PlaceAvatarsFormation();

    s32 num = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;
    for (s32 i = 0; i < num; i++) {
        RPWlyAvatar* p = RPWlyAvatarMgr::sInstance->mpAvatars[i];
        p->mSwayX = p->mSwaySpeedX = 0.0f;
    }

    u32 side = RPUtlRandom::getU32(2);
    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 5:
        for (s32 i = 0; i < num; i++) {
            RPWlyAvatar* p = Wly_GetAvatarDirect(i);
            f32 v;
            if ((IsTargetNotFound(i) && side == 0) || (!IsTargetNotFound(i) && side != 0)) {
                v = 0.5235988f * RPUtlRandom::getF32() + 0.5235988f;
            } else {
                v = -(0.5235988f * RPUtlRandom::getF32() + 0.5235988f);
            }
            p->mSwayX = p->mSwaySpeedX = v;
        }
    default:
        for (s32 i = 0; i < num; i++) {
            Wly_StartRandomFrame(Wly_GetAvatar(i), 0);
        }
        break;
    case 14:
    case 15:
    case 16:
        for (s32 i = 0; i < num; i++) {
            if (IsTargetNotFound(i)) {
                Wly_StartRandomFrame(Wly_GetAvatar(i), 12);
            } else {
                Wly_StartRandomFrame(Wly_GetAvatar(i), 0);
            }
        }
        break;
    case 17:
    case 18:
    case 19:
        for (s32 i = 0; i < num; i++) {
            if (IsTargetNotFound(i)) {
                Wly_StartRandomFrame(Wly_GetAvatar(i), 0);
            } else {
                Wly_StartRandomFrame(Wly_GetAvatar(i), 12);
            }
        }
        break;
    }
}

#pragma opt_propagation off
void RPWlyStage::CalcMotionPattern0() {
    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        RPGrpModelAnm* pAnm = RPWlyAvatarMgr::sInstance->mpAvatars[i]->mpBodyModel->GetModelAnm();
        s32 anmIdx;
        if (IsTargetFoundInPlay(i)) {
            anmIdx = 3;
            if (anmIdx != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, 3, 10.0f, 30.0f);
            }
        } else if (mHover[i].mHovered) {
            anmIdx = 4;
            if (anmIdx != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                mHover[i].unk0 = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, 4, 10.0f, 30.0f);
            }
        } else {
            anmIdx = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
            if (anmIdx == 4) {
                anmIdx = mHover[i].unk0;
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, anmIdx, 10.0f,
                                 pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, anmIdx));
            }
        }

        f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, anmIdx);
        if (frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, anmIdx) - 1.0f) {
            pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, anmIdx, 1.0f + frame);
        } else {
            pAnm->Start(RPGrpModelAnm::Anm_Chr, anmIdx, 0.0f);
        }
    }
}
#pragma opt_propagation reset

void RPWlyStage::PlacePattern1() {
    mTargetIdx = 0;
    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
    case 1:
        PlaceAvatarsFormation();
        break;
    case 3:
        PlaceAvatarsRandom();
        break;
    }

    s32 num = Wly_GetAvatarNum();
    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 9:
    case 10:
    case 11:
        mPatternTimer = 0;
        mPatternFlip = RPUtlRandom::getU32(2);
        mPatternStep = 0;
        for (s32 i = 0; i < num; i++) {
            RPWlyAvatar* p = Wly_GetAvatarDirect(i);
            f32 v;
            if ((IsTargetNotFound(i) && mPatternFlip == 0) || (!IsTargetNotFound(i) && mPatternFlip != 0)) {
                v = 0.5235988f;
            } else {
                v = -0.5235988f;
            }
            p->mSwaySpeedX = p->mSwayX = v;
        }
    default:
        for (s32 i = 0; i < num; i++) {
            Wly_StartRandomFrame(RPWlyAvatarMgr::sInstance->mpAvatars[i], 0);
        }
        break;
    case 14:
    case 15:
    case 16:
        for (s32 i = 0; i < num; i++) {
            if (IsTargetNotFound(i)) {
                Wly_StartRandomFrame(RPWlyAvatarMgr::sInstance->mpAvatars[i], 12);
            } else {
                Wly_StartRandomFrame(RPWlyAvatarMgr::sInstance->mpAvatars[i], 0);
            }
        }
        break;
    case 17:
    case 18:
    case 19:
        for (s32 i = 0; i < num; i++) {
            if (IsTargetNotFound(i)) {
                Wly_StartRandomFrame(RPWlyAvatarMgr::sInstance->mpAvatars[i], 0);
            } else {
                Wly_StartRandomFrame(RPWlyAvatarMgr::sInstance->mpAvatars[i], 12);
            }
        }
        break;
    }
}

inline u32 Wly_calcRand() {
    RPUtlRandom::sRandom.mSeed = RPUtlRandom::sRandom.mSeed * RPUtlRandom::MULT + 1;
    return RPUtlRandom::sRandom.mSeed;
}

inline f32 Wly_randF32() {
    return static_cast<f32>(static_cast<u16>(RPUtlRandom::RANDF_MAX & (Wly_calcRand() >> RPUtlRandom::RANDF_SHIFT))) /
           static_cast<f32>(RPUtlRandom::RANDF_MAX + 1);
}

inline u32 Wly_randU32(u32 max) {
    return max * Wly_randF32();
}

void RPWlyStage::CalcMotionPattern1() {
    RPWlyAvatar* p;
    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 9:
    case 10:
    case 11:
        if (RPWlyGameMgr::sInstance->mState != 4 && ++mPatternTimer >= 90) {
            mPatternTimer = 0;
            mPatternFlip = !mPatternFlip;
            if (mPatternStep < 4) {
                if (!mPatternFlip) {
                    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
                    case 0:
                        RPSndAudioMgr::instance()->startSystemSe(0x35);
                        break;
                    case 3:
                        RPSndAudioMgr::instance()->startSystemSe(0x3F);
                        break;
                    }
                } else {
                    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
                    case 0:
                        RPSndAudioMgr::instance()->startSystemSe(0x34);
                        break;
                    case 3:
                        RPSndAudioMgr::instance()->startSystemSe(0x3E);
                        break;
                    }
                }
            } else {
                if (!mPatternFlip) {
                    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
                    case 0:
                        RPSndAudioMgr::instance()->startSystemSe(0x37);
                        break;
                    case 3:
                        RPSndAudioMgr::instance()->startSystemSe(0x41);
                        break;
                    }
                } else {
                    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
                    case 0:
                        RPSndAudioMgr::instance()->startSystemSe(0x36);
                        break;
                    case 3:
                        RPSndAudioMgr::instance()->startSystemSe(0x40);
                        break;
                    }
                }
            }
            if (++mPatternStep >= 4) {
                mPatternStep = 4;
            }
        }
        break;
    }

    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        p = Wly_GetAvatar(i);
        switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
        case 9:
        case 10:
        case 11:
            if (mPatternTimer == 0) {
                if ((IsTargetNotFound(i) && mPatternFlip == 0) ||
                    (!IsTargetNotFound(i) && mPatternFlip != 0)) {
                    p->mSwaySpeedX = 0.5235988f;
                } else {
                    p->mSwaySpeedX = -0.5235988f;
                }
            }
            break;
        default:
            if (Wly_randU32(30) == 0) {
                if (Wly_randU32(2) != 0) {
                    p->mSwaySpeedX = atan2(-1.0, 1.0);
                } else {
                    p->mSwaySpeedX = atan2(1.0, 1.0);
                }
            }
            break;
        case 14:
        case 15:
        case 16:
            if (!IsTargetNotFound(i)) {
                if (Wly_randU32(30) == 0) {
                    if (Wly_randU32(2) != 0) {
                        p->mSwaySpeedX = atan2(-1.0, 1.0);
                    } else {
                        p->mSwaySpeedX = atan2(1.0, 1.0);
                    }
                }
            }
            break;
        case 17:
        case 18:
        case 19:
            if (IsTargetNotFound(i)) {
                if (Wly_randU32(30) == 0) {
                    if (Wly_randU32(2) != 0) {
                        p->mSwaySpeedX = atan2(-1.0, 1.0);
                    } else {
                        p->mSwaySpeedX = atan2(1.0, 1.0);
                    }
                }
            }
            break;
        }

        RPGrpModelAnm* pAnm = p->mpBodyModel->GetModelAnm();
        s32 anmIdx;
        if (IsTargetFoundInPlay(i)) {
            if ((anmIdx = 3) != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, 3, 10.0f, 30.0f);
            }
            p->mSwaySpeedX = 0.0f;
        } else if (mHover[i].mHovered) {
            if ((anmIdx = 4) != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                mHover[i].unk0 = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, 4, 10.0f, 30.0f);
            }
            p->mSwaySpeedX = 0.0f;
        } else {
            anmIdx = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
            if (anmIdx == 4) {
                anmIdx = mHover[i].unk0;
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, anmIdx, 10.0f,
                                 pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, anmIdx));
            }
        }

        f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, anmIdx);
        if (frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, anmIdx) - 1.0f) {
            pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, anmIdx, 1.0f + frame);
        } else {
            pAnm->Start(RPGrpModelAnm::Anm_Chr, anmIdx, 0.0f);
        }
    }
}

// Starts body animation idx at a random frame
inline void StartAnmAtRandomFrame(RPWlyAvatar* pAvatar, u16 idx) {
    RPGrpModelAnm* pAnm = pAvatar->mpBodyModel->GetModelAnm();
    f32 frame = pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, idx) * RPUtlRandom::getF32();
    pAnm->Start(RPGrpModelAnm::Anm_Chr, idx, 0.0f);
    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, frame);
}

void RPWlyStage::PlacePattern2() {
    mTargetIdx = 0;

    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
    case 6:
    case 7: {
        f32 unk20 = RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20;
        RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20 = 55.0f;
        PlaceAvatarsFormation();
        RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpeed = mClampX;
        RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20 = unk20;
        break;
    }
    default:
        PlaceAvatarsRandom();
        break;
    }

    RPWlyAvatar* pAvatar;
    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        pAvatar = Wly_GetAvatar(i);
        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1) {
            StartAnmAtRandomFrame(Wly_GetAvatar(i), 5);
        } else {
            switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
            case 6:
            case 7:
                StartAnmAtRandomFrame(Wly_GetAvatar(i), 0);
                break;
            default:
                switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
                case 12:
                    if (IsTargetNotFound(i)) {
                        StartAnmAtRandomFrame(Wly_GetAvatar(i), 2);
                    } else {
                        StartAnmAtRandomFrame(Wly_GetAvatar(i), 1);
                    }
                    break;
                case 13:
                    if (IsTargetNotFound(i)) {
                        StartAnmAtRandomFrame(Wly_GetAvatar(i), 2);
                    } else if (i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum * 2 / 3) {
                        StartAnmAtRandomFrame(Wly_GetAvatar(i), 2);
                    } else {
                        StartAnmAtRandomFrame(Wly_GetAvatar(i), 1);
                    }
                    break;
                default:
                    StartAnmAtRandomFrame(Wly_GetAvatar(i), 1);
                    break;
                }
                break;
            }
        }

        if ((s32)RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4 == 4) {
            f32 sway;
            if (RPUtlRandom::getU32(2) != 0) {
                sway = 0.3926991f * RPUtlRandom::getF32() + 0.3926991f;
            } else {
                sway = -(0.3926991f * RPUtlRandom::getF32() + 0.3926991f);
            }
            pAvatar->mSwayZ = sway;
            pAvatar->mSwaySpeedZ = sway;
        }
    }
}

void RPWlyStage::CalcMotionPattern2() {
    s32 avatarNum = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;

    for (s32 i = 0; i < avatarNum; i++) {
        RPWlyAvatar* pAvatar = Wly_GetAvatarDirect(i);
        RPGrpModelAnm* pAnm = pAvatar->mpBodyModel->GetModelAnm();
        s32 idx;

        if (RPWlyGameMgr::sInstance->mState != 1) {
            f32 amp;
            nw4r::math::VEC3 pos = pAvatar->mPos;
            f32 angle = Wly_GetAvatarAngle(pAvatar);
            f32 speed;

            if (IsTargetFoundInPlay(i) || mHover[i].mHovered) {
                speed = 0.0f;
            } else if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1) {
                f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr));
                if (15.0f <= frame && frame <= 72.0f) {
                    speed = 0.1754386f;
                } else {
                    speed = 0.0f;
                    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 12 && IsTargetNotFound(i) &&
                        RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1) {
                        speed = 0.029239766f;
                    }
                }

                RPWlyGround* pGround = RPWlyGameMgr::sInstance->mpField->mpGround;
                f32 period = pGround->unk4C;
                amp = pGround->unk48;
                pos.y = amp * (f32)cos(6.2831855f * ((f32)pGround->mFrame / (f32)pGround->unk50 + pos.z / period)) - amp;
                SetAvatarPos(pAvatar, pos);
            } else if (pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr) == 2) {
                speed = 0.33333334f;
            } else {
                speed = 0.16666667f;
            }

            if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 20) {
                if (RPWlyGameMgr::sInstance->mpWipe->mDone && unkC >= 10) {
                    if (unkC >= 30) {
                        speed *= 0.8f;
                    } else if (unkC > 20) {
                        speed *= 0.8f * ((f32)(unkC - 20) / 10.0f);
                    }
                } else {
                    speed = 0.0f;
                }
            }

            f32 dx = speed * (f32)sin(angle);
            f32 dz = speed * (f32)cos(angle);

            switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
            case 12:
                if (IsTargetNotFound(i) && RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1) {
                    dx *= 3.0f;
                    dz *= 3.0f;
                }
                break;
            case 13:
                if (IsTargetNotFound(i) && RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType != 1) {
                    dx *= 2.5f;
                    dz *= 2.5f;
                }
                break;
            }

            nw4r::math::VEC3 next = pos;
            next.x += dx;
            next.z += dz;

            f32 rangeX = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpeed;
            f32 rangeZ = RPWlyLevelTable::sInstance->GetCurrentLevel()->unk20;
            if (next.z < -rangeZ) {
                next.z += 2.0f * rangeZ;
            } else if (next.z > rangeZ) {
                next.z -= 2.0f * rangeZ;
            }

            bool move = true;
            for (s32 j = 0; j < avatarNum; j++) {
                if (i == j) {
                    continue;
                }

                nw4r::math::VEC3 other = RPWlyAvatarMgr::sInstance->mpAvatars[j]->mPos;
                f32 ox = other.x - next.x;
                f32 oz = other.z - next.z;
                f32 dist = ox * ox + oz * oz;

                if ((s32)RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4 == 4) {
                    if (dist < sAvatarSpacing[0] * sAvatarSpacing[0]) {
                        f32 dir;
                        if (Wly_randU32(2) != 0) {
                            dir = 0.3926991f * Wly_randF32() + 0.3926991f;
                        } else {
                            dir = -(0.3926991f * Wly_randF32() + 0.3926991f);
                        }
                        pAvatar->mSwaySpeedZ = dir;
                        move = false;
                        break;
                    }

                    if (next.x < -rangeX) {
                        move = false;
                        pAvatar->mSwaySpeedZ = 0.3926991f * Wly_randF32() + 0.3926991f;
                    } else if (next.x > rangeX) {
                        move = false;
                        pAvatar->mSwaySpeedZ = -(0.3926991f * Wly_randF32() + 0.3926991f);
                    }
                } else if (dist < sAvatarSpacing[0] * sAvatarSpacing[0]) {
                    f32 dir;
                    if (0.0f == pAvatar->mSwaySpeedZ) {
                        if (Wly_randU32(2) != 0) {
                            dir = 1.5707964f;
                        } else {
                            dir = -1.5707964f;
                        }
                    } else {
                        dir = 0.0f;
                    }
                    pAvatar->mSwaySpeedZ = dir;
                    move = false;
                } else if (next.x < -rangeX) {
                    pAvatar->mSwaySpeedZ = Wly_randU32(2) != 0 ? 1.5707964f : 0.0f;
                    move = false;
                } else if (next.x > rangeX) {
                    pAvatar->mSwaySpeedZ = Wly_randU32(2) != 0 ? -1.5707964f : 0.0f;
                    move = false;
                } else if (Wly_randU32(180) == 0 && 0.0f != pAvatar->mSwaySpeedZ) {
                    pAvatar->mSwaySpeedZ = 0.0f;
                    move = false;
                }
            }

            if (IsTargetFoundInPlay(i)) {
                pAvatar->mSwaySpeedZ = 0.0f;
                move = false;
                pAvatar->mSwayZ = 0.0f;
            }

            if (move) {
                SetAvatarPos(pAvatar, next);
            }
        }

        if (IsTargetFoundInPlay(i)) {
            idx = RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1 ? 7 : 3;
            if (idx != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, idx, 10.0f, 30.0f);
            }
        } else if (mHover[i].mHovered) {
            idx = RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1 ? 8 : 4;
            if (idx != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                mHover[i].unk0 = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, idx, 10.0f, 30.0f);
            }
        } else {
            idx = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
            s32 hover = RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1 ? 8 : 4;
            if (hover == idx) {
                idx = mHover[i].unk0;
                f32 frame;
                if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1) {
                    frame = 0.0f;
                } else {
                    frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, idx);
                }
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, idx, 10.0f, frame);
            }
        }

        f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, idx);
        if (frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, idx) - 1.0f) {
            if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1 &&
                RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 12 && IsTargetNotFound(i)) {
                if (IsTargetFoundInPlay(i) || mHover[i].mHovered) {
                    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 1.0f + frame);
                } else {
                    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 3.0f + frame);
                }
            }
            if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType != 1 &&
                RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 13 && IsTargetNotFound(i)) {
                if (IsTargetFoundInPlay(i) || mHover[i].mHovered) {
                    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 1.0f + frame);
                } else {
                    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 2.5f + frame);
                }
            } else if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 20) {
                if (IsTargetFoundInPlay(i) || mHover[i].mHovered) {
                    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 1.0f + frame);
                } else {
                    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 0.8f + frame);
                }
            } else {
                pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 1.0f + frame);
            }
        } else {
            if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1) {
                if (IsTargetFoundInPlay(i)) {
                    idx = 7;
                } else if (mHover[i].mHovered) {
                    idx = 8;
                } else if (idx != 7 && idx != 8) {
                    u32 r = Wly_randU32(10);
                    idx = 6;
                    if (r != 0) {
                        idx = 5;
                    }
                }
            }
            pAnm->Start(RPGrpModelAnm::Anm_Chr, idx, 0.0f);
        }

        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 20 && mHover[i].mHovered) {
            switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
            case 6:
                idx = 1;
                break;
            case 7:
                idx = 2;
                break;
            }
            f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, idx);
            if (frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, idx) - 1.0f) {
                pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 0.8f + frame);
            } else {
                pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 0.0f);
            }
        }
    }
}

void RPWlyStage::PlacePattern5() {
    mTargetIdx = 0;
    PlaceAvatarsSpaced();

    s32 avatarNum = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;
    for (s32 i = 0; i < avatarNum; i++) {
        RPWlyAvatar* pAvatar = RPWlyAvatarMgr::sInstance->mpAvatars[i];
        pAvatar->mSwayX = pAvatar->mSwaySpeedX = 0.0f;
    }

    for (s32 i = 0; i < avatarNum; i++) {
        RPGrpModelAnm* pAnm = RPWlyAvatarMgr::sInstance->mpAvatars[i]->GetBodyModel()->GetModelAnm();
        f32 r = RPUtlRandom::getF32();
        f32 num = pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, 9);
        f32 frame = num * r;
        pAnm->Start(RPGrpModelAnm::Anm_Chr, 9, 0.0f);
        pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, 9, frame);
    }
}

void RPWlyStage::CalcMotionPattern5() {
    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        RPGrpModelAnm* pAnm = RPWlyAvatarMgr::sInstance->mpAvatars[i]->GetBodyModel()->GetModelAnm();
        s32 anm;
        if (IsTargetFoundInPlay(i)) {
            if ((anm = 10) != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, anm, 10.0f, 30.0f);
            }
        } else if (mHover[i].mHovered) {
            if ((anm = 11) != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                mHover[i].unk0 = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, anm, 10.0f, 30.0f);
            }
        } else {
            anm = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
            if (anm == 11) {
                anm = mHover[i].unk0;
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, anm, 10.0f,
                                 pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, anm));
            }
        }

        f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, anm);
        if (frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, anm) - 1.0f) {
            pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, anm, 1.0f + frame);
        } else {
            pAnm->Start(RPGrpModelAnm::Anm_Chr, anm, 0.0f);
        }
    }
}

nw4r::math::VEC3 RPWlyStage::GetEscalatorSlotPos(s32 idx) {
    const char* names[] = {"esc1", "esc2", "esc3"};
    s32 node = mNodes[idx].mNode;
    s32 row;
    if (node < 14) {
        row = 0;
    } else {
        row = node < 28 ? 1 : 2;
    }

    RPGrpModel* pModel = RPWlyGameMgr::sInstance->mpField->GetModel();
    f32 frame = pModel->GetModelAnm()->GetFrame(RPGrpModelAnm::Anm_Chr, row);
    s32 col = node % 14;
    if (frame > 32.0f * col) {
        col += 14;
    }

    char name[256];
    snprintf(name, sizeof(name), "%s_%02d", names[(u16)row], col + 1);
    nw4r::math::MTX34 mtx;
    pModel->GetWorldMtx(pModel->GetJointIndex(name), &mtx);

    nw4r::math::VEC3 pos;
    pos.x = mtx._03 + mNodes[node].mOffset;
    pos.y = mtx._13;
    pos.z = mtx._23;
    return pos;
}

void RPWlyStage::PlaceEscalator() {
    s32 avatarNum = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;

    for (s32 i = 0; i < 42; i++) {
        f32 half = sAvatarSpacing[0] / 2.0f;
        mNodes[i].mNode = i;
        if (i % 2 != 0) {
            mNodes[i].mOffset = half * RPUtlRandom::getF32();
        } else {
            mNodes[i].mOffset = -half * RPUtlRandom::getF32();
        }
    }

    for (s32 i = 41; i > 0; i--) {
        u32 j = RPUtlRandom::getU32(i + 1);
        s32 tmp = mNodes[j].mNode;
        mNodes[j].mNode = mNodes[i].mNode;
        mNodes[i].mNode = tmp;
    }

    // The first three avatars stand on three different rows
    s32 row0 = mNodes[0].mNode / 14;
    u32 r = RPUtlRandom::getU32(2);
    s32 row;
    switch (row0) {
    case 0:
        row = r != 0 ? 1 : 2;
        break;
    case 1:
        row = r != 0 ? 0 : 2;
        break;
    case 2:
        row = r != 0 ? 0 : 1;
        break;
    }

    s32 i;
    for (i = 41; i > 1; i--) {
        if (row == mNodes[i].mNode / 14) {
            break;
        }
    }
    s32 tmp = mNodes[i].mNode;
    mNodes[i].mNode = mNodes[1].mNode;
    mNodes[1].mNode = tmp;

    row0 = mNodes[0].mNode / 14;
    s32 row1 = mNodes[1].mNode / 14;
    s32 row2;
    for (row2 = 0; row2 < 3; row2++) {
        if (row2 != row0 && row2 != row1) {
            break;
        }
    }

    for (i = 41; i > 2; i--) {
        if (row2 == mNodes[i].mNode / 14) {
            break;
        }
    }
    tmp = mNodes[i].mNode;
    mNodes[i].mNode = mNodes[2].mNode;
    mNodes[2].mNode = tmp;

    for (s32 i = 0; i < avatarNum; i++) {
        RPGrpModelAnm* pAnm = RPWlyAvatarMgr::sInstance->mpAvatars[i]->GetBodyModel()->GetModelAnm();
        f32 r = RPUtlRandom::getF32();
        f32 num = pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, 0);
        f32 frame = num * r;
        pAnm->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
        pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, 0, frame);
    }
}

void RPWlyStage::CalcMotionEscalator() {
    RPWlyAvatar* pAvatar;
    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        pAvatar = Wly_GetAvatar(i);
        switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
        case 1:
        case 3:
            if (Wly_randU32Value(30) == 0) {
                if (Wly_randU32Value(2) != 0) {
                    pAvatar->mSwaySpeedX = std::atan2(-1.0, 1.0);
                } else {
                    pAvatar->mSwaySpeedX = std::atan2(1.0, 1.0);
                }
            }
            break;
        }

        nw4r::math::VEC3 pos = GetEscalatorSlotPos(i);
        SetAvatarPos(pAvatar, pos);

        RPGrpModelAnm* pAnm = pAvatar->GetBodyModel()->GetModelAnm();
        s32 anm;
        if (IsTargetFoundInPlay(i)) {
            if ((anm = 3) != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, anm, 10.0f, 30.0f);
            }
            pAvatar->mSwaySpeedX = 0.0f;
        } else if (mHover[i].mHovered) {
            if ((anm = 4) != pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr)) {
                mHover[i].unk0 = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, anm, 10.0f, 30.0f);
            }
            pAvatar->mSwaySpeedX = 0.0f;
        } else {
            anm = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
            if (anm == 4) {
                anm = mHover[i].unk0;
                pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, anm, 10.0f,
                                 pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, anm));
            }
        }

        f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, anm);
        if (frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, anm) - 1.0f) {
            pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, anm, 1.0f + frame);
        } else {
            pAnm->Start(RPGrpModelAnm::Anm_Chr, anm, 0.0f);
        }
    }
}

void RPWlyStage::Reset() {
    if (!mActive) {
        RPWlyAvatarMgr::sInstance->mpQuestionAvatar->SetDrawScene(4);

        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 4) {
            PlaceEscalator();
        } else {
            switch ((s32)RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
            case 0:
                PlacePattern0();
                break;
            case 1:
            case 3:
                PlacePattern1();
                break;
            case 2:
            case 4:
            case 6:
            case 7:
                PlacePattern2();
                break;
            case 5:
                PlacePattern5();
                break;
            }
        }

        switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
        case 2:
        case 4:
            RPWlyGameMgr::sInstance->mpCamera->SetTarget(
                0.0f, RPWlyLevelTable::sInstance->GetCurrentLevel()->mCameraTarget, 0.0f);
            break;
        default:
            RPWlyGameMgr::sInstance->mpCamera->SetTarget(
                0.0f, 0.0f, RPWlyLevelTable::sInstance->GetCurrentLevel()->mCameraTarget);
            break;
        }

        RPWlyGameMgr::sInstance->mpCamera->SetDistance(RPWlyLevelTable::sInstance->GetCurrentLevel()->unk10);
        mActive = true;
    }
}

void RPWlyStage::ClampPosition(nw4r::math::VEC3* pPos) {
    f32 limitX = mClampX * 1.5f;
    f32 limitZ = mClampZ * 1.5f;

    if (pPos->x < -limitX) {
        pPos->x = -limitX;
    } else if (pPos->x > limitX) {
        pPos->x = limitX;
    }

    if ((s32)RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4 == 5) {
        if (pPos->y < -limitZ) {
            pPos->y = -limitZ;
        } else if (pPos->y > limitZ) {
            pPos->y = limitZ;
        }
    } else {
        if (pPos->z < -limitZ) {
            pPos->z = -limitZ;
        } else if (pPos->z > limitZ) {
            pPos->z = limitZ;
        }
    }
}

void RPWlyStage::Calculate() {
    if (RPWlyGameMgr::sInstance->mpWipe->mDone) {
        if (unkC == 10) {
            switch ((s32)RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
            case 6:
                for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
                    RPGrpModelAnm* pAnm =
                        Wly_GetAvatarDirect(i)->mpBodyModel->GetModelAnm();
                    f32 frame = 0.0f;
                    if (IsTargetNotFound(i)) {
                        frame = pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, 1) / 2.0f;
                    }
                    pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, 1, 10.0f, frame);
                }
                break;
            case 7:
                for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
                    RPGrpModelAnm* pAnm =
                        Wly_GetAvatarDirect(i)->mpBodyModel->GetModelAnm();
                    f32 frame = 0.0f;
                    if (IsTargetNotFound(i)) {
                        frame = pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, 2) / 2.0f;
                    }
                    pAnm->StartBlend(RPGrpModelAnm::Anm_Chr, 2, 10.0f, frame);
                }
                break;
            }
        }

        if (++unkC >= 30) {
            unkC = 30;
        }
    }

    if (RPWlyGameMgr::sInstance->mState != 0 && RPWlyGameMgr::sInstance->mState != 1) {
        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 4) {
            CalcMotionEscalator();
        } else {
            switch ((s32)RPWlyLevelTable::sInstance->GetCurrentLevel()->unk4) {
            case 0:
                CalcMotionPattern0();
                break;
            case 1:
            case 3:
                CalcMotionPattern1();
                break;
            case 2:
            case 4:
            case 6:
            case 7:
                CalcMotionPattern2();
                break;
            case 5:
                CalcMotionPattern5();
                break;
            }
        }
    }

    if (RPWlyLevelTable::sInstance->HasAltMessage()) {
        nw4r::math::VEC3 base(0.0f, 0.0f, RPWlyLevelTable::sInstance->GetCurrentLevel()->mCameraTarget);
        switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
        case 2:
        case 4:
            base.x = 0.0f;
            base.y = RPWlyLevelTable::sInstance->GetCurrentLevel()->mCameraTarget;
            base.z = 0.0f;
            break;
        }

        nw4r::math::VEC3 hit;
        EGG::Vector2f pos = RPWlyGameMgr::sInstance->mpCursors[0]->GetPos();
        RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
        nw4r::math::VEC3 point(0.0f, 0.0f, 0.0f);
        nw4r::math::VEC3 normal(0.0f, 1.0f, 0.0f);
        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 2) {
            normal.x = 0.0f;
            normal.y = 0.0f;
            normal.z = 1.0f;
        }

        RPGrpCamera* pCamera = Wly_GetCamera();
        bool valid = pCamera->GetScreenToPlane(
            &hit, point, normal, pScreen, pos.x * (f32)(u16)(EGG::Screen::GetSizeXMax() / 2),
            pos.y * (f32)(u16)(EGG::Screen::GetSizeYMax() / 2));
        f32 dist = RPWlyGameMgr::sInstance->mpCursors[0]->mDistRatio;
        if (valid) {
            nw4r::math::VEC3 cur = pCamera->GetTarget();
            nw4r::math::VEC3 target = base + (hit - base) * (1.0f - dist);
            target = cur + (target - cur) * Wly_Constant(0.01f);
            ClampPosition(&target);
            pCamera->SetTarget(target);

            if (RPWlyLevelTable::sInstance->HasAltMessage() && dist != mMotionValue) {
                mMotionTimer++;
                if (dist < mMotionValue - 0.025f) {
                    if (mMotionTimer > 30) {
                        RPSndAudioMgr::instance()->startSystemSe(5u);
                        mMotionTimer = 0;
                    }
                    mMotionValue = dist;
                } else if (dist > mMotionValue + 0.025f) {
                    if (mMotionTimer > 30) {
                        RPSndAudioMgr::instance()->startSystemSe(6u);
                        mMotionTimer = 0;
                    }
                    mMotionValue = dist;
                }
            }
        }

        RPWlyLevelTable* pTable = RPWlyLevelTable::sInstance;
        pCamera->SetDistance(pTable->GetCurrentLevel()->unk14 +
                             dist * (pTable->GetCurrentLevel()->unk10 - pTable->GetCurrentLevel()->unk14));
    }

    if (RPWlyGameMgr::sInstance->mState != 0 && RPWlyGameMgr::sInstance->mState != 1) {
        RPWlyAvatar* pAvatar;
        RPGrpModelAnm* pAnm;
        for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
            pAvatar = Wly_GetAvatarDirect(i);
            pAnm = pAvatar->mpBodyModel->GetModelAnm();
            f32 weight = pAnm->GetBlendWeight(RPGrpModelAnm::Anm_Chr, 1, 0) - 0.1f;
            if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 20 && unkC < 30 &&
                (u32)(pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr) - 1) <= 1) {
                weight += 0.05f;
            }
            if (weight < 0.0f) {
                weight = 0.0f;
                pAnm->RemoveBlendAnm(RPGrpModelAnm::Anm_Chr, 1, 0);
            }
            pAnm->SetBlendWeight(RPGrpModelAnm::Anm_Chr, 0, 1.0f - weight, 0);
            pAnm->SetBlendWeight(RPGrpModelAnm::Anm_Chr, 1, weight, 0);

            bool found = RPWlyGameMgr::sInstance->mState == 3 && IsTargetFound(i);
            if (found) {
                RPSysKokeshiManager::ChangeExpression(pAvatar->mpNigaoeModel, (RFLExpression)1);
            } else if (pAvatar->mReaction == 1) {
                RPSysKokeshiManager::ChangeExpression(pAvatar->mpNigaoeModel, (RFLExpression)3);
            } else if (pAvatar->mpBodyModel->GetModelAnm()->GetPlayIndex(RPGrpModelAnm::Anm_Chr) == 12) {
                RPSysKokeshiManager::ChangeExpression(pAvatar->mpNigaoeModel, (RFLExpression)5);
            } else {
                RPSysKokeshiManager::ChangeExpression(pAvatar->mpNigaoeModel, (RFLExpression)0);
            }
            pAvatar->CalcModelPose();
        }
    }

    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 0:
    case 8: {
        RPWlyAvatar* pQuestion = RPWlyAvatarMgr::sInstance->mpQuestionAvatar;
        SetAvatarPos(pQuestion, RPSimpleModel::sVecZero);
        pQuestion->mSwayX = pQuestion->mSwaySpeedX = 0.0f;
        pQuestion->CalcModelPose();
        break;
    }
    }

    UpdateHoverTimers();
}

bool RPWlyStage::IsTargetFoundInPlay(s32 idx) const {
    return RPWlyGameMgr::sInstance->mState == 3 || IsTargetFound(idx);
}

bool RPWlyStage::IsTargetNotFound(s32 idx) const {
    bool result = false;
    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 6:
        if (idx != -1 && !mFound[0]) {
            result = true;
        }
        break;
    default:
        if (idx != -1 && idx < mTargetNum && !mFound[idx]) {
            result = true;
        }
        break;
    }
    return result;
}

bool RPWlyStage::IsTargetFound(s32 idx) const {
    bool result = false;
    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 6:
        if (idx != -1 && mFound[0]) {
            result = true;
        }
        break;
    default:
        if (idx != -1 && idx < mTargetNum && mFound[idx]) {
            result = true;
        }
        break;
    }
    return result;
}

void RPWlyStage::MarkTargetFound(s32 idx) {
    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 6:
        if (idx != -1 && !mFound[0]) {
            mFound[0] = true;
            mFoundNum++;
            mTargetIdx = idx;
        }
        break;
    default:
        if (idx != -1 && idx < mTargetNum && !mFound[idx]) {
            mFound[idx] = true;
            mFoundNum++;
        }
        break;
    }
}

bool RPWlyStage::IsAllFound() const {
    return mFoundNum >= mTargetNum;
}

s32 RPWlyStage::GetPointValue() {
    s32 value = 1;
    if (RPWlyGameMgr::sInstance->mMode == 1) {
        s32 points[5][5] = {
            {10, 0, 0, 0, 0},
            {10, 5, 0, 0, 0},
            {10, 5, 3, 0, 0},
            {10, 7, 5, 3, 0},
            {10, 7, 5, 3, 1},
        };
        s32 row = mTargetNum - 1;
        value = points[row][mFoundNum];
    }
    return value;
}

RPSysKokeshiGenInfo RPWlyAvatarMgr::GetGenInfo(s32 idx) {
    return mGenInfos[idx];
}

RPSysKokeshiOverloadInfo RPWlyAvatarMgr::GetOverloadInfo(s32 idx) {
    return mOverloadInfos[idx];
}

void RPWlyStage::RecordFoundMii(s32 idx) {
    if (idx != -1) {
        RPWlyGameConfig config = RPSceneInfo::spInstance->mMiiEntry.GetConfig();
        config.mGenInfo = RPWlyAvatarMgr::sInstance->GetGenInfo(idx);
        config.mOverloadInfo = RPWlyAvatarMgr::sInstance->GetOverloadInfo(idx);
        RPSceneInfo::spInstance->mMiiEntry.SetConfig(config);
    }
}

void RPWlyStage::CheckMultiHoverPick(s32 idx) {
    if (idx < 0 || idx >= 100) {
        return;
    }
    {
        s32 pickedNum = 0;
        s32 correctNum = 0;
        for (s32 i = 0; i < 100; i++) {
            if (mHover[i].mHovered && mHover[i].mTimer >= 30) {
                pickedNum++;
                if (IsTargetNotFound(i)) {
                    correctNum++;
                }
            }
        }

        bool allPicked = pickedNum == mTargetNum;
        bool allCorrect = correctNum == mTargetNum;
        if (allPicked) {
            for (s32 i = 0; i < 100; i++) {
                if (mHover[i].mHovered) {
                    RPWlyGameMgr::sInstance->OnPickResolved(i, allCorrect);
                    if (!allCorrect) {
                        mHover[i].mHovered = false;
                    }
                }
            }
        }

        if (allPicked) {
            if (allCorrect) {
                RPWlyAvatarMgr::sInstance->mpAvatars[idx]->mpSound->PlayVoice(1, RPWlyGameMgr::sInstance->mCurPlayer + 1);
                if (RPWlyGameMgr::sInstance->mMode == 0) {
                    RPWlyLayoutSet::sInstance->mpPointPopups->ShowNext(0, idx, 10);
                }
            } else {
                RPWlyAvatarMgr::sInstance->mpAvatars[idx]->mpSound->PlayVoice(0, RPWlyGameMgr::sInstance->mCurPlayer + 1);
                RPWlyLayoutSet::sInstance->mpPointPopups->ShowNext(0, idx, -5);
            }
        }
    }
}

bool RPWlyStage::IsAvatarHovered(s32 idx) const {
    return mHover[idx].mHovered;
}

void RPWlyStage::UpdateHoverTimers() {
    for (s32 i = 0; i < 100; i++) {
        if (mHover[i].mHovered) {
            if (RPWlyGameMgr::sInstance->Is1pTypeAStage()) {
                if (++mHover[i].mTimer > 30) {
                    mHover[i].mTimer = 31;
                }
                if (RPWlyGameMgr::sInstance->mTimer == 1 && mHover[i].mTimer < 30) {
                    mHover[i].mTimer = 30;
                }
                if (mHover[i].mTimer == 30) {
                    RPWlyGameMgr::sInstance->OnAvatarPicked(i, mHover[i].mPlayer);
                }
            } else {
                if (RPWlyGameMgr::sInstance->mTimer == 1 && mHover[i].mTimer < 30) {
                    mHover[i].mTimer = 30;
                }
                if (++mHover[i].mTimer >= 30) {
                    mHover[i].mTimer = 10;
                    mHover[i].mHovered = false;
                    RPWlyGameMgr::sInstance->OnAvatarPicked(i, mHover[i].mPlayer);
                }
            }
        } else if (--mHover[i].mTimer < 0) {
            mHover[i].mTimer = 0;
        }
    }
}

void RPWlyStage::UpdateAvatarHoverReactions(s32 idx, s32 player) {
    if (RPWlyGameMgr::sInstance->mTimer > 2 && idx >= 0 && idx < 100) {
        s32 playerHoverNum = 0;
        s32 busyNum = 0;
        for (s32 i = 0; i < 100; i++) {
            if (mHover[i].mHovered && player == mHover[i].mPlayer) {
                playerHoverNum++;
            }
            if (IsTargetFoundInPlay(i) || (IsTargetNotFound(i) && mHover[i].mHovered)) {
                busyNum++;
            }
        }

        if (busyNum < mTargetNum && playerHoverNum < mTargetNum) {
            if (RPWlyGameMgr::sInstance->Is1pTypeAStage()) {
                if (!mHover[idx].mHovered && mHover[idx].mTimer == 0 &&
                    !RPWlyAvatarMgr::sInstance->IsWrongReacting(idx)) {
                    mHover[idx].mHovered = true;
                    mHover[idx].mTimer = 0;
                    mHover[idx].mPlayer = player;
                    RPWlyAvatarMgr::sInstance->mpAvatars[idx]->mpSound->PlayVoice(3, player + 1);
                } else if (mHover[idx].mTimer >= 10) {
                    mHover[idx].mHovered = false;
                    mHover[idx].mTimer = 10;
                    mHover[idx].mPlayer = player;
                    RPWlyAvatarMgr::sInstance->mpAvatars[idx]->mpSound->PlayVoice(4, player + 1);
                }
            } else if (!mHover[idx].mHovered && mHover[idx].mTimer == 0 &&
                       !RPWlyAvatarMgr::sInstance->IsWrongReacting(idx)) {
                mHover[idx].mHovered = true;
                mHover[idx].mTimer = 0;
                mHover[idx].mPlayer = player;
                RPWlyAvatarMgr::sInstance->mpAvatars[idx]->mpSound->PlayVoice(3, player + 1);
            }
        }
    }
}
