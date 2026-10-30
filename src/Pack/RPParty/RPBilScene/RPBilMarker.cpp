#include <Pack/RPParty/RPBilScene/RPBilMarker.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>

static s32 sCueGhostFrame[2] = {128, 0};

RPBilCueGhost::~RPBilCueGhost() {}

RPBilCueGhost::RPBilCueGhost() {}

void RPBilCueGhost::Load() {
    RPGrpModel* pModel = RPGrpModel::Construct(3, "bad_ball_2", 1, 0, 0);
    mpModel = pModel;
    pModel->SetDrawScene(2);

    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
    mpModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Clr, 0, 3, "bad_ball_2");
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
}

void RPBilCueGhost::Reset() {
    mVisible = false;
    mIsValid = false;
}

void RPBilCueGhost::Calculate() {}

void RPBilCueGhost::Exit() {
    if (!mVisible) {
        return;
    }

    if (mIsValid) {
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 255.0f);
    } else {
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0,
                                      static_cast<f32>(sCueGhostFrame[0]));
    }

    CalcModelMtx();
}

void RPBilCueGhost::UserDraw() {}
