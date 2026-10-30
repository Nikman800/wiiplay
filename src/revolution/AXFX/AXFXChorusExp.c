#include <revolution/AXFX.h>
#include <revolution/MEM.h>
#include <revolution/OS.h>

static BOOL __InitParams(AXFX_CHORUS_EXP* fx);
static BOOL __AllocDelay(AXFX_CHORUS_EXP* fx);
static BOOL __InitDelay(AXFX_CHORUS_EXP* fx);
static void __FreeDelay(AXFX_CHORUS_EXP* fx);
static void __CalcLFO(u32 param_1[], AXFX_CHORUS_EXP_LFO* fx);

u32 AXFXChorusExpGetMemSize(const AXFX_CHORUS_EXP* fx) {
    return 0x9600;
}

BOOL AXFXChorusExpInit(AXFX_CHORUS_EXP* fx) {
    BOOL enabled;

    enabled = OSDisableInterrupts();
    fx->active |= 1;
    (fx->delay).size = 0xc80;

    if (!__AllocDelay(fx)) {
        AXFXChorusExpShutdown(fx);
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    if (!__InitDelay(fx)) {
        AXFXChorusExpShutdown(fx);
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    if (!__InitParams(fx)) {
        AXFXChorusExpShutdown(fx);
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    fx->active &= ~1;
    OSRestoreInterrupts(enabled);

    return TRUE;
}

BOOL AXFXChorusExpSettings(AXFX_CHORUS_EXP* fx) {
    BOOL enabled;
    BOOL success;

    enabled = OSDisableInterrupts();
    fx->active |= TRUE;
    AXFXChorusExpShutdown(fx);

    success = AXFXChorusExpInit(fx);

    if (success == FALSE) {
        AXFXChorusExpShutdown(fx);
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    fx->active |= 2;
    fx->active &= ~1;
    OSRestoreInterrupts(enabled);

    return success;
}

void AXFXChorusExpShutdown(AXFX_CHORUS_EXP* fx) {
    BOOL enabled;

    enabled = OSDisableInterrupts();

    __FreeDelay(fx);

    OSRestoreInterrupts(enabled);
}

void AXFXChorusExpCallback(AXFX_BUFFERUPDATE* bufferUpdate,
                           AXFX_CHORUS_EXP* fx) {
    s32* lfoPtr;
    u32 samp;
    u32 ch;
    s32 diff;
    s32 pos;
    u32 n;
    u32 frac;
    u32 readPos;
    s32 sizeFP;
    u32 histIdx;
    f32* coef;
    f32 data;
    f32 out;
    s32* input[3];
    s32* inBusData[3];
    s32* outBusData[3];
    s32 lfo[96];

    if (fx->active != 0) {
        fx->active &= ~2;
        return;
    }

    input[0] = bufferUpdate->left;
    input[1] = bufferUpdate->right;
    input[2] = bufferUpdate->surround;

    if (fx->busIn != NULL) {
        inBusData[0] = fx->busIn->left;
        inBusData[1] = fx->busIn->right;
        inBusData[2] = fx->busIn->surround;
    }

    if (fx->busOut != NULL) {
        outBusData[0] = fx->busOut->left;
        outBusData[1] = fx->busOut->right;
        outBusData[2] = fx->busOut->surround;
    }

    __CalcLFO((u32*)lfo, &fx->lfo);

    lfoPtr = lfo;
    for (samp = 0; samp < 96; samp++) {
        sizeFP = fx->delay.sizeFP;
        pos = fx->delay.outPos + *lfoPtr;

        if (pos >= sizeFP) {
            pos -= sizeFP;
        } else if (pos < 0) {
            pos += sizeFP;
        }

        diff = pos - fx->delay.lastPos;
        if (diff < 0) {
            diff += sizeFP;
        }

        n = (u32)diff >> 16;
        frac = diff & 0xFFFF;
        readPos = fx->delay.lastPos >> 16;
        histIdx = fx->histIndex;

        while (n != 0) {
            fx->history[0][histIdx] = fx->delay.line[0][readPos];
            fx->history[1][histIdx] = fx->delay.line[1][readPos];
            fx->history[2][histIdx] = fx->delay.line[2][readPos];
            histIdx++;
            histIdx &= 3;

            readPos++;
            n--;
            if (readPos >= fx->delay.size) {
                readPos = 0;
            }
        }

        fx->delay.lastPos = pos & 0xFFFF0000;
        coef = __AXFXGetSrcCoef((frac >> 9) & 0x7F);

        for (ch = 0; ch < 3; ch++) {
            out = 0.0f;
            out += coef[0] * fx->history[ch][histIdx];
            histIdx++;
            histIdx &= 3;
            out += coef[1] * fx->history[ch][histIdx];
            histIdx++;
            histIdx &= 3;
            out += coef[2] * fx->history[ch][histIdx];
            histIdx++;
            histIdx &= 3;
            out += coef[3] * fx->history[ch][histIdx];
            histIdx++;
            histIdx &= 3;

            if (fx->busIn != NULL) {
                data = (f32)(*input[ch] + *(inBusData[ch]++));
            } else {
                data = (f32)(*input[ch]);
            }

            fx->delay.line[ch][fx->delay.inPos] = data + out * fx->feedback;
            *(input[ch]++) = (s32)(out * fx->outGain);

            if (fx->busOut != NULL) {
                *(outBusData[ch]++) = (s32)(out * fx->sendGain);
            }
        }

        fx->histIndex = histIdx;

        fx->delay.inPos++;
        if (fx->delay.inPos >= fx->delay.size) {
            fx->delay.inPos = 0;
        }

        fx->delay.outPos += 0x10000;
        if (fx->delay.outPos >= fx->delay.sizeFP) {
            fx->delay.outPos = 0;
        }

        lfoPtr++;
    }
}

static BOOL __AllocDelay(AXFX_CHORUS_EXP* fx) {
    u32 i;

    for (i = 0; i < 3; i++) {
        fx->delay.line[i] = __AXFXAlloc(fx->delay.size * sizeof(f32));
        if (fx->delay.line[i] == NULL) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL __InitDelay(AXFX_CHORUS_EXP* fx) {
    f32** line = fx->delay.line;
    u32 i;
    u32 uVar2;

    for (i = 0; i < 3; i++, line++) {
        if (*line == NULL) {
            return FALSE;
        }

        memset(*line, 0, (fx->delay).size * sizeof(f32));
    }

    (fx->delay).inPos = 0;
    uVar2 = (u32)(fx->delayTime * 32.0f);
    uVar2 = ((fx->delay).size - uVar2) * 0x10000;
    (fx->delay).sizeFP = (fx->delay).size << 0x10;
    (fx->delay).outPos = uVar2;
    (fx->delay).lastPos = uVar2;

    return TRUE;
}

static void __FreeDelay(AXFX_CHORUS_EXP* fx) {
    u32 i;

    fx->active |= TRUE;
    for (i = 0; i < 3; i++) {
        if (fx->delay.line[i] != NULL) {
            __AXFXFree(fx->delay.line[i]);
        }
        fx->delay.line[i] = NULL;
    }
}

static BOOL __InitParams(AXFX_CHORUS_EXP* fx) {
    s32 j;
    s32 i;
    s32* table;

    s32 phaseAdd;
    s32 tempSamp;
    f32 gradFactor;
    f32 depthSamp;
    f32 tempVal;
    f32 stepSamp;

    if ((fx->delayTime < 0.1f) || (fx->delayTime > 50.0f)) {
        return FALSE;
    }
    if ((fx->depth < 0.0f) || (fx->depth > 1.0f)) {
        return FALSE;
    }
    if ((fx->rate < 0.1f) || (fx->rate > 2.0f)) {
        return FALSE;
    }
    if ((fx->feedback < 0.0f) || (fx->feedback >= 1.0f)) {
        return FALSE;
    }
    if ((fx->outGain < 0.0f) || (fx->outGain > 1.0f)) {
        return FALSE;
    }
    if ((fx->sendGain < 0.0f) || (fx->sendGain > 1.0f)) {
        return FALSE;
    }
    table = __AXFXGetLfoSinTable();
    fx->lfo.table = table;

    depthSamp = 32.0f * fx->delayTime * fx->depth;
    if (depthSamp >= 32.0f * fx->delayTime) {
        depthSamp -= 1.0f;
        if (depthSamp < 0.0f) {
            depthSamp = 0.0f;
        }
    }

    fx->lfo.lastNum = -1;
    fx->lfo.phase = 0;
    fx->lfo.sign = 0;

    tempSamp = (s32)(65536.0f * depthSamp);
    fx->lfo.depthSamp = tempSamp;

    fx->lfo.grad = fx->lfo.lastValue = 0;

    phaseAdd = (s32)(65536.0f * (((tempVal = 256.0f) * fx->rate) / 32000.0f));
    fx->lfo.phaseAdd = phaseAdd;

    stepSamp = (32000.0f / fx->rate) / 256.0f;
    gradFactor = depthSamp / stepSamp;

    tempSamp = (s32)(65536.0f * stepSamp);
    fx->lfo.stepSamp = tempSamp;

    fx->lfo.gradFactor = (s32)(65536.0f * gradFactor);

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            fx->history[i][j] = 0.0f;
        }
    }

    fx->histIndex = 0;

    return TRUE;
}

void __CalcLFO(u32* out, AXFX_CHORUS_EXP_LFO* lfo) {
    u32 samp;
    u32 lastNum;
    s32 tableVal;
    s64 value;
    u64 diff;
    s32 wtf;

    for (samp = 0x60; samp != 0; samp--) {
        lastNum = lfo->phase & 0xFFFF0000;
        if (lastNum != lfo->lastNum) {
            lfo->lastNum = lastNum;
            lastNum = lastNum >> 16;
            tableVal = lfo->table[lastNum];
            diff = lfo->table[(lastNum + (wtf = 1)) & 0x7F] - tableVal;
            lfo->grad = (s32)((((s64)diff) * (lfo->gradFactor)) >> 24);
            value = (((s64)tableVal) * lfo->depthSamp) >> 24;
        } else {
            value = (s64)(lfo->lastValue + lfo->grad);
        }

        lfo->lastValue = (s32)value;
        if (lfo->sign >= 1) {
            value = -value;
        }

        lfo->phase += lfo->phaseAdd;
        if ((lfo->phase & 0xFF800000) != 0) {
            lfo->phase &= 0x7FFFFF;
            lfo->sign ^= wtf;
        }
        *(out++) = (u32)value;
    }
}
