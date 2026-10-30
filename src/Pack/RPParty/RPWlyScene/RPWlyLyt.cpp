#include <Pack/RPParty/RPWlyScene/RPWlyLyt.h>
class RPGrpCamera;
#include <Pack/RPGraphics/RPGrpRenderer.h>

RPWlyLayoutSet* RPWlyLayoutSet::sInstance;

void RPWlyLayoutSet::CreateInstance() {
    sInstance = new RPWlyLayoutSet;
}

void RPWlyLayoutSet::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPWlyLayoutSet::~RPWlyLayoutSet() {
    mpStartCount = NULL;
    mpDisplay = NULL;
    mpDisplayEnd = NULL;
    mpTimer = NULL;
    mpQuestion = NULL;
    mpMessage = NULL;
    mpPointPopups = NULL;
    mpEndScore = NULL;
    mpComplete = NULL;
}

void RPWlyLayoutSet::Init() {
    mpStartCount->Init();
    mpDisplay->Init();
    mpDisplayEnd->Init();
    mpTimer->Init();
    mpQuestion->Init();
    mpMessage->Init();
    mpPointPopups->Init();
    mpEndScore->Init();
    mpComplete->Init();
}

void RPWlyLayoutSet::Create() {
    mpStartCount->Create();
    mpDisplay->Create();
    mpDisplayEnd->Create();
    mpTimer->Create();
    mpQuestion->Create();
    mpMessage->Create();
    mpPointPopups->Create();
    mpEndScore->Create();
    mpComplete->Create();
}

void RPWlyLayoutSet::Reset() {
    mpStartCount->Reset();
    mpDisplay->Reset();
    mpDisplayEnd->Reset();
    mpTimer->Reset();
    mpQuestion->Reset();
    mpMessage->Reset();
    mpPointPopups->Reset();
    mpEndScore->Reset();
    mpComplete->Reset();
}

void RPWlyLayoutSet::Calculate() {
    mpStartCount->Calculate();
    mpDisplay->Calculate();
    mpDisplayEnd->Calculate();
    mpTimer->Calculate();
    mpQuestion->Calculate();
    mpMessage->Calculate();
    mpPointPopups->Calculate();
    mpEndScore->Calculate();
    mpComplete->Calculate();
}

void RPWlyLayoutSet::Destroy() {
    mpStartCount->Destroy();
    mpDisplay->Destroy();
    mpDisplayEnd->Destroy();
    mpTimer->Destroy();
    mpQuestion->Destroy();
    mpMessage->Destroy();
    mpPointPopups->Destroy();
    mpEndScore->Destroy();
    mpComplete->Destroy();
}

void RPWlyLayoutSet::Draw() {
    if (RPGrpRenderer::GetDrawStep() == 5) {
        switch (RPGrpRenderer::GetCurrentViewNo()) {
        case 3:
            mpPointPopups->Draw();
            mpStartCount->Draw();
            mpDisplayEnd->Draw();
            mpTimer->Draw();
            mpQuestion->Draw();
            mpMessage->Draw();
            mpEndScore->Draw();
            mpComplete->Draw();
            break;
        case 5:
            mpDisplay->Draw();
            break;
        }
    }
}
