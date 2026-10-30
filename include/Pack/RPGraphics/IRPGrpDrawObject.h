#ifndef RP_GRAPHICS_I_DRAW_OBJECT_H
#define RP_GRAPHICS_I_DRAW_OBJECT_H
#include <Pack/types_pack.h>

//! @addtogroup rp_graphics
//! @{

/**
 * @brief Interface for renderable objects
 */
class IRPGrpDrawObject {
    // Wii Play: the renderer walks the draw list
    friend class RPGrpRenderer;

private:
    //! Next renderable in the linked list
    IRPGrpDrawObject* mpNext; // at 0x0

public:
    /**
     * @brief Constructor
     */
    IRPGrpDrawObject() : mpNext(NULL) {}

    /**
     * @brief Destructor
     */
    virtual ~IRPGrpDrawObject() {} // at 0x8

    /**
     * @brief Standard draw pass
     */
    virtual void UserDraw() {} // at 0xC

    /**
     * @brief Additional draw pass for debugging
     */
    virtual void DebugDraw() {} // at 0x10
};

//! @}

#endif
