// Matrix construction and RPSimpleModel's shared vector/matrix statics (unit 0x801B83A8-0x801B84F0).
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

extern const f32 lbl_8043EAA8;
extern const f32 lbl_8043EAAC;

extern "C" {
nw4r::math::VEC2 RP_vec2ZeroB(lbl_8043EAA8, lbl_8043EAA8);
}
nw4r::math::VEC3 RPSimpleModel::sVecZero(lbl_8043EAA8, lbl_8043EAA8, lbl_8043EAA8);
nw4r::math::VEC3 RPSimpleModel::sVecEx(lbl_8043EAAC, lbl_8043EAA8, lbl_8043EAA8);
nw4r::math::VEC3 RPSimpleModel::sVecEy(lbl_8043EAA8, lbl_8043EAAC, lbl_8043EAA8);
nw4r::math::VEC3 RPSimpleModel::sVecEz(lbl_8043EAA8, lbl_8043EAA8, lbl_8043EAAC);
nw4r::math::VEC3 RPSimpleModel::sVecOne(lbl_8043EAAC, lbl_8043EAAC, lbl_8043EAAC);

// The identity matrix as its three rows, copy-constructed from the axes (word copies, no temporary).
nw4r::math::VEC3 RPSimpleModel::sMtxIdentity[3] = {RPSimpleModel::sVecEx, RPSimpleModel::sVecEy,
                                                    RPSimpleModel::sVecEz};

extern "C" void RP_makeMtxFromRotPos(nw4r::math::MTX34* pMtx,
                                     const nw4r::math::VEC3* pPos,
                                     const nw4r::math::MTX33* pRot) {
    pMtx->_00 = pRot->_00;
    pMtx->_10 = pRot->_01;
    pMtx->_20 = pRot->_02;
    pMtx->_01 = pRot->_10;
    pMtx->_11 = pRot->_11;
    pMtx->_21 = pRot->_12;
    pMtx->_02 = pRot->_20;
    pMtx->_12 = pRot->_21;
    pMtx->_22 = pRot->_22;
    pMtx->_03 = pPos->x;
    pMtx->_13 = pPos->y;
    pMtx->_23 = pPos->z;
}

const f32 lbl_8043EAA8 = 0.0f;
const f32 lbl_8043EAAC = 1.0f;
