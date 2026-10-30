#include <Pack/RPGraphics/RPGrpModelRecord.h>

#include <Pack/RPGraphics/RPGrpModel.h>

#include <nw4r/g3d.h>

namespace {

struct RecordFrame {
    nw4r::math::MTX34 mRootMtx;
    nw4r::math::MTX34* mpJointMtxArray;
};

class RecordFrames {
public:
    u16 mCount;
    RecordFrame* mpFrames;

    virtual ~RecordFrames();
    virtual void Record(RecordFrame* pFrame, nw4r::math::MTX34* pRootMtx);
    virtual void Interpolate(f32 fraction, nw4r::math::MTX34* pRootMtx,
                             RecordFrame* pBegin, RecordFrame* pEnd);
};

} // namespace

void RPGrpModelRecord::Calc(RPGrpModel* pModel) {
    int jointNum = pModel->GetJointNum() == 0 ? 1 : pModel->GetJointNum();

    if (mFlags & 1) {
        for (u32 i = 0; static_cast<int>(i) < jointNum; i++) {
            nw4r::math::MTX34Copy(&mpJointMtxArray[i], pModel->ReferWorldMtx(i));
        }

        PSMTXCopy(pModel->GetScnObj()->GetMtxPtr(
                      nw4r::g3d::ScnObj::MTX_LOCAL)[0],
                  mRootMtx);

        RecordFrames* pFrames = static_cast<RecordFrames*>(mpFrames);
        pFrames->Record(&pFrames->mpFrames[mRecordFrame], &mRootMtx);
    } else if (mFlags & 2) {
        f32 position = mFrame;
        int frame = static_cast<int>(position);
        RecordFrames* pFrames = static_cast<RecordFrames*>(mpFrames);
        u16 endFrame = frame + 1;

        if (endFrame >= pFrames->mCount) {
            endFrame = 0;
        }

        pFrames->Interpolate(position - static_cast<int>(position), &mRootMtx,
                             &pFrames->mpFrames[static_cast<u16>(frame)],
                             &pFrames->mpFrames[endFrame]);

        for (u32 i = 0; static_cast<int>(i) < jointNum; i++) {
            nw4r::math::MTX34Copy(pModel->ReferWorldMtx(i), &mpJointMtxArray[i]);
        }

        nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
        if (pScnObj != NULL) {
            pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mRootMtx);
        }
    }

    mFlags = 0;
}
