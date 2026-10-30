#include <Pack/RPParty/RPPartyCommon/RPMiiIconSet.h>

#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPKernel/RPSysKokeshiIcon.h>

RPMiiIconSet* RPMiiIconSet::sInstance;

void RPMiiIconSet::CreateInstance() {
    RPMiiIconSet::sInstance = new RPMiiIconSet;
}

void RPMiiIconSet::DestroyInstance() {
    delete RPMiiIconSet::sInstance;
    RPMiiIconSet::sInstance = NULL;
}

RPMiiIconSet::~RPMiiIconSet() {}

void RPMiiIconSet::CreateIcons(Param param) {
    u16 iconCount = param.mNum;
    u16 iconHeight;
    u16 iconWidth;
    iconWidth = param.mWidth;
    iconHeight = param.mHeight;
    mParam.mNum = iconCount;
    mParam.mWidth = iconWidth;
    mParam.mHeight = iconHeight;
    mParam.unk6 = param.unk6;
    mpIconsBegin = new RPSysKokeshiIcon*[iconCount];
    mpIconsLast = mpIconsBegin + iconCount - 1;

    u8 playerNo = 0;
    RPSysKokeshiIcon** pIcon = mpIconsBegin;
    do {
        RPSysKokeshiGenInfo info;
        info.SetGenType(RPSysKokeshiManager::GenType_Player);
        info.SetPlayerNo(playerNo);
        info.SetIconWidth(iconWidth);
        info.SetIconHeight(iconHeight);
        info.SetIconBGType(RFLIconBG_Direct);
        GXColor color = {0xff, 0xff, 0xff, 0xff};
        info.SetIconBGColor(color);
        info.SetIconMipMap(true);

        *pIcon = new RPSysKokeshiIcon(&info);

        ++playerNo;
    } while (++pIcon <= mpIconsLast);
}

void RPMiiIconSet::Load() {
    RPSysKokeshiIcon** pIcon = mpIconsBegin;
    do {
        (*pIcon)->LoadResource(NULL);
        ++pIcon;
    } while (pIcon <= mpIconsLast);
}

EGG::ResTIMG* RPMiiIconSet::GetTexture(s32 idx) const {
    return mpIconsBegin[idx]->GetResTIMG();
}
