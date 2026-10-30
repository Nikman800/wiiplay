#include <Pack/RPParty/RPPartyCommon/RPTextWriter.h>

#include <Pack/RPKernel/RPSysTagProcessor.h>
#include <Pack/RPKernel/RPSysWideTextWriter.h>

#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPSystem/RPSysFontManager.h>

#include <revolution/GX.h>
#include <revolution/MTX.h>

#include <cstdarg>

extern RPGrpScreen* spCurrentScreen__13RPGrpRenderer;

RPTextWriter::RPTextWriter() : nw4r::ut::WideTextWriter(), mIsInBegin(false) {
    SetBuffer(RPSysWideTextWriter::spTextBuffer, 256);
    SetTagProcessor(RP_GET_INSTANCE(RPSysTagProcessor));
}

RPTextWriter::~RPTextWriter() {}

void RPTextWriter::Begin(s32 fontIdx) {
    RPSysFontManager* pManager = RP_GET_INSTANCE(RPSysFontManager);
    nw4r::ut::Font* pFont;
    if (fontIdx == -1) {
        pFont = pManager->GetLoadedResFont();
    } else {
        pFont = pManager->GetResFont(fontIdx);
    }
    Begin(pFont);
}

#pragma auto_inline off
void RPTextWriter::Begin(const nw4r::ut::Font* pFont) {
    SetFont(*pFont);
    SetupGX();
    GXSetZMode(false, GX_ALWAYS, false);

    Mtx matrix;
    PSMTXIdentity(matrix);
    if (spCurrentScreen__13RPGrpRenderer->GetCanvasMode() ==
        EGG::Frustum::CANVAS_CC) {
        matrix[0][1] = -matrix[0][1];
        matrix[1][1] = -matrix[1][1];
        matrix[2][1] = -matrix[2][1];
    }
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    mIsInBegin = true;
}
#pragma auto_inline reset

void RPTextWriter::End() {
    mIsInBegin = false;
}

void RPTextWriter::Printf(f32 x, f32 y, const wchar_t* pFmt, int arg, ...) {
    std::va_list args;
    va_start(args, arg);

    if (spCurrentScreen__13RPGrpRenderer->GetCanvasMode() ==
        EGG::Frustum::CANVAS_CC) {
        y = -y;
    }
    SetCursor(x, y);

    int written = 0;
    wchar_t* pBuffer = GetBuffer();
    RP_GET_INSTANCE(RPSysTagProcessor)
        ->PreProcessEx(pFmt, pBuffer, 256, &written, arg, args);
    Print(pBuffer, written);

    va_end(args);
}
