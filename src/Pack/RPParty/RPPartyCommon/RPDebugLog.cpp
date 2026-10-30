// Debug log and sample graphs drawn over the scene (RP_drawDebugLogAndGraphs and helpers).
// Split from the RPBBox.cpp ledger range: this file owns .text 801C30BC-801C45B0 and its own
// .sdata2 pool (8043EC10-8043EC78; 0.0f appears again there, so it is a separate file).
// The struct-wrapped floats keep the named constants first in the pool without being folded.
#include <Pack/RPParty/RPPartyCommon/RPDebugGraph.h>
#include <Pack/RPParty/RPPartyCommon/RPTextWriter.h>
#include <egg/core/eggController.h>
#include <egg/gfxe/eggDrawGX.h>
#include <egg/gfxe/eggScreen.h>

#include <cstdarg>
#include <cwchar>

namespace {
struct DebugLine {
    wchar_t text[128];
};

template <typename T, int N> struct DebugBuffer {
    T samples[N];
    int start;
    int head;
    bool full;

    DebugBuffer() : start(0), head(0), full(false) {}

    void Clear() {
        start = 0;
        head = 0;
        full = false;
    }

    void Push(T value) {
        samples[head] = value;
        head = head + 1 < N ? head + 1 : 0;
        full = full | (head == start);
    }

    int WrapCount(int count) const {
        return count < N ? count : count - N;
    }

    int Count() const {
        if (!full) {
            return WrapCount(head - start);
        }
        return N;
    }

    const T& Recent(s32 index) const {
        return samples[head - 1 - index >= 0 ? head - 1 - index : head + N - 1 - index];
    }

    T Sample(const s32& index) {
        s32 pos = head - 1 - index;
        s32 wrapped = head + N - 1 - index;
        return samples[pos >= 0 ? pos : wrapped];
    }
};

// Accessor inline: graph A reads need its by-value index (indexing RP_debugGraphsA directly
// changes the register order in RP_drawDebugLogAndGraphs).
inline RPDebugGraph& Graph(RPDebugGraph* graphs, int index) {
    return graphs[index];
}

struct DebugF32 {
    f32 value;
};
const GXColor debugWhite = {255, 255, 255, 64};
const DebugF32 sGraphScale = {50.0f};
const DebugF32 sBarScale = {50.0f};
const DebugF32 sFontSize = {16.0f};
inline f32 BarSample(RPDebugGraph* graphs, int channel, int index) {
    return sBarScale.value * Graph(graphs, channel).Sample(index);
}
inline f32 GraphX(RPDebugGraph& graph, const int& index, u16 half) {
    return 2.0f * sGraphScale.value + sGraphScale.value * graph.Recent(index) - half;
}
}


DebugBuffer<DebugLine, 32>* RP_debugLog;
RPDebugGraph* RP_debugGraphsA;
RPDebugGraph* RP_debugGraphsB;
RPDebugGraph* RP_debugGraphsC;
s32 RP_debugGraphScroll;
f32 RP_debugGraphZoom = 2.5f;

extern "C" void RP_allocDebugBuffers() {
    RP_debugLog = new DebugBuffer<DebugLine, 32>;
    RP_debugGraphsA = new RPDebugGraph[6];
    RP_debugGraphsB = new RPDebugGraph[3];
    RP_debugGraphsC = new RPDebugGraph[3];
}

extern "C" void RP_freeDebugBuffers() {
    operator delete(RP_debugLog);
    operator delete(RP_debugGraphsA);
    operator delete(RP_debugGraphsB);
    operator delete(RP_debugGraphsC);
}

extern "C" void RP_clearDebugBuffers() {
    RP_debugLog->Clear();
    RP_debugGraphsA->Clear();
    RP_debugGraphsB->Clear();
    RP_debugGraphsC->Clear();
}

extern "C" void RP_debugLogPrintf(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    DebugLine line;
    vswprintf(line.text, 128, format, args);
    va_end(args);
    RP_debugLog->Push(line);
}

extern "C" void RP_pushDebugGraphSamples(f32 x, f32 y, f32 z) {
    RP_debugGraphsB[0].Push(x);
    RP_debugGraphsB[1].Push(y);
    RP_debugGraphsB[2].Push(z);
}

// Graph A's x helper binds the index by const reference: i - 1 is then created when the argument
// is bound, before the width load, so the lhzx is coloured first (r6, i - 1 r7). With a by-value
// sample the index was propagated into Recent()'s body and created after the width.
// Matched by mwcc_trace: the log count must be coloured before graph B's i - 1 (r31 vs r30), B's
// i - 1 after the sTVModeWidths address and before the 0x96c offset and colour pointer, and the
// colour pointer created after the offset (it is then incremented first). So the colour reference
// is taken after the samples in B and C, B reads through BarSample (its by-value index is an inline
// temporary numbered after the log's), Sample binds the index by const reference, and graph A's
// points read Recent() through GraphX.
extern "C" void RP_drawDebugLogAndGraphs() {
    using EGG::CoreControllerMgr;
    using EGG::Screen;
    using EGG::DrawGX;
    using nw4r::math::VEC3;

    if (CoreControllerMgr::instance()->getNthController(0)->down(1)) {
        s32 scroll = RP_debugGraphScroll + 1;
        RP_debugGraphScroll = scroll < 600 ? scroll : 600;
    }
    if (CoreControllerMgr::instance()->getNthController(0)->down(2)) {
        s32 scroll = RP_debugGraphScroll - 1;
        RP_debugGraphScroll = scroll >= 0 ? scroll : 0;
    }
    if (CoreControllerMgr::instance()->getNthController(0)->down(8)) {
        f32 zoom = 0.1f + RP_debugGraphZoom;
        RP_debugGraphZoom = zoom < 20.0f ? zoom : 20.0f;
    }
    if (CoreControllerMgr::instance()->getNthController(0)->down(4)) {
        f32 zoom = RP_debugGraphZoom - 0.1f;
        RP_debugGraphZoom = zoom >= 0.5f ? zoom : 0.5f;
    }

    if (RP_debugLog->Count() > 0) {
        int count = RP_debugLog->Count();
        ::RPTextWriter writer;
        writer.Begin(-1);
        writer.SetDrawFlag(0);
        writer.SetFontSize(sFontSize.value);
        writer.SetTextColor(nw4r::ut::Color(debugWhite));
        f32 x = -static_cast<int>(static_cast<u16>(Screen::GetSizeXMax() / 2)) + sFontSize.value;
        f32 y = static_cast<u16>(Screen::GetSizeYMax() / 2) - sFontSize.value / 2.0f;
        for (int i = 0; i < count; ++i) {
            writer.Printf(x, y - sFontSize.value * i, RP_debugLog->Recent(i).text, 0);
        }
        writer.End();
    }

    for (s32 channel = 0; channel < 6; channel += 2) {
        const GXColor white = debugWhite;
        if (Graph(RP_debugGraphsA, channel).Count() > 0 &&
            Graph(RP_debugGraphsA, channel + 1).Count() > 0) {
            VEC3 points[2];
            points[0].z = points[1].z = 0.0f;
            int count = RP_debugGraphScroll + Screen::GetSizeXMax() / RP_debugGraphZoom;
            for (int j = 0; j < 2; ++j) {
                RPDebugGraph& graph = Graph(RP_debugGraphsA, channel + j);
                if (count > graph.Count())
                    count = graph.Count();
            }

            DrawGX::BeginDrawLine(DrawGX::COLORCHAN_COLOR, DrawGX::ZMODE_ALWAYS);
            for (int i = 1; i <= 2; ++i) {
                DrawGX::DrawCircle2D(DrawGX::CIRCLE_16, 2.0f * sGraphScale.value - static_cast<u16>(Screen::GetSizeXMax() / 2),
                                  0.0f, sGraphScale.value * i, white, 10);
            }
            DrawGX::BeginDrawLine(DrawGX::COLORCHAN_COLOR, DrawGX::ZMODE_ALWAYS);
            for (int i = RP_debugGraphScroll + 1; i < count; ++i) {
                GXColor color = channel == 0 ?
                    (GXColor){255, 0, 0, static_cast<u8>(255 - (i - RP_debugGraphScroll) * 255 / (count - RP_debugGraphScroll))} :
                    channel == 1 ?
                    (GXColor){0, 255, 0, static_cast<u8>(255 - (i - RP_debugGraphScroll) * 255 / (count - RP_debugGraphScroll))} :
                    (GXColor){0, 0, 255, static_cast<u8>(255 - (i - RP_debugGraphScroll) * 255 / (count - RP_debugGraphScroll))};
                points[0].x = GraphX(Graph(RP_debugGraphsA, channel), i - 1, Screen::GetSizeXMax() / 2);
                points[0].y = sGraphScale.value * Graph(RP_debugGraphsA, channel + 1).Recent(i - 1);
                points[1].x = GraphX(Graph(RP_debugGraphsA, channel), i, Screen::GetSizeXMax() / 2);
                points[1].y = sGraphScale.value * Graph(RP_debugGraphsA, channel + 1).Recent(i);
                DrawGX::DrawLine(points, 2, color, 10);
            }
            u16 half = Screen::GetSizeXMax() / 2;
            points[0].x = 2.0f * sGraphScale.value + sGraphScale.value * Graph(RP_debugGraphsA, channel).Recent(0) - half;
            points[0].y = sGraphScale.value * Graph(RP_debugGraphsA, channel + 1).Recent(0);
            GXColor color = channel == 0 ? (GXColor){255, 0, 0, 255} :
                            channel == 1 ? (GXColor){0, 255, 0, 255} :
                                           (GXColor){0, 0, 255, 255};
            DrawGX::BeginDrawLine(DrawGX::COLORCHAN_COLOR, DrawGX::ZMODE_ALWAYS);
            DrawGX::DrawPoint(points, 1, color, 30);
        }
    }

    if ((Graph(RP_debugGraphsB, 0).Count() > 0 && Graph(RP_debugGraphsB, 1).Count() > 0 && Graph(RP_debugGraphsB, 2).Count() > 0) ||
        (Graph(RP_debugGraphsC, 0).Count() > 0 && Graph(RP_debugGraphsC, 1).Count() > 0 && Graph(RP_debugGraphsC, 2).Count() > 0)) {
        VEC3 points[2];
        points[0].z = points[1].z = 0.0f;
        DrawGX::BeginDrawLine(DrawGX::COLORCHAN_COLOR, DrawGX::ZMODE_ALWAYS);
        int halfWidth = static_cast<u16>(Screen::GetSizeXMax() / 2);
        points[0].x = -halfWidth;
        points[1].x = halfWidth;
        for (int i = -2; i <= 2; ++i) {
            const GXColor white = debugWhite;
            points[0].y = points[1].y = sBarScale.value * i;
            DrawGX::DrawLine(points, 2, white, 10);
        }
        points[0].y = -RP_debugGraphZoom;
        points[1].y = RP_debugGraphZoom;
        points[0].x = points[1].x = static_cast<u16>(Screen::GetSizeXMax() / 2);
        while (points[0].x >= -static_cast<int>(static_cast<u16>(Screen::GetSizeXMax() / 2))) {
            const GXColor white = debugWhite;
            DrawGX::DrawLine(points, 2, white, 10);
            points[0].x -= 30.0f * RP_debugGraphZoom;
            points[1].x -= 30.0f * RP_debugGraphZoom;
        }
    }

    if (Graph(RP_debugGraphsB, 0).Count() > 0 && Graph(RP_debugGraphsB, 1).Count() > 0 && Graph(RP_debugGraphsB, 2).Count() > 0) {
        int count = RP_debugGraphScroll + Screen::GetSizeXMax() / RP_debugGraphZoom;
        RPDebugGraph* graph = RP_debugGraphsB;
        for (int i = 3; i != 0; --i, ++graph) {
            if (count > graph->Count()) count = graph->Count();
        }
        VEC3 points[2];
        points[0].z = points[1].z = 0.0f;
        for (int i = RP_debugGraphScroll + 1; i < count; ++i) {
            u8 alpha = 255 - (i - RP_debugGraphScroll) * 255 / (count - RP_debugGraphScroll);
            GXColor colors[3] = {(GXColor){255, 0, 0, alpha},
                                 (GXColor){0, 255, 0, alpha},
                                 (GXColor){0, 0, 255, alpha}};
            points[0].x = -RP_debugGraphZoom * (i - RP_debugGraphScroll - 1) + static_cast<u16>(Screen::GetSizeXMax() / 2);
            points[1].x = -RP_debugGraphZoom * (i - RP_debugGraphScroll) + static_cast<u16>(Screen::GetSizeXMax() / 2);
            for (int channel = 0; channel < 3; ++channel) {
                points[0].y = BarSample(RP_debugGraphsB, channel, i - 1);
                points[1].y = BarSample(RP_debugGraphsB, channel, i);
                const GXColor& color = colors[channel];
                DrawGX::DrawLine(points, 2, color, 10);
            }
        }
    }
    if (Graph(RP_debugGraphsC, 0).Count() > 0 && Graph(RP_debugGraphsC, 1).Count() > 0 && Graph(RP_debugGraphsC, 2).Count() > 0) {
        int count = RP_debugGraphScroll + Screen::GetSizeXMax() / RP_debugGraphZoom;
        RPDebugGraph* graph = RP_debugGraphsC;
        for (int i = 3; i != 0; --i, ++graph) {
            if (count > graph->Count()) count = graph->Count();
        }
        VEC3 points[2];
        points[0].z = points[1].z = 0.0f;
        for (int i = RP_debugGraphScroll + 1; i < count; ++i) {
            u8 alpha = 255 - (i - RP_debugGraphScroll) * 255 / (count - RP_debugGraphScroll);
            GXColor colors[3] = {(GXColor){255, 0, 0, alpha},
                                 (GXColor){0, 255, 0, alpha},
                                 (GXColor){0, 0, 255, alpha}};
            points[0].x = points[1].x = -RP_debugGraphZoom * (i - RP_debugGraphScroll) + static_cast<u16>(Screen::GetSizeXMax() / 2);
            for (int channel = 0; channel < 3; ++channel) {
                points[0].y = 0.0f;
                points[1].y = BarSample(RP_debugGraphsC, channel, i);
                const GXColor& color = colors[channel];
                DrawGX::DrawLine(points, 2, color, 30);
            }
        }
    }
}
