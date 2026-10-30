#include <Pack/RPKernel.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysParticleManager.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

/**
 * @brief Constructor
 */
RPSysParticleManager::RPSysParticleManager() : mModifyFlags(0) {
    Reset();
}

/**
 * @brief Destructor
 */
RPSysParticleManager::~RPSysParticleManager() {}

/**
 * @brief Prepares this particle manager for creation
 *
 * @param pParent Parent emitter
 * @param pResource Emitter resource data
 * @return Success
 */
bool RPSysParticleManager::Initialize(nw4r::ef::Emitter* pParent,
                                      nw4r::ef::EmitterResource* pResource) {
    bool success = nw4r::ef::ParticleManager::Initialize(pParent, pResource);

    Reset();
    return success;
}

/**
 * @brief Updates the particles' state and applies any modifiers
 */
void RPSysParticleManager::Calc() {
    nw4r::ef::ParticleManager::Calc();

    if (mModifyFlags == 0) {
        return;
    }

    nw4r::ef::Particle* pPrev = NULL;

    RP_NW4R_LIST_FOREACH (nw4r::ef::Particle, it,
                          GetParticleList()->mActiveList) {
        // Wii Play: only active or waiting particles (unsigned range test)
        u32 status = it->GetLifeStatus();

        if (it->mTick != 1) {
            continue;
        }

        if (status - nw4r::ef::ReferencedObject::NW4R_EF_LS_ACTIVE >
            nw4r::ef::ReferencedObject::NW4R_EF_LS_WAIT -
                nw4r::ef::ReferencedObject::NW4R_EF_LS_ACTIVE) {
            continue;
        }

        if (mModifyFlags & EFlag_ModifyLife) {
            it->mLife = mModifyLife;
        }

        nw4r::ef::ParticleParameter* pParameter = it->GetParticleParameter();
        if (pParameter == NULL) {
            continue;
        }

        if (mModifyFlags & EFlag_ModifySize) {
            pParameter->mSize = mModifySize;
        }

        if (mModifyFlags & EFlag_ModifyRotate) {
            pParameter->mRotate = mModifyRotate;
        }

        pPrev = it;
    }
}

/**
 * @brief Sets the life of all active particles
 *
 * @param life New life
 */
void RPSysParticleManager::SetParticleLife(u16 life) {
    mModifyFlags |= EFlag_ModifyLife;
    mModifyLife = life;
}

/**
 * @brief Sets the size of all active particles
 *
 * @param rSize New size
 */
// Wii Play: rSize is read after the flags are stored (unscheduled), y first
#pragma push
#pragma scheduling off
void RPSysParticleManager::SetParticleSize(const nw4r::math::VEC2& rSize) {
    mModifyFlags |= EFlag_ModifySize;
    static_cast<EGG::Vector2f&>(mModifySize).set(rSize.x, rSize.y);
}
#pragma pop

/**
 * @brief Sets the rotation of all active particles
 *
 * @param rRotate New rotation
 */
void RPSysParticleManager::SetParticleRotate(const nw4r::math::VEC3& rRotate) {
    mModifyFlags |= EFlag_ModifyRotate;
    mModifyRotate = rRotate;
}

/**
 * @brief Resets all particle modifiers
 */
void RPSysParticleManager::Reset() {
    mModifyFlags = 0;

    mModifyLife = 0;

    mModifySize.x = 1.0f;
    mModifySize.y = 1.0f;

    mModifyRotate.x = 0.0f;
    mModifyRotate.y = 0.0f;
    mModifyRotate.z = 0.0f;
}
