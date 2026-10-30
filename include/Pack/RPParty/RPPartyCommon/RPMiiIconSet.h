#ifndef RP_PARTY_COMMON_MII_ICON_SET_H
#define RP_PARTY_COMMON_MII_ICON_SET_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <egg/core.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysKokeshiIcon;

/**
 * @brief Set of player Mii icon textures (one RPSysKokeshiIcon per player)
 * @details vtable 80357458. Size 0x14, verified (RPMiiIconSet::CreateInstance; singleton 8043C648).
 * Used by Shooting Range and Tanks for the Mii icons on targets.
 * @note Wii Play-only framework class (ledger Mii_MiiIconSet); the name is a project convention.
 */
class RPMiiIconSet : public RPSceneObj {
public:
    struct Param {
        u16 mNum;    // at 0x0 icon count
        u16 mWidth;  // at 0x2
        u16 mHeight; // at 0x4
        u16 unk6;    // at 0x6
    };

public:
    /**
     * @brief Creates sInstance (ledger Mii_createMiiIconSet) [H]
     */
    static void CreateInstance();
    /**
     * @brief Deletes sInstance and clears it (ledger Mii_destroyMiiIconSet) [H]
     */
    static void DestroyInstance();

    RPMiiIconSet() : mpIconsBegin(NULL), mpIconsLast(NULL) {}
    virtual ~RPMiiIconSet(); // at 0x8

    /**
     * @brief Calls LoadResource on every icon (ledger calcAll) [H]
     */
    virtual void Load(); // at 0x10

    /**
     * @brief Creates param.mNum player icons (ledger
     * Mii_MiiIconSet_createIcons) [H]
     */
    void CreateIcons(Param param);

    // Wii Play: read by Shooting Range (RPDucStartLayout::DrawMiiIcons)
    RPSysKokeshiIcon* GetIcon(s32 idx) const {
        return mpIconsBegin[idx];
    }

    static RPMiiIconSet* sInstance; // 8043C648
    EGG::ResTIMG* GetTexture(s32 idx) const;

private:
    Param mParam;                     // at 0x4
    RPSysKokeshiIcon** mpIconsBegin;  // at 0xC new[mNum]
    RPSysKokeshiIcon** mpIconsLast;   // at 0x10 last element
};

//! @}

#endif
