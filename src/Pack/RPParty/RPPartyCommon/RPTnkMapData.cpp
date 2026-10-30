#include <Pack/RPParty/RPPartyCommon/RPTnkMapData.h>

#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/gfxe/eggScreen.h>

// NONMATCHING-DEBT: RP_TnkMapData_getCell reads mpMaps through this non-polymorphic view. As
// RPTnkMapData::GetCell (declared in the header) or as a friend reading pMapData->mpMaps, the
// compiler keeps this+4 in a register and indexes with lwzx; the original adds the indices to
// the object and loads with displacement 0x4. Tried 2026-10-29: indexed, local width, offset
// temporary, reference-to-row, GetMaps()/GetMap() accessors, a this copy, friend free function.
struct RPTnkMapDataAccess {
    void* vtable;
    RPTnkMapData::Map* mpMaps[2][2][30];
};

inline RPTnkMapData::Map* (&GetTnkMaps(RPTnkMapData& mapData))[2][2][30] {
    return reinterpret_cast<RPTnkMapDataAccess&>(mapData).mpMaps;
}

inline RPTnkMapData::Map* const (&GetTnkMapsConst(const RPTnkMapData& mapData))[2][2][30] {
    return reinterpret_cast<const RPTnkMapDataAccess&>(mapData).mpMaps;
}

inline const RPTnkMapData::Map& GetTnkMap(RPTnkMapData& mapData, s32 playerIdx, s32 stage,
                                          s32 tvMode) {
    return *GetTnkMaps(mapData)[tvMode][playerIdx][stage];
}

inline s32 GetTnkWidth(const RPTnkMapData::Map& map) {
    return map.width;
}

inline s32 GetTnkCellOffset(s32 x, s32 width, s32 y) {
    return x + width * y;
}


const char RP_tnkMapDataPathFmt[] = "MapData/TnkMapData_P%d_%02d_%d.bin";

RPTnkMapData* RP_tnkMapDataInstance;

void RPDrawKokeshiIcon(RPSysKokeshiIcon* pIcon, nw4r::lyt::Pane* pPane, BOOL black,
                       RPGrpScreen* pScreen) {
    if (pScreen == NULL) {
        pScreen = RPGrpRenderer::GetCurrentScreen();
    }

    GXColor color = {255, 255, 255, pPane->GetGlbAlpha()};
    if (black) {
        color.b = 0;
        color.g = 0;
        color.r = 0;
    }

    RPSysLayout::drawKokeshiIcon(pScreen, pPane, pIcon, color);
}

void RPTnkMapData::CreateInstance() {
    RP_tnkMapDataInstance = new RPTnkMapData;
}

void RPTnkMapData::DestroyInstance() {
    delete RP_tnkMapDataInstance;
    RP_tnkMapDataInstance = NULL;
}

RPTnkMapData::RPTnkMapData() {
    for (s32 variant = 0; variant < 2; variant++) {
        for (s32 player = 0; player < 2; player++) {
            for (s32 stage = 0; stage < 30; stage++) {
                char path[256];
                snprintf(path, sizeof(path), RP_tnkMapDataPathFmt, player + 1, stage, variant);

                u32 size;
                void* pFile = RPSysResourceManager::GetFileFromArchive(
                    RPSysResourceManager::LoadGameCommonArchive(-1, NULL), path, &size);
                Map* pMap = reinterpret_cast<Map*>(new u8[size]);
                memcpy(pMap, pFile, size);
                mpMaps[variant][player][stage] = pMap;
            }
        }
    }
}

RPTnkMapData::~RPTnkMapData() {}

s32 RPTnkMapData::GetUnk08(s32 stage, s32 playerIdx) const {
    return mpMaps[EGG::Screen::GetTVMode()][playerIdx][stage]->unk8;
}

s32 RPTnkMapData::GetUnk0C(s32 stage, s32 playerIdx) const {
    return mpMaps[EGG::Screen::GetTVMode()][playerIdx][stage]->unkC;
}

extern "C" s32 RP_TnkMapData_getCell(RPTnkMapData* pMapData, s32 stage, s32 x, s32 y,
                                     s32 playerIdx) {
    const s32 tvMode = EGG::Screen::GetTVMode();
    return GetTnkMap(*pMapData, playerIdx, stage, tvMode)
        .cells[GetTnkCellOffset(x, GetTnkWidth(*GetTnkMapsConst(*pMapData)[tvMode][0][stage]), y)];
}
