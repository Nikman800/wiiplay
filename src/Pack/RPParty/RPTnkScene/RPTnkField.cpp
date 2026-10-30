// Tanks field, floor models and circle-against-wall collision.
// Unit 0x802575BC-0x80258ADC.
#include <Pack/RPParty/RPTnkScene/RPTnkField.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem.h>
#include <egg/gfxe/eggScreen.h>
#include <egg/gfxe/eggDrawGX.h>
#include <Pack/RPParty/RPTnkScene/RPTnkBlock.h>
#include <Pack/RPParty/RPTnkScene/RPTnkStageMap.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTask.h>

extern RPAxes sFloorQuadBasis;

RPTnkField* RPTnkField::sInstance;
f32 RPTnkField::sCellWidth = 35.0f;
f32 RPTnkField::sCellDepth = 35.0f;

extern const f32 sWallMargin;

void RPTnkField::CreateInstance() { sInstance = new RPTnkField; }

void RPTnkField::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkField::~RPTnkField() {}

void RPTnkField::InitWalls() {
    f32 depth;
    f32 zero;
    f32 startX;
    f32 margin;
    f32 half;
    f32 width;
    half = 0.5f;
    margin = sWallMargin;
    depth = mDepth;
    width = mWidth;
    startX = half * width + margin;
    zero = 0.0f;
    mWalls[0].mPos.x = startX;
    mWalls[0].mPos.y = zero;
    mWalls[0].mPos.z = half * -depth;
    mWalls[0].mNormal.x = 0.0f;
    mWalls[0].mNormal.y = 0.0f;
    mWalls[0].mNormal.z = 1.0f;
    mWalls[0].mLength = width + 2.0f * margin;
    mWalls[0].unk1C = 0;
    mWalls[1].mPos.x = half * width;
    mWalls[1].mPos.y = 0.0f;
    mWalls[1].mPos.z = half * depth + margin;
    mWalls[1].mNormal.x = -1.0f;
    mWalls[1].mNormal.y = 0.0f;
    mWalls[1].mNormal.z = 0.0f;
    mWalls[1].mLength = depth + 2.0f * margin;
    mWalls[1].unk1C = 0;
    mWalls[2].mPos.x = -(half * width + margin);
    mWalls[2].mPos.y = 0.0f;
    mWalls[2].mPos.z = half * depth;
    mWalls[2].mNormal.x = 0.0f;
    mWalls[2].mNormal.y = 0.0f;
    mWalls[2].mNormal.z = -1.0f;
    mWalls[2].mLength = width + 2.0f * margin;
    mWalls[2].unk1C = 0;
    mWalls[3].mPos.x = -(half * width);
    mWalls[3].mPos.y = 0.0f;
    mWalls[3].mPos.z = -(half * depth + margin);
    mWalls[3].mNormal.x = 1.0f;
    mWalls[3].mNormal.y = 0.0f;
    mWalls[3].mNormal.z = 0.0f;
    mWalls[3].mLength = depth + 2.0f * margin;
    mWalls[3].unk1C = 0;
}

void RPTnkField::Create() {
    mpModel = new RPTnkFieldModel;
    mpModel->Create();
    RPTnkBlockPool::CreateInstance();
    mpBlockPool = RPTnkBlockPool::sInstance;
    mpBlockPool->Init(256);
    RPTnkStageMap::CreateInstance();
    mpStageMap = RPTnkStageMap::sInstance;
    mpStageMap->Create();
    RPGrpScreen* screen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    mpCaptureTex = new RPGrpCaptureTexture(screen, GX_TF_IA8, 0);
    RPTnkStepTimer::CreateInstance();
    mpStepTimer = RPTnkStepTimer::sInstance;
    mpStepTimer->Create();
}

void RPTnkField::Load() {
    mpModel->Load();
    mpBlockPool->Load();
    mpStageMap->Load();
    mpStepTimer->Load();
}

void RPTnkField::Reset() {
    switch (EGG::Screen::GetTVMode()) {
    case EGG::Screen::TV_MODE_WIDE:
        mWidth = 770.0f;
        mDepth = 595.0f;
        mCols = 22;
        mRows = 17;
        break;
    case EGG::Screen::TV_MODE_STD:
        mWidth = 560.0f;
        mDepth = 595.0f;
        mCols = 16;
        mRows = 17;
        break;
    }
    f32 cellWidth = mWidth / mCols;
    f32 cellDepth = mDepth / mRows;
    mCellWidth = cellWidth;
    mCellDepth = cellDepth;
    sCellWidth = cellWidth;
    sCellDepth = cellDepth;
    InitWalls();
    mpModel->Reset();
    mpModel->SetGridSize(mWidth, mDepth, mCols, mRows);
    mpBlockPool->Reset();
    mpStageMap->Reset();
    mWorldScale = 1.0f;
    mpStepTimer->Reset();
}

// Stand-in for an unused helper the linker dropped (its 0.5f and int->float constants stay in
// .sdata2 between DrawCaptureTexture's and Reset's).
extern "C" f32 Tnk_Field_deadHelper(s32 col) { return 0.5f * col; }

struct RPTnkFieldActorAccess : RPTnkActor {
    static f32 HitRadius(const RPTnkActor* actor) {
        return actor->*(&RPTnkFieldActorAccess::mHitRadius);
    }
};
// NONMATCHING-DEBT: word-wise copies. The original copies the wall and the hit centre with
// lwz/stw pairs; `pos = center` forwards the floats (0.94), `VEC3 pos(center)` in the loop is
// 79/91, and RPTnkWall (VEC3 members) copies as floats (39+, see below).
struct RPTnkWallWords {
    nw4r::math::_VEC3 pos, normal;
    f32 length;
    u32 id;
};
struct RPTnkVec3Words {
    u32 x, y, z;
};

BOOL RPTnkField::CollideActor(RPTnkActor* actor, s32 mode) {
    f32 radius = RPTnkFieldActorAccess::HitRadius(actor);
    nw4r::math::VEC3 pos;
    nw4r::math::VEC3 normal;
    nw4r::math::VEC3 center = actor->GetHitCenter();
    s32 i;
    BOOL hit;
    RPTnkField* wallField;
    hit = FALSE;
    wallField = this;
    i = 0;
    do {
        u32 id;
        f32 depth;
        RPTnkWallWords wall = *reinterpret_cast<RPTnkWallWords*>(&wallField->mWalls[0]);
        *reinterpret_cast<RPTnkVec3Words*>(&pos) = *reinterpret_cast<RPTnkVec3Words*>(&center);
        if (RPTnkTestCircleVsWall(radius, &normal, &depth, &id, &pos,
                                  reinterpret_cast<RPTnkWall*>(&wall))) {
            actor->OnHitWall(depth, &normal, id, RP_vecZero);
            hit = TRUE;
        }
        // NONMATCHING-DEBT: byte step of sizeof(RPTnkWall) over `this`. The original's walker
        // starts at `this` (mr) with mWalls' 0x1C folded into every load. The natural walk
        // (`const RPTnkWall* pWall = mWalls; ... pWall++`) starts the walker at this + 0x1C:
        // 9 words (addi instead of mr, eight load displacements); an index loop over mWalls[i]
        // (s32/u32/int, for or do-while, &mWalls[i] or mWalls + i) costs 17 (the same plus
        // walker/hit/i register rotation); RPTnkWall copied as a struct, not as words, 39+.
        // A start-value difference, not numbering, so a sim.py replay does not reach it.
        wallField = reinterpret_cast<RPTnkField*>(reinterpret_cast<char*>(wallField) + 0x20);
        i++;
    } while (i < 4);
    bool result = hit;
    result |= mpBlockPool->CollideObject(actor, mode);
    return result;
}

void RPTnkField::Calculate() {
    mpModel->Calculate();
    mpStepTimer->Calculate();
}

void RPTnkField::UpdateModels() {
    mpModel->CalcModelMtx();
    mpBlockPool->CalcModelMtxAll();
}

void RPTnkBlockPool::Destroy() { DestroyInstance(); }

void RPTnkField::Exit() {
    mpBlockPool->Destroy();
    mpStageMap->Exit();
    RPTnkStageMap::DestroyInstance();
    RPTnkStepTimer::DestroyInstance();
}

void RPTnkField::SetWorldScale(f32 scale) {
    mWorldScale = scale;
}

void RPTnkField::CaptureTexture() { mpCaptureTex->Capture(43); }

void RPTnkField::DrawClearQuad() {
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    RPGrpScreen* screen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    GXColor color = {255, 255, 255, 255};
    nw4r::math::VEC3 scale(screen->GetWidth(), screen->GetHeight(), 1.0f);
    nw4r::math::MTX34 mtx;
    RPAxes rot = sFloorQuadBasis;
    RPDucMakeMtx(&mtx, rot, scale, pos);
    EGG::DrawGX::BeginDrawQuad(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_ALWAYS,
                              EGG::DrawGX::BLEND_NORMAL, false, true);
    EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_QUAD, mtx, color);
}

void RPTnkField::DrawCaptureTexture(s32 steps) {
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    GXColor color = {0, 0, 0, 255};
    u8 amount = steps << 2;
    bool valid = false;
    if (steps >= 1 && steps <= 255) valid = true;
    if (valid) {
        GXColor fade = {amount, amount, amount, 255};
        color = fade;
    }
    RPGrpScreen* screen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    nw4r::math::VEC3 scale(screen->GetWidth(), screen->GetHeight(), 1.0f);
    nw4r::math::MTX34 mtx;
    RPAxes rot = sFloorQuadBasis;
    RPDucMakeMtx(&mtx, rot, scale, pos);
    EGG::DrawGX::SetMat_ColorChannel(EGG::DrawGX::COLORCHAN_COLOR);
    EGG::DrawGX::SetMat_TexGen(EGG::DrawGX::TEXGEN_ENABLE);
    EGG::DrawGX::SetMat_Ind();
    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_C0, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    EGG::DrawGX::SetZMode(EGG::DrawGX::ZMODE_ALWAYS);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    EGG::DrawGX::SetVtxState(EGG::DrawGX::VTXTYPE_QUAD_TEXTURE);
    mpCaptureTex->SetFilter(1);
    mpCaptureTex->GetCapTexture()->load(GX_TEXMAP0);
    EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_QUAD_TEXTURE, mtx, EGG::DrawGX::WHITE);
}

void RPTnkField::DrawPlainQuad() {
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    GXColor color = {255, 255, 255, 255};
    nw4r::math::VEC3 scale(1000.0f, 1000.0f, 1.0f);
    nw4r::math::MTX34 mtx;
    RPAxes rot = sFloorQuadBasis;
    RPDucMakeMtx(&mtx, rot, scale, pos);
    EGG::DrawGX::BeginDrawQuad(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_XLU,
                              EGG::DrawGX::BLEND_NORMAL, false, false);
    EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_QUAD, mtx, color);
}

// NONMATCHING-DEBT: volatile word copy + volatile pool read force `lwz r3,0xa0(r3)` after the three
// stw of the VEC3 copy (natural `VEC3 point = pos; return mpBlockPool->IsPointInsideBlock(point);`
// hoists that load between the stores, 2 instructions). Tried: copy-construct, assign, temp,
// by-value inline helper, pool local, volatile on only one side.
// --sched (2026-10-29): scheduling, not numbering. `VEC3 point(pos.x, pos.y, pos.z)` is 1 word
// off: the scheduler has no edge between the three stack stores and the this-relative load and
// picks the load at cycle 4 between the stores; only a may-alias edge keeps it last.
// Rule check (2026-10-29, alias_probe): blocked. IsBlockAtPos is const (mangled CF), so `this` is
// pointer-to-const: its dummy is never in the worst-case set, and the stack copy (address-taken
// local, always in) can't conflict with the mpBlockPool load. The original's store->load edge
// needs a non-const `this`; no natural statement order or parameter form adds one.
BOOL RPTnkField::IsBlockAtPos(nw4r::math::VEC3 pos) const {
    nw4r::math::VEC3 point;
    *reinterpret_cast<volatile u32*>(&point.x) = *reinterpret_cast<const u32*>(&pos.x);
    *reinterpret_cast<volatile u32*>(&point.y) = *reinterpret_cast<const u32*>(&pos.y);
    *reinterpret_cast<volatile u32*>(&point.z) = *reinterpret_cast<const u32*>(&pos.z);
    RPTnkBlockPool* pool = *reinterpret_cast<RPTnkBlockPool* const volatile*>(&mpBlockPool);
    return pool->IsPointInsideBlock(point);
}

