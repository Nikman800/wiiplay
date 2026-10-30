#ifndef EGG_GFXE_LIGHT_TEXTURE_MANAGER_H
#define EGG_GFXE_LIGHT_TEXTURE_MANAGER_H
#include <egg/types_egg.h>

#include <egg/core.h>
#include <egg/gfxe/eggScreen.h>

namespace EGG {

// Forward declarations
class LightManager;
class LightTexture;
class LightObject;

// Wii Play: public, game code calls SetBinary (RPPnpGame::Load)
class LightTextureManager : public IBinary<LightTextureManager> {
public:
    struct BinData {
        char unk10[0x20 - 0x10];
    };

public:
    LightTextureManager(const LightManager* pManager);

    virtual void SetBinaryInner(const IBinary::Bin& rBinary); // at 0x8
    virtual void GetBinaryInner(IBinary::Bin* pBinary) const; // at 0xC

    virtual const char* GetBinaryType() const; // at 0x10
    virtual u8 GetVersion() const;            // at 0x18

    u16 createTexture(const char* pName);
    u16 replaceModelTexture(int, ModelEx*) const;
    u16 replaceModelTexture(const char* pName, ModelEx* pMdl) const;
    int getTextureIndex(const char* pName) const;

    void correctLightObject();
    void frameReset();
    void drawAndCaptureTexture(LightManager*, const Screen::DataEfb&, f32, f32,
                               f32, f32);

    // Wii Play: RPFshPlayer::Create [L name]
    void onFlag2() {
        mFlags |= cFlag_2;
    }

    bool isEnableSetPixelFormat() const {
        return mFlags & cFlag_5;
    }

    bool isEnableClearWorkSpace() const {
        return mFlags & cFlag_ClearWorkSpace;
    }

    u32 getMax() const {
        return LIGHT_TEX_MAX;
    }

    // Wii Play: RPGrpDrawPathLightMap On/Off/IsEnable (flag 0x10)
    void onFlag4() {
        mFlags |= cFlag_4;
    }
    void offFlag4() {
        mFlags &= ~cFlag_4;
    }
    bool isFlag4() const {
        return mFlags & cFlag_4;
    }

    // Wii Play: RPGrpDrawPathLightMap (assert "index < mTexNum" at eggLightTextureManager.h:112)
    LightTexture* getTexture(u16 index) const {
#line 112
        EGG_ASSERT(index < mTexNum);
        return mppLightTextures[index];
    }

    // Wii Play: LightTexture::draw
    const LightManager* getLightManager() const {
        return mpLightManager;
    }
    u16 getLightObjectMax() const {
        return mLightObjectMax;
    }
    LightObject* getLightObject(u16 index) const {
#line 179
        EGG_ASSERT(index < mLightObjectMax);
        return mppLightObjs[index];
    }

private:
    enum {
        cFlag_2 = 1 << 2,
        cFlag_ClearWorkSpace = 1 << 3,
        cFlag_4 = 1 << 4,
        cFlag_5 = 1 << 5,
        cFlag_6 = 1 << 6,
    };

    static const u8 VERSION = 0;

    static const int LIGHT_TEX_MAX = 32;
    static const int LIGHT_OBJ_MAX = 32;

private:
    u8 mFlags;                          // at 0x4
    u16 mTexNum;                        // at 0x6
    LightTexture** mppLightTextures;    // at 0x8
    const LightManager* mpLightManager; // at 0xC
    u32 mDrawFlags;                     // at 0x10
    u16 mLightObjectMax;                // at 0x14
    LightObject** mppLightObjs;         // at 0x18
};

} // namespace EGG

#endif
