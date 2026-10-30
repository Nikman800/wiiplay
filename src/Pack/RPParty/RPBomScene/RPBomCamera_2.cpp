// Pose Mii: the second source file of the old RPBomCamera range (unit 0x802D6090-0x802D6354,
// dtk/game_units.tsv): screen helpers, the work-entry shuffle and identity matrices, with its
// own __sinit. Functions in reverse address order.
#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>

struct BomShuffleEntry {
    s32 value;
    u32 key;
};

struct BomWorkEntry {
    BomWorkEntry();
    u32 unk[4];
};

// Read by RPBomPanel_2 (Bom_Panel_checkPlayerMatch); the first object of this file's .sdata2.
extern const f32 Bom_panelMatchScale = 9.0f;

static nw4r::math::VEC3 sAxisX(1.0f, 0.0f, 0.0f);
static nw4r::math::VEC3 sAxisY(0.0f, 0.0f, -1.0f);
static nw4r::math::VEC3 sAxisZ(0.0f, 1.0f, 0.0f);
extern "C" {
// Rotation by -90 degrees about X (read by RPBomStartBg)
RPAxes Bom_rotXNeg90 = {sAxisX, sAxisY, sAxisZ};

f32 Bom_screenWidth = 1.0f;
f32 Bom_screenHeight = 1.0f;
f32 Bom_screenHalfWidth = 1.0f;
f32 Bom_screenHalfHeight = 1.0f;

BomShuffleEntry* Bom_shuffleBuffer;
BomWorkEntry* Bom_shuffleBuffer2;
}

// A body that is not empty to the front end keeps the constructor in __construct_new_array.
BomWorkEntry::BomWorkEntry() {
    unk[0];
}

extern "C" void Bom_allocShuffleBuffers() {
    Bom_shuffleBuffer = new BomShuffleEntry[0x100];
    Bom_shuffleBuffer2 = new BomWorkEntry[0x100];
}

extern "C" void Bom_updateScreenSize() {
    RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(2)->GetScreen();
    Bom_screenWidth = pScreen->GetWidth();
    Bom_screenHeight = pScreen->GetHeight();
    Bom_screenHalfWidth = 0.5f * Bom_screenWidth;
    Bom_screenHalfHeight = 0.5f * Bom_screenHeight;
}

extern "C" f32 Bom_getScreenWidth() {
    return Bom_screenWidth;
}

extern "C" f32 Bom_getScreenHeight() {
    return Bom_screenHeight;
}

extern "C" f32 Bom_getScreenHalfWidth() {
    return Bom_screenHalfWidth;
}

extern "C" f32 Bom_getScreenHalfHeight() {
    return Bom_screenHalfHeight;
}

static inline u32 BomRand() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}
#pragma opt_unroll_loops off
extern "C" void Bom_shuffleArray(s32* pValues, u32 num) {
    if (num == 1) {
        return;
    }
    BomShuffleEntry tmp;
    u32 i = 0;
    do {
        Bom_shuffleBuffer[i].value = pValues[i];
        Bom_shuffleBuffer[i].key = (BomRand() >> 8) & 0xFFFF;
        i++;
    } while (i < num);
    i = 0;
    do {
        u32 j = i + 1;
        do {
            if (Bom_shuffleBuffer[i].key < Bom_shuffleBuffer[j].key) {
                tmp = Bom_shuffleBuffer[i];
                Bom_shuffleBuffer[i] = Bom_shuffleBuffer[j];
                Bom_shuffleBuffer[j] = tmp;
            }
            j++;
        } while (j < num);
        i++;
    } while (i < num - 1);
    i = 0;
    do {
        pValues[i] = Bom_shuffleBuffer[i].value;
        i++;
    } while (i < num);
}
#pragma opt_unroll_loops reset
