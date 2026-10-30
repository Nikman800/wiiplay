// Laser Hockey rink: walls as line segments, wall animations, wall collision tests.
// Unit 0x802191DC-0x8021A4B4 (dtk/game_units.tsv). With -inline deferred CodeWarrior emits a
// file's functions in reverse order, so the source runs from the last address to the first.
#include <Pack/RPParty/RPHkyScene/RPHkyField.h>

#include <Pack/RPGraphics.h>

#include <cmath>

f32 RPHkyField::sScale = 1.0f;
// sLength and sWidth are defined in RPHkyGame.cpp (they are in its .sdata)

RPHkyField::~RPHkyField() {}

// Kept as a macro: it fills the wall table entry by entry. As an inline
// SetWall(RPHkySegment&, px, pz, dx, dz, len) the arguments are computed first and Create
// differs in 155 of 180 words.
#define SET_WALL(i, px, pz, dx, dz, len) \
    mWalls[i].mPos.x = px;              \
    mWalls[i].mPos.y = 0.0f;            \
    mWalls[i].mPos.z = pz;              \
    mWalls[i].mDir.x = dx;              \
    mWalls[i].mDir.y = 0.0f;            \
    mWalls[i].mDir.z = dz;              \
    mWalls[i].mLength = len;

void RPHkyField::Create() {
    mpWallIter = NULL;

    f32 side = sLength - 2.0f * cCorner;
    f32 corner = 0.5f * (cRinkDepth - cGoalWidth * sScale) - cCorner;
    f32 x = sWidth - cCorner;
    f32 z = cHalfDepth - cCorner;
    f32 goal = 0.5f * (cGoalWidth * sScale);


    SET_WALL(0, -x, -z, 1.0f, 0.0f, side);
    SET_WALL(1, x, z, -1.0f, 0.0f, side);
    SET_WALL(2, x, -z, 0.0f, 1.0f, corner);
    SET_WALL(3, x, goal, 0.0f, 1.0f, corner);
    SET_WALL(4, -x, z, 0.0f, -1.0f, corner);
    SET_WALL(5, -x, -goal, 0.0f, -1.0f, corner);
    f32 unusedDepth = -cHalfDepth;
    mWalls[6].mPos.x = x;
    mWalls[6].mPos.y = 0.0f;
    mWalls[6].mPos.z = -goal;
    mWalls[6].mDir.x = 1.0f;
    mWalls[6].mDir.y = 0.0f;
    mWalls[6].mDir.z = 0.0f;
    mWalls[6].mLength = cCorner;
    SET_WALL(7, sWidth, goal, -1.0f, 0.0f, cCorner);
    SET_WALL(8, -x, goal, -1.0f, 0.0f, cCorner);
    SET_WALL(9, -sWidth, -goal, 1.0f, 0.0f, cCorner);
    SET_WALL(10, sWidth, -cHalfDepth, -1.0f, 0.0f, sLength);
    SET_WALL(11, -sWidth, cHalfDepth, 1.0f, 0.0f, sLength);
    SET_WALL(12, sWidth, -goal, 0.0f, -1.0f, corner + cCorner);
    mWalls[13].mPos.x = sWidth;
    mWalls[13].mPos.y = 0.0f;
    mWalls[13].mPos.z = cHalfDepth;
    mWalls[13].mDir.x = 0.0f;
    mWalls[13].mDir.y = 0.0f;
    mWalls[13].mDir.z = -1.0f;
    mWalls[13].mLength = corner + cCorner;
    SET_WALL(14, -sWidth, goal, 0.0f, 1.0f, corner + cCorner);
    SET_WALL(15, -sWidth, -cHalfDepth, 0.0f, 1.0f, corner + cCorner);
    SET_WALL(16, x + cGoalInset, -goal, 0.0f, 1.0f, cGoalWidth * sScale);
    SET_WALL(17, -(x + cGoalInset), goal, 0.0f, -1.0f, cGoalWidth * sScale);
}

#undef SET_WALL

void RPHkyField::Load() {
    RPGrpHandle file = RPSimpleModel::LoadResource(EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE
                                                       ? "/G3D/Hck_Field_1.brres"
                                                       : "/G3D/Hck_Field_2.brres");
    RPSimpleModel::LoadModel(EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE ? "Hck_Field_1"
                                                                                : "Hck_Field_2",
                             file, 1, 0, 0x108);

    const char* anmNames[6] = {"line1", "line3", "line4", "line6", "goal_L", "goal_R"};
    RPGrpHandle anms[6];

    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 6);
    RPGrpHandle anmFile = RPSimpleModel::LoadResource("/G3D/Hck_anim.brres");

    for (u32 i = 0; i < 6; i++) {
        RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
        RPGrpHandle anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmClr,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, anmFile, anmNames[(u16)i]));
        anms[(u16)i] = anm;
        mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, i, anms[(u16)i]);
    }
}

#pragma opt_propagation off
void RPHkyField::Reset() {
    RPGrpModelAnm* pStart;
    mPos = sVecZero;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

    for (u32 i = 0; i < 6; i++) {
        RPGrpModelAnm* pFrame = mpModel->GetModelAnm();
        pStart = pFrame;
        pStart->Start(RPGrpModelAnm::Anm_Clr, i, pFrame->GetNumFrame(RPGrpModelAnm::Anm_Clr, i) - 1.0f);
        mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, i, 0.0f);
    }
}

#pragma opt_propagation reset

void RPHkyField::Calculate() {
    mScale.z = sScale;
    CalcModelMtx();
}

void RPHkyField::Exit() {}

void RPHkyField::ResetPuckWallIter() {
    mpWallIter = &mWalls[15];
}

void RPHkyField::ResetPaddleWallIter() {
    mpWallIter = &mWalls[17];
}

BOOL RPHkyField::TestCircleCollision(f32 radius, const nw4r::math::VEC3& rPos,
                                     nw4r::math::VEC3* pNormal, f32* pDepth) {
    if (mpWallIter < mWalls) {
        return FALSE;
    }

    do {
        RPHkySegment* pWall = mpWallIter;
        nw4r::math::VEC3 normal;
        normal.x = -pWall->mDir.z;
        normal.y = 0.0f;
        normal.z = pWall->mDir.x;
        nw4r::math::VEC3 rel;
        rel.x = rPos.x - pWall->mPos.x;
        rel.y = 0.0f;
        rel.z = rPos.z - pWall->mPos.z;

        f32 dist = normal.x * rel.x + normal.z * rel.z;
        if (dist >= radius || dist <= 0.02f * (0.5f * cCorner)) {
            continue;
        }

        f32 along = pWall->mDir.x * rel.x + pWall->mDir.z * rel.z;
        if (along >= pWall->mLength + radius || along <= -radius) {
            continue;
        }

        if (0.0f <= along) {
            if (along <= pWall->mLength) {
                *pNormal = nw4r::math::VEC3(static_cast<const nw4r::math::_VEC3&>(normal));
                *pDepth = radius - dist;
                mpWallIter--;
                return TRUE;
            }

            f32 over = along - pWall->mLength;
            if (over > dist) {
                continue;
            }
            f32 overSq = over * over;
            f32 radiusSq = radius * radius;
            if (dist * dist + overSq >= radiusSq) {
                continue;
            }
            *pDepth = std::sqrtf(radiusSq - overSq) - dist;
            *pNormal = nw4r::math::VEC3(static_cast<const nw4r::math::_VEC3&>(normal));
            mpWallIter--;
            return TRUE;
        } else {
            if (-along >= dist) {
                continue;
            }
            f32 alongSq = along * along;
            f32 radiusSq = radius * radius;
            if (dist * dist + alongSq >= radiusSq) {
                continue;
            }
            *pDepth = std::sqrtf(radiusSq - alongSq) - dist;
            *pNormal = nw4r::math::VEC3(static_cast<const nw4r::math::_VEC3&>(normal));
            mpWallIter--;
            return TRUE;
        }
    } while (--mpWallIter >= mWalls);

    return FALSE;
}

BOOL RPHkyField::TestRectCollision(f32 halfWidth, f32 halfDepth, const nw4r::math::VEC3& rPos,
                                   const nw4r::math::VEC3& rDir, nw4r::math::VEC3* pNormal,
                                   f32* pDepth) {
    if (mpWallIter < mWalls) {
        return FALSE;
    }

    do {
        RPHkySegment* pWall = mpWallIter;
        f32 dot = rDir.x * pWall->mDir.x + rDir.z * pWall->mDir.z;
        f32 cross = -rDir.z * pWall->mDir.x + rDir.x * pWall->mDir.z;

        // The rectangle corner nearest the wall
        f32 side;
        f32 front;
        if (0.0f == dot) {
            nw4r::math::VEC3 rel;
            rel.x = rPos.x - pWall->mPos.x;
            rel.z = rPos.z - pWall->mPos.z;
            if (rel.x * pWall->mDir.x + rel.z * pWall->mDir.z >= 0.5f * pWall->mLength) {
                if (cross >= 0.0f) {
                    side = halfDepth;
                    front = halfWidth;
                } else {
                    side = -halfDepth;
                    front = -halfWidth;
                }
            } else if (cross >= 0.0f) {
                front = halfWidth;
                side = -halfDepth;
            } else {
                side = halfDepth;
                front = -halfWidth;
            }
        } else if (0.0f == cross) {
            nw4r::math::VEC3 rel;
            rel.x = rPos.x - pWall->mPos.x;
            rel.z = rPos.z - pWall->mPos.z;
            if (rel.x * pWall->mDir.x + rel.z * pWall->mDir.z >= 0.5f * pWall->mLength) {
                if (dot >= 0.0f) {
                    front = halfWidth;
                    side = -halfDepth;
                } else {
                    side = halfDepth;
                    front = -halfWidth;
                }
            } else if (dot >= 0.0f) {
                side = -halfDepth;
                front = -halfWidth;
            } else {
                side = halfDepth;
                front = halfWidth;
            }
        } else {
            if (dot >= 0.0f) {
                side = -halfDepth;
            } else {
                side = halfDepth;
            }
            if (cross >= 0.0f) {
                front = halfWidth;
            } else {
                front = -halfWidth;
            }
        }

        nw4r::math::VEC3 corner;
        corner.x = rPos.x + rDir.x * front - rDir.z * side;
        corner.y = 0.0f;
        corner.z = rPos.z + rDir.z * front + rDir.x * side;
        nw4r::math::VEC3 rel;
        rel.x = corner.x - pWall->mPos.x;
        rel.y = 0.0f;
        rel.z = corner.z - pWall->mPos.z;
        nw4r::math::VEC3 normal;
        normal.x = -pWall->mDir.z;
        normal.y = 0.0f;
        normal.z = pWall->mDir.x;
        f32 dist = normal.x * rel.x + normal.z * rel.z;
        if (dist >= 0.0f) {
            continue;
        }

        f32 along = pWall->mDir.x * rel.x + pWall->mDir.z * rel.z;
        if (along >= pWall->mLength) {
            // Past the end of the wall: test the edge to the neighbouring corner
            if (side >= 0.0f) {
                if (front >= 0.0f) {
                    side = -side;
                } else {
                    front = -front;
                }
            } else if (front >= 0.0f) {
                front = -front;
            } else {
                side = -side;
            }

            nw4r::math::VEC3 corner2;
            corner2.x = rPos.x + rDir.x * front - rDir.z * side;
            corner2.y = 0.0f;
            corner2.z = rPos.z + rDir.z * front + rDir.x * side;
            nw4r::math::VEC3 rel2;
            rel2.x = corner2.x - pWall->mPos.x;
            rel2.z = corner2.z - pWall->mPos.z;
            f32 dist2 = normal.x * rel2.x + normal.z * rel2.z;
            if (dist2 <= 0.5f * -cCorner) {
                continue;
            }
            f32 along2 = pWall->mDir.x * rel2.x + pWall->mDir.z * rel2.z;
            if (along2 >= pWall->mLength) {
                continue;
            }

            if (dist2 >= 0.0f) {
                nw4r::math::VEC3 edge;
                edge.x = corner2.x - corner.x;
                edge.z = corner2.z - corner.z;
                f32 len = std::sqrtf(edge.x * edge.x + edge.z * edge.z);
                if (0.0f == len) {
                    continue;
                }
                f32 inv = 1.0f / len;
                edge.x *= inv;
                edge.z *= inv;
                pWall = mpWallIter;
                if (edge.x * pWall->mDir.x + edge.z * pWall->mDir.z > -0.7071068f) {
                    continue;
                }

                nw4r::math::VEC3 toStart;
                toStart.x = -rel.x;
                toStart.z = -rel.z;
                nw4r::math::VEC3 toEnd;
                toEnd.x = pWall->mDir.x * pWall->mLength + toStart.x;
                toEnd.z = pWall->mDir.z * pWall->mLength + toStart.z;
                nw4r::math::VEC3 edgeNormal;
                edgeNormal.x = -edge.z;
                edgeNormal.z = edge.x;
                f32 startDot = toStart.x * edgeNormal.x + toStart.z * edgeNormal.z;
                f32 endDot = toEnd.x * edgeNormal.x + toEnd.z * edgeNormal.z;
                if (startDot * endDot >=
                    0.0f) {
                    continue;
                }

                *pDepth = (dist2 - dist) * (pWall->mLength - along2) / (along - along2) - dist2;
                *pNormal = nw4r::math::VEC3(static_cast<const nw4r::math::_VEC3&>(normal));
                mpWallIter--;
                return TRUE;
            } else {
                if (pWall->mLength - along2 < -dist2) {
                    continue;
                }

                nw4r::math::VEC3 edge;
                edge.x = corner2.x - corner.x;
                edge.z = corner2.z - corner.z;
                f32 len = std::sqrtf(edge.x * edge.x + edge.z * edge.z);
                if (0.0f == len) {
                    continue;
                }
                f32 inv = 1.0f / len;
                edge.x *= inv;
                edge.z *= inv;
                pWall = mpWallIter;
                if (edge.x * pWall->mDir.x + edge.z * pWall->mDir.z > -0.7071068f) {
                    continue;
                }

                *pDepth = -dist - (dist2 - dist) * (along - pWall->mLength) / (along - along2);
                *pNormal = nw4r::math::VEC3(static_cast<const nw4r::math::_VEC3&>(normal));
                mpWallIter--;
                return TRUE;
            }
        } else if (along <= 0.0f) {
            // Before the start of the wall
            if (side >= 0.0f) {
                if (front >= 0.0f) {
                    front = -front;
                } else {
                    side = -side;
                }
            } else if (front >= 0.0f) {
                side = -side;
            } else {
                front = -front;
            }

            nw4r::math::VEC3 corner2;
            corner2.x = rPos.x + rDir.x * front - rDir.z * side;
            corner2.y = 0.0f;
            corner2.z = rPos.z + rDir.z * front + rDir.x * side;
            nw4r::math::VEC3 rel2;
            rel2.x = corner2.x - pWall->mPos.x;
            rel2.z = corner2.z - pWall->mPos.z;
            f32 dist2 = normal.x * rel2.x + normal.z * rel2.z;
            if (dist2 <= 0.5f * -cCorner) {
                continue;
            }
            f32 along2 = pWall->mDir.x * rel2.x + pWall->mDir.z * rel2.z;
            if (along2 <= 0.0f) {
                continue;
            }

            if (dist2 >= 0.0f) {
                nw4r::math::VEC3 edge;
                edge.x = corner2.x - corner.x;
                edge.z = corner2.z - corner.z;
                f32 len = std::sqrtf(edge.x * edge.x + edge.z * edge.z);
                if (0.0f == len) {
                    continue;
                }
                f32 inv = 1.0f / len;
                edge.x *= inv;
                edge.z *= inv;
                pWall = mpWallIter;
                if (edge.x * pWall->mDir.x + edge.z * pWall->mDir.z < 0.7071068f) {
                    continue;
                }

                nw4r::math::VEC3 toStart;
                toStart.x = -rel.x;
                toStart.z = -rel.z;
                nw4r::math::VEC3 toEnd;
                toEnd.x = pWall->mDir.x * pWall->mLength + toStart.x;
                toEnd.z = pWall->mDir.z * pWall->mLength + toStart.z;
                nw4r::math::VEC3 edgeNormal;
                edgeNormal.x = -edge.z;
                edgeNormal.z = edge.x;
                f32 startDot = toStart.x * edgeNormal.x + toStart.z * edgeNormal.z;
                f32 endDot = toEnd.x * edgeNormal.x + toEnd.z * edgeNormal.z;
                if (startDot * endDot >=
                    0.0f) {
                    continue;
                }

                *pDepth = along2 * (dist2 - dist) / (along2 - along) - dist2;
                *pNormal = nw4r::math::VEC3(static_cast<const nw4r::math::_VEC3&>(normal));
                mpWallIter--;
                return TRUE;
            } else {
                if (along2 < -dist2) {
                    continue;
                }

                nw4r::math::VEC3 edge;
                edge.x = corner2.x - corner.x;
                edge.z = corner2.z - corner.z;
                f32 len = std::sqrtf(edge.x * edge.x + edge.z * edge.z);
                if (0.0f == len) {
                    continue;
                }
                f32 inv = 1.0f / len;
                edge.x *= inv;
                edge.z *= inv;
                pWall = mpWallIter;
                if (edge.x * pWall->mDir.x + edge.z * pWall->mDir.z < 0.7071068f) {
                    continue;
                }

                *pDepth = -dist - along * (dist2 - dist) / (along - along2);
                *pNormal = nw4r::math::VEC3(static_cast<const nw4r::math::_VEC3&>(normal));
                mpWallIter--;
                return TRUE;
            }
        } else {
            if (dist <= 0.5f * -cCorner) {
                continue;
            }

            *pDepth = -dist;
            *pNormal = nw4r::math::VEC3(static_cast<const nw4r::math::_VEC3&>(normal));
            mpWallIter--;
            return TRUE;
        }
    } while (--mpWallIter >= mWalls);

    return FALSE;
}

void RPHkyField::PlayWallAnim(int idx) {
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, idx, 0.0f);
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, idx, 1.0f);
}

s32 RPHkyField::GetHitWallIndex() const {
    for (int i = 0; i < 18; i++) {
        if (mpWallIter + 1 == &mWalls[i]) {
            return i;
        }
    }

    return -1;
}
