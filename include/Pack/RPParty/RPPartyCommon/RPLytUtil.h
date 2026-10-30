#ifndef RP_PARTY_COMMON_LYT_UTIL_H
#define RP_PARTY_COMMON_LYT_UTIL_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPGrpScreen;
class RPSysKokeshiIcon;
class RPSysLayout;
class RPSysLytResAccessor;
namespace nw4r {
namespace lyt {
class Pane;
}
} // namespace nw4r

/**
 * @name Layout helpers shared by the minigame layouts (Hky, Pnp, Wly)
 * @note Wii Play-only free functions (ledger Lyt_createLocalResAccessor 801BA40C,
 * Lyt_createLayoutDefault 801BA3FC, Lyt_drawKokeshiIconWithPaneAlpha 801BFE58); the names are a project convention.
 */
//! @{

/**
 * @brief Creates a resource accessor on the default heap and mounts the layout archive pArcName
 * from the scene's local archive
 */
RPSysLytResAccessor* RPCreateLocalLytResAccessor(const char* pArcName);

/**
 * @brief RPSysLayout::create on the default heap
 */
RPSysLayout* RPCreateLayout(RPSysLytResAccessor* pAccessor, const char* pName);

/**
 * @brief Draws a Mii icon on a pane with the pane's alpha (white, or black when black is set),
 * on pScreen or the current screen (ledger Lyt_drawKokeshiIconWithPaneAlpha 801BFE58)
 */
void RPDrawKokeshiIcon(RPSysKokeshiIcon* pIcon, nw4r::lyt::Pane* pPane, BOOL black,
                       RPGrpScreen* pScreen);

/**
 * @brief Value between start and end at frame of frameNum: type 1 linear, 2 ease out (sine),
 * otherwise ease in-out (cosine); start before frame 1, end from frameNum on
 * @note Ledger Pnp_interpolateEased 801BA2A4 (RPPolyline unit); used by Table Tennis.
 */
f32 RPInterpolate(f32 start, f32 end, s32 frame, s32 frameNum, s32 type);

//! @}

//! @}

#endif
