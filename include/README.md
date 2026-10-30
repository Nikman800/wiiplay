# include/

Vendored from [doldecomp/ogws](https://github.com/doldecomp/ogws) (Wii Sports decompilation),
commit `f5a1bb5ed7692a9e09e737bc1236f592274dcf73`, licensed CC0-1.0 (`LICENSE.ogws`). These are community-written headers for
the RVL SDK, NintendoWare (nw4r), EGG, RVLFaceLib, HomeButtonMiniLib, MSL, MetroTRK and the RP
framework (`Pack/`) shared with Wii Play. They describe Wii Sports' library versions; where
Wii Play's mid-2007 versions differ, edit the header here and note it in the commit.

Project-specific headers: `hky.h` (early plain-C Laser Hockey layouts, still used by
`src/Hky/FieldWallIter.cpp` and `src/probe/`; superseded by `Pack/RPParty/RPHkyScene/`).
