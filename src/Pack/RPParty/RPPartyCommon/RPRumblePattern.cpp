#include <Pack/RPParty/RPPartyCommon/RPRumblePattern.h>

RPRumblePattern RPRumblePattern::sSlots[4][4];

RPRumblePattern::RPRumblePattern() : mLength(0), mRemaining(0), mpSelf(this) {}

RPRumblePattern::RPRumblePattern(s32 length, f32 strength, s32 attack) {
    mLength = length;
    mRemaining = length;
    mpSelf = this;
    Build(length, strength, attack);
}

void RPRumblePattern::Build(s32 length, f32 strength, s32 attack) {
    f32 step = strength / (attack + 1);
    f32 acc = 0.0f;
    f32 level = 0.0f;

    mLength = length;

    for (s32 i = 0; i < length / 32 + 1; i++) {
        mBits[i] = 0;
        for (s32 j = 0; j < 32; j++) {
            if (length - 1 - (j + i * 32) >= 0) {
                if (level < strength) {
                    level += step;
                    level = level < strength ? level : strength;
                }
                acc += level;
                if (acc >= 1.0f) {
                    mBits[i] += 1 << j;
                    acc -= 1.0f;
                }
            }
        }
    }
}

// Stand-ins (unreferenced, dead-stripped at link): compiled before Build (deferred codegen runs
// last-defined first). The original .sdata2 is 0.0f (lbl_8043EC78), the s32->f32 magic double
// (8043EC80), 1.0f (8043EC88).
// STAND-IN: dead-stripped; puts the s32->f32 magic double (8043EC80) ahead of 1.0f (8043EC88).
// Without it the double comes after 1.0f.
extern "C" f32 RP_rumblePoolOrderS32(s32 x) {
    return x;
}

// STAND-IN: dead-stripped; puts 0.0f (8043EC78) first in .sdata2. Without it 0.0f comes last,
// after the magic double and 1.0f.
extern "C" f32 RP_rumblePoolOrderZero() {
    return 0.0f;
}
