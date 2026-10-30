// Pose Mii point popup (RPBomLytPoint, poseManPoint_00).
#include <Pack/RPParty/RPBomScene/RPBomLyt.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>

#include <egg/gfxe/eggScreen.h>
#include <nw4r/lyt.h>

namespace {
inline f32 PointMax(f32 a, f32 b) {
    if (a > b) {
        return a;
    }
    return b;
}
inline f32 PointMin(f32 a, f32 b) {
    if (a < b) {
        return a;
    }
    return b;
}
inline f32 PointClamp(f32 v, f32 lo, f32 hi) {
    return PointMin(hi, PointMax(lo, v));
}
} // namespace

RPBomLytPoint::RPBomLytPoint() {
    mActive = false;
}

RPBomLytPoint::~RPBomLytPoint() {}

void RPBomLytPoint::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "poseManPoint_00.brlyt");
    mpAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "poseManPoint_00_inOutAll_00.brlan");
    mpAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
}

void RPBomLytPoint::Reset() {
    mpLayout->reset();

    int i = 0;
    do {
        (&mpAnmObj)[i]->resetFrame();
        (&mpAnmObj)[i]->stop();
        (&mpAnmObj)[i]->setFrame(0.0f);
    } while (++i < 1);

    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_unitDob_00")->setMessage(pMessage, 0xAF4, 0, 0);
    mpLayout->findTextBox("T_unitDob_01")->setMessage(pMessage, 0xAF4, 0, 0);
    mpLayout->unbindAllAnmObj();

    int j = 0;
    do {
        mText[j] = 0;
    } while (++j < 4);

    mActive = false;
}

void RPBomLytPoint::Calculate() {
    mpLayout->calc();

    if (mActive && mpAnmObj->isFinished()) {
        RPBomLayoutMgr::sInstance->mPointNum--;
        mActive = false;
    }
}

void RPBomLytPoint::Draw() {
    mpLayout->draw();
}

void RPBomLytPoint::VF_0x18() {}

void RPBomLytPoint::Show(const nw4r::math::VEC2* pScreenPos, s32 points, s32 player,
                         BOOL doubled) {
    RPSysStringUtility::GetStringHalfSizeNumber(points, mText, 4, false);

    const char* centerPanes[2] = {"N_pointCenter_00", "N_dobCenter_00"};
    if (!doubled) {
        mpLayout->findPane(centerPanes[0])->SetVisible(true);
        mpLayout->findPane(centerPanes[1])->SetVisible(false);
    } else {
        mpLayout->findPane(centerPanes[0])->SetVisible(false);
        mpLayout->findPane(centerPanes[1])->SetVisible(true);
    }

    const char* textPanes[4] = {"T_number_00", "T_number_01", "T_numberDob_00",
                                "T_numberDob_01"};
    const char** ppName = &textPanes[doubled * 2];
    int i = 0;
    do {
        mpLayout->findTextBox(*ppName)->setText(mText, 0);
        if (i == 0) {
            u32 color = 0;
            if (!doubled) {
                if (player == 0) {
                    color = 0;
                } else if (player == 1) {
                    color = 1;
                }
            } else {
                if (player == 0) {
                    color = 2;
                } else if (player == 1) {
                    color = 3;
                }
            }
            mpLayout->changeFontColor(*ppName, color);
        }
        ppName++;
    } while (++i < 2);

    // NONMATCHING-DEBT: Show clamps the caller's position in place (RPBomLayoutMgr passes a
    // scratch copy). Declaring pScreenPos non-const (one ledger rename) and writing through it
    // directly moves the y store by one slot (tested 2026-10-29), so the const_cast local stays.
    nw4r::math::VEC2* pPos = const_cast<nw4r::math::VEC2*>(pScreenPos);
    pPos->x = PointClamp(pPos->x, -0.9f, 0.9f);
    pPos->y = PointClamp(pPos->y, -0.9f, 0.9f);

    nw4r::lyt::Pane* pPane = mpLayout->findPane(centerPanes[doubled]);
    const nw4r::math::VEC3& rTrans = pPane->GetTranslate();
    nw4r::math::VEC3 trans(rTrans.x, rTrans.y, rTrans.z);
    trans.x = pPos->x * (EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD) * 0.5f);
    trans.y = pPos->y * (EGG::Screen::GetSizeYMax(EGG::Screen::TV_MODE_STD) * 0.5f);
    pPane->SetTranslate(trans);

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObj);
    mpAnmObj->restart();
    mActive = true;
}
