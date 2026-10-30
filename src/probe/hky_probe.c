/* Compiler-identification probe: small Laser Hockey functions reconstructed
 * from the original assembly, compiled by tools/cc_probe.py. */
#include "hky.h"

extern u16  lbl_8043C7D0; /* play-area override flag */
extern f32  lbl_8043C7D4; /* override position X */
extern f32  lbl_8043C7D8; /* override position Z */
extern Vec3 lbl_80439170; /* zero vector (RP_sinit_80439170) */

void Hky_Field_resetPuckWallIter(HkyField* field) {
    field->wallIter = &field->walls[15];
}

void Hky_Puck_setInPlay(HkyPuck* puck) {
    puck->inPlay = 1;
}

void Hky_Puck_applyVelocity(HkyPuck* puck) {
    puck->posAccum[0].x = puck->posAccum[0].y = puck->posAccum[0].z = 0.0f;
    puck->posAccum[1].x = puck->posAccum[1].y = puck->posAccum[1].z = 0.0f;
    puck->velAccum[0].x = puck->velAccum[0].y = puck->velAccum[0].z = 0.0f;
    puck->velAccum[1].x = puck->velAccum[1].y = puck->velAccum[1].z = 0.0f;
    puck->spinImpulse[0] = 0.0f;
    puck->spinImpulse[1] = 0.0f;
    puck->base.pos.x += puck->velocity.x;
    puck->base.pos.y += puck->velocity.y;
    puck->base.pos.z += puck->velocity.z;
    if (lbl_8043C7D0 == 0) {
        return;
    }
    puck->base.pos.x = lbl_8043C7D4;
    puck->base.pos.y = 0.0f;
    puck->base.pos.z = lbl_8043C7D8;
    puck->velocity = lbl_80439170;
}

int Hky_Field_getHitWallIndex(HkyField* field) {
    int i;
    for (i = 0; i < 18; i++) {
        if (field->wallIter + 1 == &field->walls[i]) {
            return i;
        }
    }
    return -1;
}

extern f32    lbl_8043B5D4; /* puck speed cap (current) */
extern f32    lbl_8043B5D8; /* puck speed cap (initial) */
extern RPAxes lbl_804391AC; /* identity basis (RP_sinit_80439170) */

void Hky_Puck_resetState(HkyPuck* puck) {
    int i;
    puck->inPlay = 0;
    for (i = 0; i < 4; i++) {
        puck->paddleHitCooldown[i] = 0;
        puck->paddleHitValue[i] = 0.0f;
    }
    for (i = 0; i < 3; i++) {
        puck->hitCooldown[i] = 0;
    }
    puck->posAccum[0].x = puck->posAccum[0].y = puck->posAccum[0].z = 0.0f;
    puck->posAccum[1].x = puck->posAccum[1].y = puck->posAccum[1].z = 0.0f;
    puck->velAccum[0].x = puck->velAccum[0].y = puck->velAccum[0].z = 0.0f;
    puck->velAccum[1].x = puck->velAccum[1].y = puck->velAccum[1].z = 0.0f;
    puck->spinImpulse[0] = 0.0f;
    puck->spinImpulse[1] = 0.0f;
    puck->spin = 0.0f;
    puck->base.axes = lbl_804391AC;
    puck->velocity = lbl_80439170;
    puck->base.pos.x = puck->base.pos.z = 0.0f;
    puck->base.pos.y = 300.0f;
    lbl_8043B5D4 = lbl_8043B5D8;
}
