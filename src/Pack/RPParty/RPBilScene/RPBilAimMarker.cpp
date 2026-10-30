// Billiards aim marker (RPBilAimMarker). Unit 802C9BA0-802C9E18 (dtk/game_units.tsv): its thunks
// end the file; .rodata 803273B0-803273C0, .data 8035E1E0-8035E270, .sdata 8043B8C8-8043B8D0,
// .sdata2 804413B8-804413C8.
#include <Pack/RPParty/RPBilScene/RPBilObject.h>
// The original marker derives directly from RPActorBase. RPBilActor adds no data.
#define RPBilActor RPActorBase
#include <Pack/RPParty/RPBilScene/RPBilMarker.h>
#undef RPBilActor
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>

static s32 sAimMarkerFrame = 150;

RPBilAimMarker::~RPBilAimMarker() {}

RPBilAimMarker::RPBilAimMarker(s32 index) : mIndex(index) {}

void RPBilAimMarker::Load() {
    RPGrpModel* pModel = RPGrpModel::Construct(3, "bad_ball_2", 4, 0, 0);
    RPActorBase::mpModel = pModel;
    pModel->SetDrawScene(1);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
    mpModel->GetModelAnm()->RegistFromResFile(
        RPGrpModelAnm::Anm_Clr, 0, 3, "bad_ball_2", 0);
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, sAimMarkerFrame);
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
}

void RPBilAimMarker::Reset() {
    mVisible = false;
}

void RPBilAimMarker::Exit() {}

void RPBilAimMarker::Calculate() {
    if (mVisible) {
        RPActorBase::mpModel->GetModelAnm()->Start(
            RPGrpModelAnm::Anm_Clr, 0, sAimMarkerFrame);
        CalcModelMtx();
    }
}

void RPBilAimMarker::UserDraw() {}

