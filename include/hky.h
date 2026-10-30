/*
 * Laser Hockey (Hky) - recovered object layouts for Wii Play (RHAE01, Rev 1).
 *
 * Inferred from Ghidra pseudocode of the shipped DOL; not original source.
 * Sizes come from operator new (fn_800A39F8) calls in Hky_Game_ctor; offsets
 * from tools/field_xref.py plus reading the listed methods.
 * Confidence per field:  [H] code shows it directly  [M] strong inference  [L] guess.
 * unkXXX = accessed but meaning not established; padXXX = never accessed by Hky code.
 *
 * Plain C (originally for Ghidra import; superseded by include/Pack/RPParty/RPHkyScene/).
 */
#ifndef HKY_H
#define HKY_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

typedef struct Vec3 {
    f32 x, y, z;
} Vec3;

/* Rink wall / paddle edge for 2D (XZ) collision. 0x1C bytes.
 * Built in Hky_Field_create: first wall = pos (x, 0, -220), dir (1, 0, 0). */
typedef struct HkySegment {
    Vec3 pos;        /* 0x00 [H] start point */
    Vec3 dir;        /* 0x0C [H] unit direction */
    f32  length;     /* 0x18 [M] extent along dir */
} HkySegment;

/* 3x3 rotation basis: one aggregate (CodeWarrior copies it with word moves). 0x24 bytes. */
typedef struct RPAxes {
    Vec3 x, y, z;
} RPAxes;

/* Common base of every Hky world object and phase (RP_SimpleModel_ctor). 0x44 bytes.
 * Startup constants (RP_sinit_80439170): scale (1,1,1), axes X/Y/Z = identity. */
typedef struct RPSimpleModel {
    void *vtable;    /* 0x00 [H] */
    Vec3  pos;       /* 0x04 [H] world position; play surface is the XZ plane */
    Vec3  scale;     /* 0x10 [H] reset to (1,1,1) */
    RPAxes axes;     /* 0x1C [H] rotation basis, copied as one 36-byte aggregate (resetState);
                        puck rebuilds axes.x = axes.y x axes.z */
    void *model;     /* 0x40 [H] g3d model wrapper (flag at +0x8 bit0 = visible) */
} RPSimpleModel;

typedef struct HkyPuckTrail HkyPuckTrail;     /* effect holder, Hky_PuckTrail_* */
typedef struct HkyPaddleTrail HkyPaddleTrail; /* effect holder, Hky_PaddleTrail_* */

/* The puck. 0xC8 bytes, Game+0x1C. vtable 803592A4. */
typedef struct HkyPuck {
    RPSimpleModel base;             /* 0x00 */
    s32   unk44;                    /* 0x44 [L] passed to goal effect calls */
    HkyPuckTrail *trail;            /* 0x48 [M] Hky_balltail effect; hide() calls its vfunc +0x10 */
    u8    active;                   /* 0x4C [L] cleared by reset, set by serve */
    u8    pad4D[3];
    u8    inPlay;                   /* 0x50 [M] Hky_Puck_setInPlay */
    u8    pad51[3];
    s32   paddleHitCooldown[4];     /* 0x54 [M] per-paddle frame counters, wrap to 0 (handleGoal) */
    f32   paddleHitValue[4];        /* 0x64 [L] per-paddle float fed to hit effect */
    s32   hitCooldown[3];           /* 0x74 [L] same counter pattern as 0x54 */
    Vec3  posAccum[2];              /* 0x80 [H] pos += posAccum[0] + posAccum[1]; cleared each step */
    Vec3  velAccum[2];              /* 0x98 [H] vel += velAccum[0] + velAccum[1]; cleared each step */
    f32   spin;                     /* 0xB0 [H] spin += spinImpulse[0..1], clamped to +-max */
    f32   spinImpulse[2];           /* 0xB4 [H] cleared each step */
    Vec3  velocity;                 /* 0xBC [H] XZ damped by friction, Y += gravity, XZ speed capped */
} HkyPuck;

/* The rink. 0x240 bytes, Game+0x20. vtable 803592D0. */
typedef struct HkyField {
    RPSimpleModel base;             /* 0x00 */
    HkySegment walls[18];           /* 0x44 [H] built in Hky_Field_create */
    HkySegment *wallIter;           /* 0x23C [H] puck iteration starts at walls[15], paddle at walls[17] */
} HkyField;

/* A player's paddle. 0x1CC bytes, Game+0x24 / +0x28 (ctor arg = player 0/1). vtable 80359404. */
typedef struct HkyPaddle {
    RPSimpleModel base;             /* 0x00 */
    Vec3  accum[4];                 /* 0x44 [M] collision/force accumulators, cleared in applyVelocity */
    u8    pad74[0xC];
    Vec3  velocity;                 /* 0x80 [H] pos.x += velocity.x, pos.z += velocity.z */
    Vec3  posHistory[5];            /* 0x8C [H] shifted every frame in updateControl, newest last */
    Vec3  prevPos;                  /* 0xC8 [M] copied from pos in updateControl */
    f32   history[4];               /* 0xD4 [M] shifted every frame in updateControl */
    f32   unkE4;                    /* 0xE4 [L] */
    f32   prevAngle;                /* 0xE8 [L] */
    f32   angle;                    /* 0xEC [M] steps of 0.1396 rad (8 deg) */
    HkyPaddleTrail *trail;          /* 0xF0 [H] Hky_PaddleTrail_updateBySpeed target */
    void *unkF4;                    /* 0xF4 [L] object with vtable; used by calc/setShape */
    s32   playerIndex;              /* 0xF8 [M] most-read field; set from ctor arg */
    s32   shapeType;                /* 0xFC [L] read by collision + CPU target (rect vs round) */
    s32   state;                    /* 0x100 [L] */
    s32   rumbleTimer;              /* 0x104 [L] Hky_Paddle_requestRumble */
    u8    pad108[8];
    s32   unk110;                   /* 0x110 [L] updateMovement */
    s32   unk114;                   /* 0x114 [L] requestRumble */
    u8    pad118[8];
    s32   unk120;                   /* 0x120 [L] updateMovement */
    /* CPU opponent block (calcCpuTargetRect / calcCpuTargetRound / updateCpuStallTimer) */
    f32   cpuTargetX;               /* 0x124 [L] */
    s32   cpuStallTimer;            /* 0x128 [M] */
    s32   cpuUnk12C;                /* 0x12C [L] */
    f32   cpuUnk130;                /* 0x130 [L] */
    s32   cpuUnk134;                /* 0x134 [L] */
    f32   cpuUnk138;                /* 0x138 [L] */
    u32   cpuUnk13C[7];             /* 0x13C [L] set in ctor/reset, used by updateMovement */
    HkySegment edges[4];            /* 0x158 [H] paddle outline, built by setShape */
    HkySegment *edgeIter;           /* 0x1C8 [H] */
} HkyPaddle;

/* Round paddle model + glow tied to a paddle. 0x50 bytes, Game+0x2C / +0x30. vtable 803594F0. */
typedef struct HkyPaddleRound {
    RPSimpleModel base;             /* 0x00 */
    s32   playerIndex;              /* 0x44 [M] ctor arg */
    HkyPaddle *owner;               /* 0x48 [H] ctor arg (Game paddle 0/1) */
    void *effect;                   /* 0x4C [M] paddletail_rc/bc_glow effect */
} HkyPaddleRound;

/* Game phases (state objects owned by HkyGame). Base + step/timer. */
typedef struct HkyPhase {
    RPSimpleModel base;             /* 0x00 */
    s32   step;                     /* 0x44 [M] sub-step within the phase */
    s32   timer;                    /* 0x48 [M] frame counter */
} HkyPhase;                         /* GoalPhase/TimeUpPhase/WaitPhase: 0x4C */

typedef struct HkyIntroPhase {
    HkyPhase phase;
    u32   pad4C;                    /* allocated 0x50 */
} HkyIntroPhase;

typedef struct HkyResultPhase {
    HkyPhase phase;
    s32   unk4C;                    /* 0x4C [L] */
    u8    unk50;                    /* 0x50 [L] */
    u8    pad51[3];
} HkyResultPhase;                   /* 0x54 */

/* Laser Hockey manager singleton (lbl_8043C7E0). 0x50 bytes. vtable 80359300. */
typedef struct HkyGame {
    void *vtable;                   /* 0x00 [H] */
    u32   unk04;                    /* 0x04 [L] load */
    u32   unk08;                    /* 0x08 [L] calc, load */
    s32   unk0C;                    /* 0x0C [L] create, stepPhysics */
    s32   timer;                    /* 0x10 [M] counts down; -1 when time is up */
    s32   unk14;                    /* 0x14 [L] flag tested with state */
    s32   state;                    /* 0x18 [H] 0 intro,1 play,2 goal,3 time-up,4 wait,5 result */
    HkyPuck  *puck;                 /* 0x1C [H] */
    HkyField *field;                /* 0x20 [H] */
    HkyPaddle *paddle[2];           /* 0x24 [H] */
    HkyPaddleRound *paddleRound[2]; /* 0x2C [H] */
    HkyIntroPhase  *introPhase;     /* 0x34 [H] state 0 */
    HkyPhase       *goalPhase;      /* 0x38 [H] state 2 */
    HkyPhase       *timeUpPhase;    /* 0x3C [H] state 3 */
    HkyPhase       *waitPhase;      /* 0x40 [H] state 4 */
    HkyResultPhase *resultPhase;    /* 0x44 [H] state 5 */
    void *camera;                   /* 0x48 [H] Gfx_Camera, 0x178 bytes */
    void *view;                     /* 0x4C [M] 0x68-byte screen/view object */
} HkyGame;

#endif /* HKY_H */
