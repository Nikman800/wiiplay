#include <egg/math.h>

#include <revolution/GX.h>

namespace EGG {

class Color : public GXColor {
public:
    Color(u32 color) {
        *reinterpret_cast<u32*>(this) = color;
    }
    Color(u8 red, u8 green, u8 blue, u8 alpha) {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }
    ~Color() {}

    u32 toU32() const {
        return *reinterpret_cast<const u32*>(this);
    }
};

class Graphics {
public:
    static void drawPoint(const Vector3f& rPos);
    static void drawPoint2D(const Vector2f& rPos, f32 z);
    static void drawLine(const Vector3f& rStart, const Vector3f& rEnd);
    static void drawAxisLine(const Vector3f& rPos, const Vector3f& rDir,
                             const Vector3f& rUp);
    static void setColor(u8 r, u8 g, u8 b, u8 a);
    static Vector3f* createVertexArray(int num);
    static void drawLineToMidpoint(const Vector3f& rStart,
                                   const Vector3f& rEnd);
    static Matrix34f* createMatrixArray(int num);

    static Color mColor;
};

Color Graphics::mColor(0xFFFFFFFF);

void Graphics::drawPoint(const Vector3f& rPos) {
    GXBegin(GX_POINTS, GX_VTXFMT0, 1);
    GXPosition3f32(rPos.x, rPos.y, rPos.z);
    GXColor1u32(mColor.toU32());
    GXEnd();
}

void Graphics::drawLine(const Vector3f& rStart, const Vector3f& rEnd) {
    GXBegin(GX_LINES, GX_VTXFMT0, 2);
    GXPosition3f32(rStart.x, rStart.y, rStart.z);
    GXColor1u32(mColor.toU32());
    GXPosition3f32(rEnd.x, rEnd.y, rEnd.z);
    GXColor1u32(mColor.toU32());
    GXEnd();
}

void Graphics::drawPoint2D(const Vector2f& rPos, f32 z) {
    Vector2f pos = rPos;
    Vector3f v(pos.x, pos.y, z);
    drawPoint(v);
}

void Graphics::drawAxisLine(const Vector3f& rPos, const Vector3f& rDir,
                            const Vector3f& rUp) {
    Vector3f side = rDir.cross(rUp);
    drawLine(rPos, side);
}

void Graphics::setColor(u8 r, u8 g, u8 b, u8 a) {
    Color color(r, g, b, a);
    mColor = color;
}

Vector3f* Graphics::createVertexArray(int num) {
    return new Vector3f[num];
}

void Graphics::drawLineToMidpoint(const Vector3f& rStart,
                                  const Vector3f& rEnd) {
    drawLine(rStart, (rStart + rEnd) * 0.5f);
}

Matrix34f* Graphics::createMatrixArray(int num) {
    return new Matrix34f[num];
}

} // namespace EGG
