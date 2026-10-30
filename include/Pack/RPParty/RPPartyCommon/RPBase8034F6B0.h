#ifndef RP_PARTY_COMMON_BASE_8034F6B0_H
#define RP_PARTY_COMMON_BASE_8034F6B0_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RPSysAvatar.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPGrpModel;

/**
 * @brief Mii avatar that only has a hand model (no head or body)
 * @details vtable 8034FA08 (ledger name ResTexObj, lyt batch). Size 0xA8 [H] (RPSysAvatar 0xA4
 * plus mpHandModel; RPCls8034F6B0, allocated with 0xA8, adds nothing). The constructor resolves
 * the Mii location like RPSysKokeshi::Construct; LoadResource creates the hand model,
 * textures and colours like RPSysKokeshi.
 * @note Wii Play-only framework class; the name is a project convention. The ledger's
 * RP_Base8034F6B0_dtorBase is this class's destructor.
 */
class RPBase8034F6B0 : public RPSysAvatar {
public:
    explicit RPBase8034F6B0(const RPSysKokeshiGenInfo* pGenInfo);
    virtual ~RPBase8034F6B0(); // at 0x8

    virtual void LoadResource(const RPSysKokeshiOverloadInfo* pOverloadInfo =
                                  NULL);              // at 0xC
    virtual void ApplyLightTexture(u8 drawScene); // at 0x10

    /**
     * @brief Wii Play (801A5030): world matrix of the hand's item (racket) joint [L name]
     */
    void GetItemJointMtx(nw4r::math::MTX34* pMtx);

    // Wii Play: RPPnpPlayer
    RPGrpModel* GetHandModel() const {
        return mpHandModel;
    }

protected:
    /**
     * @brief Resolves the generation location (player, friend or direct) and registers the
     * avatar (RPSysAvatar::Setup); called by the constructor
     */
    void Construct();

    RPGrpModel* mpHandModel; // at 0xA4
};

/**
 * @brief RPBase8034F6B0 with its own vtable and nothing else (Fsh and Pnp players)
 * @details vtable 8034F6B0. Size 0xA8, verified (Fsh_Player, Pnp_Player).
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPCls8034F6B0 : public RPBase8034F6B0 {
public:
    explicit RPCls8034F6B0(const RPSysKokeshiGenInfo* pGenInfo);
    virtual ~RPCls8034F6B0(); // at 0x8
};

//! @}

#endif
