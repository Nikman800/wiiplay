#include <egg/gfxe.h>
#include <egg/prim.h>

#include <nw4r/g3d.h>
#include <nw4r/math.h>

#include <algorithm>

namespace EGG {

// Unreferenced (the linker strips it); its assert strings open this file's string pool.
static void initAABB(ModelBoundingInfo::AABBShape* pAABB,
                     const nw4r::math::VEC3* min, const nw4r::math::VEC3* max) {
    EGG_ASSERT(pAABB);
    EGG_ASSERT(min);
    EGG_ASSERT(max);

    pAABB->min = *min;
    pAABB->max = *max;
}

static inline void calcAABB(ModelBoundingInfo::AABBShape* pAABB,
                             const nw4r::math::VEC3* min,
                             const nw4r::math::VEC3* max,
                             nw4r::math::VEC3* (&pBases)[3]) {
    ModelBoundingInfo::AABBShape aabb;
    aabb.min = *min;
    aabb.max = *max;

    nw4r::math::VEC3 delta;
    nw4r::math::VEC3Sub(&delta, &aabb.max, &aabb.min);
    f32 ext[3];
    ext[0] = nw4r::math::FAbs(nw4r::math::VEC3Dot(&delta, pBases[0]));
    ext[1] = nw4r::math::FAbs(nw4r::math::VEC3Dot(&delta, pBases[1]));
    ext[2] = nw4r::math::FAbs(nw4r::math::VEC3Dot(&delta, pBases[2]));

    *pAABB = aabb;

    nw4r::math::VEC3 point;
    for (int i = 0; i < 8; i++) {
        if (i < 6) {
            nw4r::math::VEC3Scale(&point, pBases[i % 3],
                                  ext[i % 3] * (i < 3 ? 1.0f : -1.0f));
            nw4r::math::VEC3Add(&point, i < 3 ? &aabb.min : &aabb.max,
                                &point);
        } else {
            point = i == 6 ? aabb.min : aabb.max;
        }

        if (point.x < pAABB->min.x) {
            pAABB->min.x = point.x;
        } else if (point.x > pAABB->max.x) {
            pAABB->max.x = point.x;
        }

        if (point.y < pAABB->min.y) {
            pAABB->min.y = point.y;
        } else if (point.y > pAABB->max.y) {
            pAABB->max.y = point.y;
        }

        if (point.z < pAABB->min.z) {
            pAABB->min.z = point.z;
        } else if (point.z > pAABB->max.z) {
            pAABB->max.z = point.z;
        }
    }
}

ModelBoundingInfo::ModelBoundingInfo(u32 flags, ModelEx* pMdl)
    : m_pMdl(pMdl), unk4(1.0f), m_pAABB(NULL), m_pSphere(NULL) {

#line 148
    EGG_ASSERT(m_pMdl);
    EGG_ASSERT(m_pMdl->getNumShape() > 0);

    m_pMdl->attachBoundingInfo(this);
    m_pBV = createShape<AABBShape>(true);

    for (u16 i = 0; i < m_pMdl->getNumShape(); i++) {
        AABBShape box;
        pMdl->getShapeMinMax(i, &box.min, &box.max, i == 0);

        if (box.min.x > box.max.x) {
            std::swap(box.min.x, box.max.x);
        }

        if (box.min.y > box.max.y) {
            std::swap(box.min.y, box.max.y);
        }

        if (box.min.z > box.max.z) {
            std::swap(box.min.z, box.max.z);
        }

        if (i != 0) {
            setAABB(m_pBV, box.min, box.max);
        } else {
            m_pBV->m_shape = box;
        }

        m_pBV->m_pShapeSet[i] = box;
    }

    if (flags & cBoundFlag_AABB) {
        m_pAABB = createShape<AABBShape>(flags & (cBoundFlag_AABB | cBoundFlag_AABBSet));
    }

    if (flags & cBoundFlag_Sphere) {
        m_pSphere = createShape<SphereShape>(flags & (cBoundFlag_Sphere | cBoundFlag_SphereSet));
    }

    calc();
}

ModelBoundingInfo::~ModelBoundingInfo() {
    deleteShape(m_pBV);

    if (m_pAABB != NULL) {
        deleteShape(m_pAABB);
    }

    if (m_pSphere != NULL) {
        deleteShape(m_pSphere);
    }
}

void ModelBoundingInfo::calc() {
    nw4r::math::MTX34 mtx;
    m_pMdl->getLocalMtx(&mtx);

    nw4r::math::VEC3 bx(mtx._00, mtx._10, mtx._20);
    nw4r::math::VEC3 by(mtx._01, mtx._11, mtx._21);
    nw4r::math::VEC3 bz(mtx._02, mtx._12, mtx._22);
    nw4r::g3d::ScnLeaf* pScnLeaf = m_pMdl->getScnLeaf();
    nw4r::math::VEC3* pBases[] = {&bx, &by, &bz};

    nw4r::math::VEC3 scale;

    if (pScnLeaf != NULL) {
        scale = pScnLeaf->GetScale();
    } else {
        scale.x = scale.y = scale.z = 1.0f;
    }

    mtx._00 *= scale.x;
    mtx._10 *= scale.x;
    mtx._20 *= scale.x;

    mtx._01 *= scale.y;
    mtx._11 *= scale.y;
    mtx._21 *= scale.y;

    mtx._02 *= scale.z;
    mtx._12 *= scale.z;
    mtx._22 *= scale.z;

    nw4r::math::VEC3 min;
    nw4r::math::VEC3 max;
    nw4r::math::VEC3Transform(&min, &mtx, &m_pBV->m_shape.min);
    nw4r::math::VEC3Transform(&max, &mtx, &m_pBV->m_shape.max);

    f32 s = unk4;
    if (s != 1.0f) {
        nw4r::math::VEC3 expand(0.5f * (s - 1.0f) * (max.x - min.x),
                                0.5f * (s - 1.0f) * (max.y - min.y),
                                0.5f * (s - 1.0f) * (max.z - min.z));

        min.x -= expand.x;
        min.y -= expand.y;
        min.z -= expand.z;
        max.x += expand.x;
        max.y += expand.y;
        max.z += expand.z;
    }

    nw4r::math::VEC3 setMin[255];
    nw4r::math::VEC3 setMax[255];

    if ((m_pAABB != NULL && m_pAABB->m_pShapeSet != NULL) ||
        (m_pSphere != NULL && m_pSphere->m_pShapeSet != NULL)) {

        for (u16 i = 0; i < m_pMdl->getNumShape(); i++) {
            nw4r::math::VEC3Transform(&setMin[i], &mtx,
                                      &m_pBV->m_pShapeSet[i].min);
            nw4r::math::VEC3Transform(&setMax[i], &mtx,
                                      &m_pBV->m_pShapeSet[i].max);

            if (unk4 != 1.0f) {
                f32 sc = 0.5f * (unk4 - 1.0f);

                nw4r::math::VEC3 expand(sc * (setMax[i].x - setMin[i].x),
                                        sc * (setMax[i].y - setMin[i].y),
                                        sc * (setMax[i].z - setMin[i].z));

                setMin[i].x -= expand.x;
                setMin[i].y -= expand.y;
                setMin[i].z -= expand.z;
                setMax[i].x += expand.x;
                setMax[i].y += expand.y;
                setMax[i].z += expand.z;
            }
        }
    }

    if (m_pAABB != NULL) {
        calcAABB(&m_pAABB->m_shape, &min, &max, pBases);

        if (m_pAABB->m_pShapeSet != NULL) {
            for (u16 i = 0; i < m_pMdl->getNumShape(); i++) {
                calcAABB(&m_pAABB->m_pShapeSet[i], &setMin[i], &setMax[i], pBases);
            }
        }
    }

    if (m_pSphere != NULL) {
        setSphere(&m_pSphere->m_shape, min, max);

        if (m_pSphere->m_pShapeSet != NULL) {
            for (u16 i = 0; i < m_pMdl->getNumShape(); i++) {
                setSphere(&m_pSphere->m_pShapeSet[i], setMin[i], setMax[i]);
            }
        }
    }
}

} // namespace EGG
