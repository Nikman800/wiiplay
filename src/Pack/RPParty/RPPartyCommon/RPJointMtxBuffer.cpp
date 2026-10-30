#include <Pack/RPParty/RPPartyCommon/RPJointMtxBuffer.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <egg/math/eggMatrix.h>

#include <nw4r/g3d/g3d_scnobj.h>

RPJointMtxBuffer::~RPJointMtxBuffer() {}

RPJointMtxBuffer::RPJointMtxBuffer(RPGrpModel* pModel) {
    mJointNum = pModel->GetJointNum();
    mpLocalMtx = new EGG::Matrix34f[mJointNum];
    mpWorldMtx = new EGG::Matrix34f[mJointNum];

    pModel->SetCallback(this);
    mCurJoint = 0;
    pModel->SetCallbackJointIndex(0);

    EGG::Matrix34f mtx;
    mtx.makeIdentity();
    nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pModel->Calc();
    pModel->SetCallback(NULL);
}

void RPJointMtxBuffer::EditJointLocalMtx(RPGrpModel* pModel, u16 idx,
                                         nw4r::math::MTX34* pMtx,
                                         nw4r::math::VEC3*) {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) {
            mpLocalMtx[idx].m[row][col] = pMtx->m[row][col];
        }
    }

    if (mCurJoint >= mJointNum - 1) {
        return;
    }

    ++mCurJoint;
    pModel->SetCallbackJointIndex(mCurJoint);
}

void RPJointMtxBuffer::EditWorldMtx(RPGrpModel* pModel) {
    for (u16 idx = 0; idx < mJointNum; ++idx) {
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                mpWorldMtx[idx].m[row][col] = pModel->ReferWorldMtx(idx)->m[row][col];
            }
        }
    }
}

const nw4r::math::MTX34& RPJointMtxBuffer::GetLocalMtx(u16 idx) const {
    return mpLocalMtx[idx];
}

const nw4r::math::MTX34& RPJointMtxBuffer::GetWorldMtx(u16 idx) const {
    return mpWorldMtx[idx];
}
