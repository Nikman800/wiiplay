# Phase 4: status and open items

Summary of the game-code decompilation (the day-by-day working log is kept in the private
development repository).

## Where things stand

**487 of 491 game units link from source and the DOL matches byte for byte (2026-10-29; RPGrpRenderer absorbed RPScnProcDrawHook, RPGrpView_2 and RPScnProcDrawHook_2, so the count is 491).** All nine
minigames are complete; the RP framework and Party common are 204/217. The 4 left (RPSysTagProcessor, RPCPadCursor_2, RPCPadCursor_3,
RPKokeshiMgr_3; RPPathFollower linked 2026-10-29) are the one-function holdouts (parked 2026-10-26, reopened by the user 2026-10-28; Open items). Two units are accepted FAKEMATCHes
(RPTnkMine_2, RPCowFieldObj). Phase 5 has 636 of 637 (BTE split into 68 units) library units linked (2026-10-28, docs/PLAN.md).

| Minigame | Units | Linked from source | Not linked |
|---|---|---|---|
| Shooting Range (RPDucScene) | 36 | 36 | none |
| Fishing (RPFshScene) | 22 | 22 | none |
| Tanks (RPTnkScene) | 44 | 44 (Mine_2 is a FAKEMATCH) | none |
| Laser Hockey (RPHkyScene) | 21 | 21 | none |
| Table Tennis (RPPnpScene) | 29 | 29 | none |
| Pose Mii (RPBomScene) | 29 | 29 | none |
| Billiards (RPBilScene) | 31 | 31 | none |
| Find Mii (RPWlyScene) | 31 | 31 | none |
| Charge! (RPCowScene) | 34 | 34 (RPCowFieldObj is a FAKEMATCH, needs your OK) | none |

Every unit has source (RPTnkScene.cpp was the last, linked in 8422f96). Linked
units are listed in `config/RHAE01/matching.txt`; `tools/unit.py status <unit>` gives the
per-function state of the others.

## Open items (updated 2026-10-29: Phases 4 and 5 marked done; the holdouts are functional and marked)

- **Four unlinked game units**, each blocked at the compiler level (closest natural C in source, full
  notes in docs/NONMATCHING.md): RPSysTagProcessor ProcessRuby (4 differences: y/descent float
  registers), RPKokeshiMgr_3 ctor (8: backend value numbering merges a load the original reloads),
  RPCPadCursor_2/_3 SetTarget (pane-load aliasing plus PCode order).
- **Phase 5:** nw4r/ef/ef_animcurve.cpp is the last library unit (75 differences; see NONMATCHING.md).
- **Natural-C debt** is tracked in docs/NONMATCHING.md ("Natural-C debt"); every remaining item
  carries a `// NONMATCHING-DEBT:` comment with what the natural form costs.
- **RPCowFieldObj (FAKEMATCH, accepted by the user 2026-10-26):** VF_0x3C's inlined RPCowObjSound ctor matches
  only with a dead `CtorScope` stand-in temporary (RPCowSound.h; its weak dtor is stripped) plus
  `#pragma opt_dead_code off`. The likely real shape is RPCowObjSound deriving from
  `TRPSndGameObject<4>` (matches with opt_dead_code off), but that adds weak TRPSndGameObject<4>
  data to RPCowPlayer_6/RPCowSound_2. Revert = drop the unit from matching.txt and the
  CtorScope/default ctor from RPCowSound.h.
  2026-10-29 (--sched): CtorScope + opt_dead_code off keeps TRPSndObject<4>'s overwritten vtable
  stores alive through the front end (the back end later removes them, leaving 4 dead PCode
  instructions that shift the vreg numbering and the pre-RA schedule); the original did the same by
  some construct in the base-initialiser expression that leaves no code. The TRPSndGameObject<4> level
  is ruled out by the destructors (RPCowObjSound/RPCowPlayerSound dtors gain a null check each).
- **RPTnkMine_2 (FAKEMATCH, accepted 2026-10-23):** the missing 0x90 bytes of zero-filled weak
  data come from `RPSndGameObjectHelper` in RPSndObject.h, an invented class (34 placeholder
  slots). 2026-10-29: it is not a TRPSndGameObject vtable (that would be 0xDC; RPBomPlayer has no
  TRPSndGameObject level yet keeps the same 0x90 after its TRPSndObject<4> family). Only RPTnkMine_2
  and RPBomPlayer keep it (not RPBomPanel_2); it correlates with units that define a sound actor
  destructor (spSoundArchivePlayer, __destroy_arr), so it is an unreferenced weak object from the
  destructor side (a never-stored vtable or an inline's local static table). Separate from Cow's
  CtorScope. RPBomPlayer instantiates it through an extern "C" stand-in (Bom_Player_instantiateSndHelper).
- **RPDucSound.h stand-in:** `RPDucSndObjectInstantiate<1>()` reproduces RPDucGame's 0x168-byte
  zero block; the original's template argument is unknown (N=4 would steal RPWlySound's thunks).
- Every unit named in the older near-miss and ledger notes (RPResFile_3, RPSysControllerMgr,
  RPSysHomeMenuMgr, RPSysPauseMenu, RPSysKokeshiIcon, RPSysLayout, RPGrpModelRfl, RPScene80358188,
  RPGrpRenderer with RPScnProcDrawHook/RPGrpView_2 merged in, RPGrpModelMaterial, RPGrpModelScene,
  RPSelectGameLyt, RPSysSceneCreator with its .rodata, RPActorBase, RPCollisionModel, RPPathFollower)
  links now; their history is in the development log.

## Stand-in functions (invented, user-approved 2026-10-28)

Pool-order stand-ins (evidence recorded 2026-10-29; each was deleted and the object compared):
- RPFshFish `Fsh_fishPoolOrderS32`: .sdata2 s32->f32 double lbl_8043F368 between 0.001f and 0.9f (otherwise after 0.8f).
- RPBilCamera_2 `Bil_cameraPoolOrderEpsilon`: .sdata2 FLT_EPSILON lbl_804411EC before Reset's 180.0f/0.5f/0.711111f.
- RPTnkWeaponManager `Tnk_weaponPoolOrderHalf`: code evidence: the 0.5f literal (lbl_804403E0) must exist before LayMine, which loads it before `stw r31`; pool order unaffected.
- RPBomPanel_2 `Bom_panelPoolOrderU32`: .sdata2 u32->f32 double lbl_804417B0 between 14.2222f and Spawn's 180.0f.
- RPRumblePattern `RP_rumblePoolOrderZero` (0.0f first) and `RP_rumblePoolOrderS32` (double before 1.0f); `RP_rumblePoolOrderOne` had no evidence and was removed.
- RPBilBall `Bil_ballPoolOrderU32`: u32->f32 double lbl_804412C8 after 0.0f, before 0.01f.
- RPBilBallMgr `Bil_ballMgrPoolOrderZero`: 0.0f lbl_80441320 before PlaceCueBall's 1.0f.
- RPCrowdMii `Party_crowdPoolOrder`: 0.1f lbl_8043F250 before 20.0f.
- RPDucBalloon `Duc_balloonPoolOrderU32` (double before 1.0f/-1.0f/FLT_EPSILON) and `Duc_balloonPoolOrderOne` (1.0f before -1.0f/FLT_EPSILON).
- RPDucShot `Duc_shotPoolOrderU32`: u32->f32 double lbl_8043FEC8 between 0.0656f and 0.082f.
- RPPlayerNumSelect_2 `SePoolOrder`: .rodata order of the nine RP_COMMON_SE_* strings (0x80319E58..0x80319F47).
Where the original object evidently held a function the linker later dead-stripped (an unreferenced
string in the pool, a constant-pool order, a weak instance ahead of the first remaining function),
an invented stand-in reproduces what it left behind. Each is marked "STAND-IN" in its source;
replace it if the real function is identified.
- egg/gfxe/eggPostEffectMaskDOF.cpp: DOFStringDemo::f (the "m_pDstScreen" assert string).
- egg/gfxe/eggFrustum.cpp: FrustumStandIn (1.0f first in .sdata2), FrustumStandInFovy (SetFovy's
  constants before CalcMtxPerspective_'s -1.0f), FrustumStandInVec (the "pVec" assert string).
- egg/gfxe/eggDrawPathBase.cpp: class DrawPathBaseStandIn (derived, out-of-line ctor and reset()):
  gives the original function and IScnProc thunk order. Evidence is indirect (the order itself;
  nothing of a derived class survives), unlike the leftover strings and constants above. Checked
  2026-10-28: Skyward Sword's matching DrawPathBase is a later EGG version (it adds
  nw4r::g3d::IScnObjCallback as a third base); Wii Play's vtable has no such base, so it does not
  explain our order.
- egg/geom/eggPlane.cpp: PlaneStandIn (the weak operator*(f32, const Vector3f&) before project).
