// Shooting Range point popup (RPDucPointPopup): the "+N" / "-N" number shown where a target was hit.
// Unit 0x8024D58C-0x8024DB30 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucHud.h>

#include <Pack/RPKernel.h>
#include <Pack/RPSystem.h>

#include <egg/gfxe.h>

static inline f32 ClampPopupCoordinate(f32 value) {
    f32 v = -0.9f > value ? -0.9f : value;
    return 0.9f < v ? 0.9f : v;
}

RPDucPointPopup::RPDucPointPopup() {
    mActive = false;
}

RPDucPointPopup::~RPDucPointPopup() {}

void RPDucPointPopup::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "duckhunt.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "duckhuntPoint_00.brlyt");
    mpAnmObjs[0] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "duckhuntPoint_00_inOutAll_00.brlan");
    mpAnmObjs[0]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
}

void RPDucPointPopup::Reset() {
    mpLayout->reset();

    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 1);

    mpLayout->unbindAllAnmObj();

    i = 0;
    do {
        mNumText[i] = 0;
    } while (++i < 4);
    mActive = false;
}

void RPDucPointPopup::Calculate() {
    if (mActive && mpAnmObjs[0]->isFinished()) {
        RPDucHud::sInstance->mPopupNum--;
        mActive = false;
    }
    mpLayout->calc();
}

void RPDucPointPopup::Draw() {
    mpLayout->draw();
}

void RPDucPointPopup::VF_0x18() {}

void RPDucPointPopup::Show(nw4r::math::VEC2 screenPos, s32 points, s32 player, s32 effect) {
    RPSysStringUtility::GetStringHalfSizeNumber(points, mNumText, 4, true);

    const char* centerNames[2] = {"N_centerU_00", "N_centerD_00"};
    u32 down = (u32)points >> 31;
    if (!down) {
        mpLayout->findPane(centerNames[0])->SetVisible(true);
        mpLayout->findPane(centerNames[1])->SetVisible(false);
    } else {
        mpLayout->findPane(centerNames[0])->SetVisible(false);
        mpLayout->findPane(centerNames[1])->SetVisible(true);
    }

    const char* textNames[2][2] = {
        {"T_numberU_00", "T_numberU_01"},
        {"T_numberD_00", "T_numberD_01"},
    };
    for (s32 i = 0; i < 2; i++) {
        mpLayout->findTextBox(textNames[down][i])->setText(mNumText, 0);
        if (i == 0) {
            u32 color = 0;
            if (!down) {
                if (player == 0) {
                    color = 0;
                } else if (player == 1) {
                    color = 2;
                }
            } else {
                if (player == 0) {
                    color = 1;
                } else if (player == 1) {
                    color = 3;
                }
            }
            mpLayout->changeFontColor(textNames[down][i], color);
        }
    }

    screenPos.x = ClampPopupCoordinate(screenPos.x);
    screenPos.y = ClampPopupCoordinate(screenPos.y);

    nw4r::lyt::Pane* pPane = mpLayout->findPane(centerNames[down]);
    nw4r::math::VEC3 trans(pPane->GetTranslate().x, pPane->GetTranslate().y,
                           pPane->GetTranslate().z);
    trans.x = screenPos.x * (0.5f * EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD));
    trans.y = screenPos.y * (0.5f * EGG::Screen::GetSizeYMax(EGG::Screen::TV_MODE_STD));
    pPane->SetTranslate(trans);

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mActive = true;

    if (effect) {
        nw4r::math::VEC2 pos;
        pos.x = screenPos.x * static_cast<u16>(EGG::Screen::GetSizeXMax() / 2);
        pos.y = screenPos.y * static_cast<u16>(EGG::Screen::GetSizeYMax() / 2);
        nw4r::math::VEC2 effectPos = pos;
        RPSysEffect eff("Sht_2d_good", 2, 0);
        eff.create();
        nw4r::math::VEC3 effectScale;
        nw4r::math::VEC3 effectTrans(effectPos.x, effectPos.y, 0.0f);
        eff.setPos(effectTrans);
        effectScale.x = 0.8f;
        effectScale.y = 0.8f;
        effectScale.z = 0.8f;
        eff.setScale(effectScale);
        eff.update();
    }
}
