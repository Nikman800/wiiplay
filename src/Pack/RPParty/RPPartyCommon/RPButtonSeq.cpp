#include <Pack/RPParty/RPPartyCommon/RPButtonSeq.h>

const u16 RPButtonSeqHolder::sSequence[] = {
    0x0800, 0x0008, 0x0100, 0x0004, 0x0400, 0x0200,
    0x0200, 0x0400, 0x0001, 0x0800, 0x0002, 0x0000,
};

RPButtonSeqHolder::RPButtonSeqHolder() {
    mpSeq = new RPButtonSeq(0, sSequence);
}

RPButtonSeqHolder::~RPButtonSeqHolder() {}

bool RPButtonSeqHolder::IsComplete() const {
    return mpSeq->IsComplete();
}

void RPButtonSeqHolder::Update() {
    mpSeq->Update();
}
