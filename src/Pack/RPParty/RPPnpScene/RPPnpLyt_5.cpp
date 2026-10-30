// Table Tennis rally counter layout (pingpongCount_00.brlyt), single player.
// Unit 0x80231F54-0x802327AC (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpAudience.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

#include <cwchar>

// Not inlined into its callers in the original
#pragma push
#pragma auto_inline off
void RPPnpCountLyt::SetNumber(s32 number) {
    std::swprintf(mNumberText, 5, L"%d", number);
    for (int i = 0; i < 5; i++) {
        if (mNumberText[i] == 0) {
            break;
        }

        mNumberText[i] += 0xFEE0;
    }

    const char* names[] = {"T_number_00", "T_number_01"};
    for (int i = 0; i < 2; i++) {
        mpLayout->findTextBox(names[i])->setText(mNumberText, 0);
    }
}
#pragma pop

void RPPnpCountLyt::Create() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_SWITCH] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpCountLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongCount_00.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongCount_00_inAll_00.brlan");
    mpAnmObjs[ANM_SWITCH] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongCount_00_switch_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongCount_00_outAll_00.brlan");

    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_number_00")->setMessage(pMessage, 0x8A0, 0, 0);
    mpLayout->findTextBox("T_number_01")->setMessage(pMessage, 0x8A0, 0, 0);
    SetNumber(0);
}

void RPPnpCountLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mState = 0;
    mCount = 0;

    if (mpLayout != NULL) {
        SetNumber(0);
    }
}

void RPPnpCountLyt::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            mState = 1;
        }
        break;
    case 3:
        if (mpAnmObjs[ANM_SWITCH]->isFinished()) {
            mState = 1;
        }
        break;
    case 4:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = 1;
        }
        break;
    }

    mpLayout->calc();
}

void RPPnpCountLyt::Exit() {}

void RPPnpCountLyt::Draw() {
    mpLayout->draw();
}

void RPPnpCountLyt::PlayIn() {
    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}

void RPPnpCountLyt::SetCount(s32 count) {
    mCount = count;
    SetNumber(count);

    if (count != 0 && count % 10 == 0) {
        mState = 3;
        RPSndAudioMgr::instance()->startSystemSe(0x3A);
        RPSndAudioMgr::instance()->startSystemSe(0x37);

        nw4r::math::MTX34 mtx = mpLayout->findPane("N_number_00")->GetGlobalMtx();
        nw4r::math::VEC3 pos;
        pos.x = mtx._03;
        pos.y = mtx._13;
        pos.z = mtx._23;
        nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);

        RPSysEffect effect("Pnp_count", 2, 0);
        effect.create();
        effect.setPos(pos);
        effect.setScale(scale);
        effect.update();

        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[ANM_SWITCH]);
        mpAnmObjs[ANM_SWITCH]->restart();
    }

    switch (count) {
    case 100:
        if (RPPnpGame::sRallyLimit != 100) {
            RPSndAudioMgr::instance()->startSystemSe(0x38);
            RPPnpAudience::GetInstance()->CheerTimed(-1);
        }
        break;
    case 40:
    case 70:
    case 200:
    case 300:
    case 400:
    case 500:
    case 600:
    case 700:
    case 800:
    case 900:
        RPSndAudioMgr::instance()->startSystemSe(0x38);
        RPPnpAudience::GetInstance()->CheerTimed(-1);
        break;
    }
}

void RPPnpCountLyt::PlayOut() {
    mState = 4;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
    mpAnmObjs[ANM_OUT]->restart();
}
