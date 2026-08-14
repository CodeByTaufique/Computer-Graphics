// ============================================================================
//  PIXEL PANIC: SAVING REALITY.exe
//  A single-file OpenGL (GLFW) open-world graphics-comedy game.
//
//  Author:   (your name here)
//  Course:   Computer Graphics
//  Engine:   OpenGL (legacy/immediate-mode, C++17)
//
//  --------------------------------------------------------------------------
//  BUILD INSTRUCTIONS
//  --------------------------------------------------------------------------
//  Dependencies: GLFW3, OpenGL (no GLU / GLEW / GLAD required)
//
//  Windows (MSYS2 / MinGW):
//      g++ PixelPanic.cpp -o PixelPanic.exe -lglfw3 -lopengl32 -lgdi32 -std=c++17
//
//  Linux (Ubuntu/Debian):
//      sudo apt install libglfw3-dev libgl1-mesa-dev
//      g++ PixelPanic.cpp -o PixelPanic -lglfw -lGL -ldl -lpthread -std=c++17
//
//  macOS (Homebrew):
//      brew install glfw
//      g++ PixelPanic.cpp -o PixelPanic -lglfw -framework OpenGL -std=c++17
//
//  --------------------------------------------------------------------------
//  DESIGN OVERVIEW
//  --------------------------------------------------------------------------
//  Pixel City is one continuous 2D world (top-down) split into nine themed
//  districts. There are no discrete "levels" - the player (Patch) walks
//  freely and the Graphics Engine spontaneously misbehaves nearby, spawning
//  "Glitches" that must be repaired using classic computer-graphics
//  algorithms. Each glitch type maps to one graphics technique and, unlike
//  a simple "hold E to fill a bar" interaction, each one runs its own small
//  interactive minigame so the algorithm is genuinely demonstrated:
//
//      Glitch Type          Algorithm Demonstrated        Minigame
//      --------------------  ---------------------------  ------------------
//      StrayPixel             Point plotting                Click/aim capture
//      BrokenRoad              DDA & Bresenham line           Rebuild by steps
//      WobblyWheel               Midpoint Circle algorithm      Radius match
//      DriftedBuilding            2D Transform (translate/rot)   Nudge to slot
//      NoodleTower                  Shearing                       Un-shear drag
//      MirrorTwin                     Reflection                     Mirror align
//      TangledRiver                     Bezier curve fitting            Control point
//      LeakingObject                      Clipping (Cohen-Sutherland)      Wall fit
//      ColorSickness                        RGB color balancing              Slider mix
//
//  A cast of comedic NPCs roam the districts with their own self-contained
//  behaviors (Captain Circle's shape cycle, Polygon Dog's polygon count,
//  RGB Wizard's color spells, Debug Pigeon's mischief, Bezier Grandma's
//  curve-knitting, Professor Pixel's incorrect narration). Random "Dynamic
//  Events" (Coffee Overflow, RGB Festival, Polygon Flu, Inverted Gravity,
//  Giant Cursor Attack, Windows Update, Low FPS Storm) occasionally wash
//  over the whole city and temporarily change simulation rules.
//
//  --------------------------------------------------------------------------
//  FILE MAP  (search these section banners to navigate)
//  --------------------------------------------------------------------------
//    SECTION  1  Includes & global constants
//    SECTION  2  Math / geometry helpers (Vec2, Color, lerp, easing, RNG)
//    SECTION  3  Bitmap font & on-screen text rendering
//    SECTION  4  Drawing primitives built on classic graphics algorithms
//    SECTION  5  Particle system
//    SECTION  6  World data: districts, props, camera, minimap
//    SECTION  7  Player entity
//    SECTION  8  NPC entities & behavior
//    SECTION  9  Dialogue system
//    SECTION 10  Glitch base system + per-type minigame logic
//    SECTION 11  Dynamic world events
//    SECTION 12  HUD & UI screens (title, pause, help, stats)
//    SECTION 13  Save/stat tracking
//    SECTION 14  Game state aggregation & update loop
//    SECTION 15  Top-level rendering
//    SECTION 16  Input handling & GLFW callbacks
//    SECTION 17  main()
// ============================================================================

#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================================
// SECTION 1 — GLOBAL CONSTANTS
// ============================================================================

namespace Config {
constexpr int   kWindowWidth      = 1280;
constexpr int   kWindowHeight     = 800;
constexpr float kWorldWidth       = 3600.0f;   // total scrollable world size
constexpr float kWorldHeight      = 2400.0f;
constexpr float kPlayerSpeed      = 220.0f;    // pixels / second
constexpr float kPlayerRunMult    = 1.6f;      // shift-to-run multiplier
constexpr float kCameraEase       = 4.0f;      // camera follow smoothing
constexpr float kGlitchSpawnEvery = 4.5f;      // seconds between spawns
constexpr float kEventSpawnEvery  = 25.0f;     // seconds between world events
constexpr int   kMaxGlitches      = 6;
constexpr float kInteractRadius   = 70.0f;
constexpr float kMinimapSize      = 170.0f;
constexpr float kMinimapMargin    = 16.0f;
constexpr int   kTargetGlitchesForWin = 40;    // "soft win" milestone (endless after)
}  // namespace Config

// ============================================================================
// SECTION 2 — MATH / GEOMETRY HELPERS
// ============================================================================

struct Vec2 {
    float x = 0.0f, y = 0.0f;
    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSq() const { return x * x + y * y; }
    Vec2 normalized() const {
        float l = length();
        return (l > 0.0001f) ? Vec2{x / l, y / l} : Vec2{0, 0};
    }
    float dot(const Vec2& o) const { return x * o.x + y * o.y; }
};

struct Color {
    float r = 1, g = 1, b = 1, a = 1;
    Color() = default;
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
    Color withAlpha(float alpha) const { return {r, g, b, alpha}; }
};

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
inline Vec2  lerpVec(Vec2 a, Vec2 b, float t) { return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)}; }
inline Color lerpColor(Color a, Color b, float t) {
    return {lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t), lerp(a.a, b.a, t)};
}
inline float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }
inline float randf(float lo, float hi) { return lo + (hi - lo) * (std::rand() / (float)RAND_MAX); }
inline int   randi(int lo, int hi) { return lo + std::rand() % (hi - lo + 1); }
inline float easeOutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
inline float easeInOutSine(float t) { return -(std::cos((float)M_PI * t) - 1.0f) / 2.0f; }
inline float pingPong(float t, float length) {
    float m = std::fmod(t, length * 2.0f);
    return m < length ? m : length * 2.0f - m;
}
inline float degToRad(float d) { return d * (float)M_PI / 180.0f; }

// Simple deterministic-ish string hash, used to give NPCs/props stable
// per-instance jitter without storing extra random seeds.
inline unsigned int hashString(const std::string& s) {
    unsigned int h = 2166136261u;
    for (char c : s) { h ^= (unsigned char)c; h *= 16777619u; }
    return h;
}

// ============================================================================
// SECTION 3 — BITMAP FONT & ON-SCREEN TEXT RENDERING
// ----------------------------------------------------------------------------
//  The game needs on-screen dialogue, HUD labels, and menu text, but pulling
//  in a font-loading library (FreeType, stb_truetype) would break the
//  "single dependency-free file" goal. Instead we define a compact 5x7
//  pixel font for the ASCII characters we need and rasterize it with
//  GL_POINTS / GL_QUADS at draw time. This is itself a nice demonstration
//  of "point plotting" applied to typography, which fits the game's theme.
// ============================================================================

namespace Font {

// Each glyph is 5 columns x 7 rows, stored as 7 bytes where the low 5 bits
// of each byte are the pixels of that row (bit 4 = leftmost column).
using Glyph = std::array<unsigned char, 7>;

inline const std::unordered_map<char, Glyph>& glyphTable() {
    static const std::unordered_map<char, Glyph> table = {
        {'A', {0b01110,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001}},
        {'B', {0b11110,0b10001,0b10001,0b11110,0b10001,0b10001,0b11110}},
        {'C', {0b01111,0b10000,0b10000,0b10000,0b10000,0b10000,0b01111}},
        {'D', {0b11100,0b10010,0b10001,0b10001,0b10001,0b10010,0b11100}},
        {'E', {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b11111}},
        {'F', {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b10000}},
        {'G', {0b01111,0b10000,0b10000,0b10011,0b10001,0b10001,0b01111}},
        {'H', {0b10001,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001}},
        {'I', {0b01110,0b00100,0b00100,0b00100,0b00100,0b00100,0b01110}},
        {'J', {0b00111,0b00010,0b00010,0b00010,0b00010,0b10010,0b01100}},
        {'K', {0b10001,0b10010,0b10100,0b11000,0b10100,0b10010,0b10001}},
        {'L', {0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b11111}},
        {'M', {0b10001,0b11011,0b10101,0b10101,0b10001,0b10001,0b10001}},
        {'N', {0b10001,0b10001,0b11001,0b10101,0b10011,0b10001,0b10001}},
        {'O', {0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110}},
        {'P', {0b11110,0b10001,0b10001,0b11110,0b10000,0b10000,0b10000}},
        {'Q', {0b01110,0b10001,0b10001,0b10001,0b10101,0b10010,0b01101}},
        {'R', {0b11110,0b10001,0b10001,0b11110,0b10100,0b10010,0b10001}},
        {'S', {0b01111,0b10000,0b10000,0b01110,0b00001,0b00001,0b11110}},
        {'T', {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b00100}},
        {'U', {0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110}},
        {'V', {0b10001,0b10001,0b10001,0b10001,0b10001,0b01010,0b00100}},
        {'W', {0b10001,0b10001,0b10001,0b10101,0b10101,0b10101,0b01010}},
        {'X', {0b10001,0b10001,0b01010,0b00100,0b01010,0b10001,0b10001}},
        {'Y', {0b10001,0b10001,0b01010,0b00100,0b00100,0b00100,0b00100}},
        {'Z', {0b11111,0b00001,0b00010,0b00100,0b01000,0b10000,0b11111}},
        {'0', {0b01110,0b10011,0b10101,0b10101,0b10101,0b11001,0b01110}},
        {'1', {0b00100,0b01100,0b00100,0b00100,0b00100,0b00100,0b01110}},
        {'2', {0b01110,0b10001,0b00001,0b00010,0b00100,0b01000,0b11111}},
        {'3', {0b11110,0b00001,0b00001,0b01110,0b00001,0b00001,0b11110}},
        {'4', {0b00010,0b00110,0b01010,0b10010,0b11111,0b00010,0b00010}},
        {'5', {0b11111,0b10000,0b11110,0b00001,0b00001,0b10001,0b01110}},
        {'6', {0b00110,0b01000,0b10000,0b11110,0b10001,0b10001,0b01110}},
        {'7', {0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000}},
        {'8', {0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110}},
        {'9', {0b01110,0b10001,0b10001,0b01111,0b00001,0b00010,0b01100}},
        {'!', {0b00100,0b00100,0b00100,0b00100,0b00100,0b00000,0b00100}},
        {'?', {0b01110,0b10001,0b00001,0b00110,0b00100,0b00000,0b00100}},
        {'.', {0b00000,0b00000,0b00000,0b00000,0b00000,0b01100,0b01100}},
        {',', {0b00000,0b00000,0b00000,0b00000,0b01100,0b01100,0b01000}},
        {':', {0b00000,0b01100,0b01100,0b00000,0b01100,0b01100,0b00000}},
        {'-', {0b00000,0b00000,0b00000,0b11111,0b00000,0b00000,0b00000}},
        {'+', {0b00000,0b00100,0b00100,0b11111,0b00100,0b00100,0b00000}},
        {'\'',{0b01100,0b01100,0b00100,0b00000,0b00000,0b00000,0b00000}},
        {'(', {0b00010,0b00100,0b01000,0b01000,0b01000,0b00100,0b00010}},
        {')', {0b01000,0b00100,0b00010,0b00010,0b00010,0b00100,0b01000}},
        {'/', {0b00001,0b00010,0b00010,0b00100,0b01000,0b01000,0b10000}},
        {'%', {0b11001,0b11010,0b00010,0b00100,0b01000,0b01011,0b10011}},
        {'_', {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b11111}},
        {'>', {0b01000,0b00100,0b00010,0b00001,0b00010,0b00100,0b01000}},
        {'<', {0b00010,0b00100,0b01000,0b10000,0b01000,0b00100,0b00010}},
        {' ', {0,0,0,0,0,0,0}},
    };
    return table;
}

// Draws a single glyph with the bottom-left corner at (x, y), each pixel
// scaled to `px` screen pixels. Uses filled quads (not points) so text
// stays crisp at any scale.
inline void drawGlyph(char ch, float x, float y, float px, Color c) {
    ch = (char)std::toupper((unsigned char)ch);
    const auto& table = glyphTable();
    auto it = table.find(ch);
    if (it == table.end()) return;
    const Glyph& g = it->second;
    glColor4f(c.r, c.g, c.b, c.a);
    for (int row = 0; row < 7; ++row) {
        unsigned char bits = g[row];
        for (int col = 0; col < 5; ++col) {
            if (bits & (1 << (4 - col))) {
                float gx = x + col * px;
                float gy = y + row * px;
                glBegin(GL_QUADS);
                glVertex2f(gx, gy);
                glVertex2f(gx + px, gy);
                glVertex2f(gx + px, gy + px);
                glVertex2f(gx, gy + px);
                glEnd();
            }
        }
    }
}

// Draws a left-aligned string. Returns the total pixel width consumed,
// which callers use to right-align or center text manually.
inline float drawText(const std::string& text, float x, float y, float px, Color c) {
    float cursor = x;
    for (char ch : text) {
        if (ch == '\n') continue;  // caller should split multi-line text
        drawGlyph(ch, cursor, y, px, c);
        cursor += 6.0f * px;  // 5px glyph + 1px spacing
    }
    return cursor - x;
}

inline float textWidth(const std::string& text, float px) {
    return (float)text.size() * 6.0f * px;
}

inline void drawTextCentered(const std::string& text, float centerX, float y, float px, Color c) {
    float w = textWidth(text, px);
    drawText(text, centerX - w * 0.5f, y, px, c);
}

// Draws multi-line text (split on '\n'), left-aligned, with a fixed
// line height. Useful for dialogue boxes and help screens.
inline void drawParagraph(const std::string& text, float x, float y, float px, Color c, float lineHeight) {
    std::stringstream ss(text);
    std::string line;
    float cursorY = y;
    while (std::getline(ss, line, '\n')) {
        drawText(line, x, cursorY, px, c);
        cursorY += lineHeight;
    }
}

// Very small word-wrap helper: greedily packs words into lines no wider
// than maxWidth (in "px units" of the font scale used later), returning
// the wrapped text with '\n' inserted.
inline std::string wordWrap(const std::string& text, size_t maxCharsPerLine) {
    std::stringstream in(text);
    std::string word;
    std::string result;
    size_t lineLen = 0;
    bool first = true;
    while (in >> word) {
        if (!first && lineLen + 1 + word.size() > maxCharsPerLine) {
            result += '\n';
            lineLen = 0;
            first = true;
        }
        if (!first) { result += ' '; lineLen += 1; }
        result += word;
        lineLen += word.size();
        first = false;
    }
    return result;
}

}  // namespace Font

// ============================================================================
// SECTION 4 — DRAWING PRIMITIVES
//    These wrap the classic algorithms named in the design doc so the
//    "funny gameplay" is literally built out of the graphics techniques
//    being taught, not just referenced in flavor text.
// ============================================================================

namespace Draw {

// ---- Point plotting -------------------------------------------------------
inline void point(float x, float y, Color c, float size = 4.0f) {
    glColor4f(c.r, c.g, c.b, c.a);
    glPointSize(size);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

inline void points(const std::vector<Vec2>& pts, Color c, float size = 3.0f) {
    glColor4f(c.r, c.g, c.b, c.a);
    glPointSize(size);
    glBegin(GL_POINTS);
    for (auto& p : pts) glVertex2f(p.x, p.y);
    glEnd();
}

// ---- DDA line algorithm (Digital Differential Analyzer) -------------------
// Generates evenly spaced sample points along a line using incremental
// floating-point steps. Used for the *visual preview* of roads/segments
// before the player commits to a Bresenham-based repair.
inline std::vector<Vec2> ddaLine(Vec2 a, Vec2 b) {
    std::vector<Vec2> pts;
    float dx = b.x - a.x, dy = b.y - a.y;
    int steps = (int)std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) { pts.push_back(a); return pts; }
    float xInc = dx / steps, yInc = dy / steps;
    float x = a.x, y = a.y;
    for (int i = 0; i <= steps; ++i) {
        pts.emplace_back(x, y);
        x += xInc;
        y += yInc;
    }
    return pts;
}

// ---- Bresenham's line algorithm (integer, classic form) -------------------
// Used for "repairing broken roads." Returns the list of pixel centers so
// gameplay code can animate the road being rebuilt point-by-point. This is
// the *authoritative* algorithm for the BrokenRoad glitch's minigame -
// the player literally watches Bresenham's decision variable pick pixels.
inline std::vector<Vec2> bresenhamLine(int x0, int y0, int x1, int y1) {
    std::vector<Vec2> pts;
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int guard = 0;
    while (guard++ < 10000) {
        pts.emplace_back((float)x0, (float)y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    return pts;
}

inline void drawLineDDA(Vec2 a, Vec2 b, Color c, float thickness = 3.0f) {
    glColor4f(c.r, c.g, c.b, c.a);
    glLineWidth(thickness);
    glBegin(GL_LINES);
    glVertex2f(a.x, a.y);
    glVertex2f(b.x, b.y);
    glEnd();
}

inline void drawPolyline(const std::vector<Vec2>& pts, Color c, float thickness = 2.0f) {
    if (pts.size() < 2) return;
    glColor4f(c.r, c.g, c.b, c.a);
    glLineWidth(thickness);
    glBegin(GL_LINE_STRIP);
    for (auto& p : pts) glVertex2f(p.x, p.y);
    glEnd();
}

// ---- Midpoint Circle algorithm ---------------------------------------------
// Used to "inflate or shrink wheels." Generates circle points via the
// integer decision-variable method (no trig calls) which are then rendered
// as points to visually demonstrate 8-way symmetry.
inline std::vector<Vec2> midpointCircle(int cx, int cy, int r) {
    std::vector<Vec2> pts;
    if (r <= 0) { pts.emplace_back((float)cx, (float)cy); return pts; }
    int x = 0, y = r;
    int d = 1 - r;
    while (x <= y) {
        pts.emplace_back((float)(cx + x), (float)(cy + y));
        pts.emplace_back((float)(cx - x), (float)(cy + y));
        pts.emplace_back((float)(cx + x), (float)(cy - y));
        pts.emplace_back((float)(cx - x), (float)(cy - y));
        pts.emplace_back((float)(cx + y), (float)(cy + x));
        pts.emplace_back((float)(cx - y), (float)(cy + x));
        pts.emplace_back((float)(cx + y), (float)(cy - x));
        pts.emplace_back((float)(cx - y), (float)(cy - x));
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
    return pts;
}

inline void fillCircle(Vec2 center, float radius, Color c, int segments = 28) {
    if (radius <= 0.0f) return;
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(center.x, center.y);
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / segments * 2.0f * (float)M_PI;
        glVertex2f(center.x + radius * std::cos(theta),
                   center.y + radius * std::sin(theta));
    }
    glEnd();
}

inline void ringCircle(Vec2 center, float radius, Color c, float thickness = 2.0f, int segments = 32) {
    glColor4f(c.r, c.g, c.b, c.a);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        float theta = (float)i / segments * 2.0f * (float)M_PI;
        glVertex2f(center.x + radius * std::cos(theta),
                   center.y + radius * std::sin(theta));
    }
    glEnd();
}

inline void strokeCircleFromMidpoint(Vec2 center, float radius, Color c, float pointSize = 2.0f) {
    auto pts = midpointCircle(0, 0, (int)radius);
    glColor4f(c.r, c.g, c.b, c.a);
    glPointSize(pointSize);
    glBegin(GL_POINTS);
    for (auto& p : pts) glVertex2f(center.x + p.x, center.y + p.y);
    glEnd();
}

// ---- 2D affine transforms (translate / rotate / scale / shear) -----------
struct Transform {
    Vec2  translation{0, 0};
    float rotationDeg = 0.0f;
    Vec2  scale{1, 1};
    float shearX = 0.0f;
    float shearY = 0.0f;
};

inline Vec2 applyTransform(Vec2 p, const Transform& t) {
    // Scale
    p.x *= t.scale.x;
    p.y *= t.scale.y;
    // Shear (x' = x + shearX * y,  y' = y + shearY * x)
    float shX = p.x + t.shearX * p.y;
    float shY = p.y + t.shearY * p.x;
    p.x = shX; p.y = shY;
    // Rotate
    float rad = t.rotationDeg * (float)M_PI / 180.0f;
    float cosA = std::cos(rad), sinA = std::sin(rad);
    float rx = p.x * cosA - p.y * sinA;
    float ry = p.x * sinA + p.y * cosA;
    // Translate
    return {rx + t.translation.x, ry + t.translation.y};
}

inline void drawPolygon(const std::vector<Vec2>& localPts, const Transform& t, Color c) {
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_POLYGON);
    for (auto& p : localPts) {
        Vec2 w = applyTransform(p, t);
        glVertex2f(w.x, w.y);
    }
    glEnd();
}

inline void drawPolygonOutline(const std::vector<Vec2>& localPts, const Transform& t, Color c, float thickness = 2.0f) {
    glColor4f(c.r, c.g, c.b, c.a);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    for (auto& p : localPts) {
        Vec2 w = applyTransform(p, t);
        glVertex2f(w.x, w.y);
    }
    glEnd();
}

// Regular N-gon generator, used heavily for Polygon Dog's changing side
// count and for generic "polygonal" props around Polygon Market.
inline std::vector<Vec2> regularPolygon(float radius, int sides) {
    std::vector<Vec2> pts;
    sides = std::max(3, sides);
    for (int i = 0; i < sides; ++i) {
        float theta = (float)i / sides * 2.0f * (float)M_PI - (float)M_PI / 2.0f;
        pts.emplace_back(radius * std::cos(theta), radius * std::sin(theta));
    }
    return pts;
}

// ---- Reflection -------------------------------------------------------------
inline Vec2 reflectAcrossVerticalAxis(Vec2 p, float axisX) {
    return {2.0f * axisX - p.x, p.y};
}
inline Vec2 reflectAcrossHorizontalAxis(Vec2 p, float axisY) {
    return {p.x, 2.0f * axisY - p.y};
}
inline Vec2 reflectAcrossPoint(Vec2 p, Vec2 pivot) {
    return {2.0f * pivot.x - p.x, 2.0f * pivot.y - p.y};
}

// ---- Quadratic & cubic Bezier curves -----------------------------------------
inline Vec2 bezierQuad(Vec2 p0, Vec2 p1, Vec2 p2, float t) {
    float u = 1.0f - t;
    return p0 * (u * u) + p1 * (2.0f * u * t) + p2 * (t * t);
}

inline Vec2 bezierCubic(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t) {
    float u = 1.0f - t;
    return p0 * (u * u * u) + p1 * (3 * u * u * t) + p2 * (3 * u * t * t) + p3 * (t * t * t);
}

inline void drawBezierQuad(Vec2 p0, Vec2 p1, Vec2 p2, Color c, int segments = 24, float thickness = 4.0f) {
    glColor4f(c.r, c.g, c.b, c.a);
    glLineWidth(thickness);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / segments;
        Vec2 p = bezierQuad(p0, p1, p2, t);
        glVertex2f(p.x, p.y);
    }
    glEnd();
}

inline void drawBezierCubic(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, Color c, int segments = 30, float thickness = 4.0f) {
    glColor4f(c.r, c.g, c.b, c.a);
    glLineWidth(thickness);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / segments;
        Vec2 p = bezierCubic(p0, p1, p2, p3, t);
        glVertex2f(p.x, p.y);
    }
    glEnd();
}

// ---- Cohen-Sutherland style clipping (axis-aligned rect) --------------------
struct Rect {
    float x0, y0, x1, y1;
    float width() const { return x1 - x0; }
    float height() const { return y1 - y0; }
    Vec2 center() const { return {(x0 + x1) * 0.5f, (y0 + y1) * 0.5f}; }
    bool contains(Vec2 p) const { return p.x >= x0 && p.x <= x1 && p.y >= y0 && p.y <= y1; }
};

inline int computeOutCode(Vec2 p, const Rect& r) {
    int code = 0;
    if (p.x < r.x0) code |= 1;       // left
    else if (p.x > r.x1) code |= 2;  // right
    if (p.y < r.y0) code |= 4;       // bottom
    else if (p.y > r.y1) code |= 8;  // top
    return code;
}

// Clips a segment to rect; returns true and fills a,b with the clipped
// endpoints if any part of the segment survives.
inline bool clipSegment(Vec2 a, Vec2 b, const Rect& r, Vec2& outA, Vec2& outB) {
    int codeA = computeOutCode(a, r);
    int codeB = computeOutCode(b, r);
    bool accept = false;
    for (int guard = 0; guard < 8; ++guard) {
        if (!(codeA | codeB)) { accept = true; break; }
        if (codeA & codeB) break;  // trivially reject
        int codeOut = codeA ? codeA : codeB;
        Vec2 p;
        if (codeOut & 8) {         // top
            p.x = a.x + (b.x - a.x) * (r.y1 - a.y) / (b.y - a.y);
            p.y = r.y1;
        } else if (codeOut & 4) {  // bottom
            p.x = a.x + (b.x - a.x) * (r.y0 - a.y) / (b.y - a.y);
            p.y = r.y0;
        } else if (codeOut & 2) {  // right
            p.y = a.y + (b.y - a.y) * (r.x1 - a.x) / (b.x - a.x);
            p.x = r.x1;
        } else {                   // left
            p.y = a.y + (b.y - a.y) * (r.x0 - a.x) / (b.x - a.x);
            p.x = r.x0;
        }
        if (codeOut == codeA) { a = p; codeA = computeOutCode(a, r); }
        else { b = p; codeB = computeOutCode(b, r); }
    }
    if (accept) { outA = a; outB = b; }
    return accept;
}

inline void drawRectOutline(Rect r, Color c, float thickness = 2.0f) {
    glColor4f(c.r, c.g, c.b, c.a);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f(r.x0, r.y0);
    glVertex2f(r.x1, r.y0);
    glVertex2f(r.x1, r.y1);
    glVertex2f(r.x0, r.y1);
    glEnd();
}

inline void filledRect(Rect r, Color c) {
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_QUADS);
    glVertex2f(r.x0, r.y0);
    glVertex2f(r.x1, r.y0);
    glVertex2f(r.x1, r.y1);
    glVertex2f(r.x0, r.y1);
    glEnd();
}

inline void filledRoundedRectApprox(Rect r, float corner, Color c, int cornerSegs = 6) {
    // Cheap rounded-rect: central cross of quads + quarter-circle fans.
    corner = std::min(corner, std::min(r.width(), r.height()) * 0.5f);
    filledRect({r.x0 + corner, r.y0, r.x1 - corner, r.y1}, c);
    filledRect({r.x0, r.y0 + corner, r.x1, r.y1 - corner}, c);
    Vec2 corners[4] = {
        {r.x0 + corner, r.y0 + corner}, {r.x1 - corner, r.y0 + corner},
        {r.x1 - corner, r.y1 - corner}, {r.x0 + corner, r.y1 - corner}
    };
    float startAngle[4] = {180.0f, 270.0f, 0.0f, 90.0f};
    glColor4f(c.r, c.g, c.b, c.a);
    for (int k = 0; k < 4; ++k) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(corners[k].x, corners[k].y);
        for (int i = 0; i <= cornerSegs; ++i) {
            float a = degToRad(startAngle[k] + 90.0f * i / cornerSegs);
            glVertex2f(corners[k].x + corner * std::cos(a), corners[k].y + corner * std::sin(a));
        }
        glEnd();
    }
}

inline void filledTriangle(Vec2 a, Vec2 b, Vec2 c_, Color c) {
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_TRIANGLES);
    glVertex2f(a.x, a.y);
    glVertex2f(b.x, b.y);
    glVertex2f(c_.x, c_.y);
    glEnd();
}

}  // namespace Draw

// ============================================================================
// SECTION 5 — PARTICLE SYSTEM
//    Small, dependency-free particle pool used for repair sparks, pixel-
//    escape puffs, celebration bursts, and ambient district effects
//    (falling leaves, floating bubbles near Reflection Lake, etc).
// ============================================================================

struct Particle {
    Vec2  pos;
    Vec2  vel;
    Color color;
    float life = 1.0f;
    float maxLife = 1.0f;
    float size = 3.0f;
    float gravity = 0.0f;
    float spin = 0.0f;
    bool  square = false;

    bool alive() const { return life > 0.0f; }

    void update(float dt) {
        vel.y += gravity * dt;
        pos += vel * dt;
        life -= dt;
        spin += dt * 4.0f;
    }

    void draw(const struct Camera& cam) const;  // defined after Camera exists
};

class ParticleSystem {
public:
    void spawnBurst(Vec2 worldPos, Color c, int count, float speed, float life, bool square = false) {
        for (int i = 0; i < count; ++i) {
            Particle p;
            p.pos = worldPos;
            float angle = randf(0.0f, 2.0f * (float)M_PI);
            float spd = randf(speed * 0.4f, speed);
            p.vel = {std::cos(angle) * spd, std::sin(angle) * spd};
            p.color = c;
            p.life = p.maxLife = randf(life * 0.6f, life);
            p.size = randf(2.0f, 5.0f);
            p.gravity = 60.0f;
            p.square = square;
            particles_.push_back(p);
        }
    }

    void spawnSparkle(Vec2 worldPos, Color c) {
        spawnBurst(worldPos, c, 6, 90.0f, 0.6f, false);
    }

    void spawnConfetti(Vec2 worldPos) {
        static const Color palette[5] = {
            {1.0f, 0.3f, 0.3f, 1}, {0.3f, 0.7f, 1.0f, 1}, {1.0f, 0.85f, 0.2f, 1},
            {0.4f, 0.9f, 0.4f, 1}, {0.8f, 0.4f, 1.0f, 1}
        };
        for (int i = 0; i < 24; ++i) {
            Particle p;
            p.pos = worldPos;
            float angle = randf(0.0f, 2.0f * (float)M_PI);
            float spd = randf(40.0f, 160.0f);
            p.vel = {std::cos(angle) * spd, std::sin(angle) * spd - 60.0f};
            p.color = palette[randi(0, 4)];
            p.life = p.maxLife = randf(0.8f, 1.6f);
            p.size = randf(3.0f, 6.0f);
            p.gravity = 140.0f;
            p.square = true;
            particles_.push_back(p);
        }
    }

    void update(float dt) {
        for (auto& p : particles_) p.update(dt);
        particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                         [](const Particle& p) { return !p.alive(); }),
                          particles_.end());
    }

    const std::vector<Particle>& particles() const { return particles_; }
    size_t count() const { return particles_.size(); }

private:
    std::vector<Particle> particles_;
};

// ============================================================================
// SECTION 6 — WORLD DATA: DISTRICTS, PROPS, CAMERA, MINIMAP
// ============================================================================

struct District {
    std::string name;
    Draw::Rect  bounds;
    Color       tint;
    std::string flavorText;   // shown briefly when the player enters
};

static std::vector<District> gDistricts = {
    {"Pixel Plaza",          {0,    0,    600,  500},  {0.85f, 0.85f, 0.95f, 1},
     "Where every pixel in the city gets its start in life."},
    {"Line Street",          {600,  0,    1300, 500},  {0.80f, 0.90f, 0.85f, 1},
     "Home to the DDA & Bresenham road crews."},
    {"Polygon Market",       {1300, 0,    2000, 500},  {0.95f, 0.85f, 0.75f, 1},
     "Vendors sell N-gons of every side count."},
    {"RGB Park",             {2000, 0,    2700, 500},  {0.90f, 0.80f, 0.90f, 1},
     "The RGB Wizard's favorite picnic spot."},
    {"Curve Garden",         {2700, 0,    3600, 500},  {0.80f, 0.95f, 0.80f, 1},
     "Bezier Grandma knits every path here."},
    {"Reflection Lake",      {0,    500,  900,  1200}, {0.75f, 0.88f, 0.95f, 1},
     "Perfectly symmetrical. Usually."},
    {"Debug Laboratory",     {900,  500,  1800, 1200}, {0.88f, 0.88f, 0.88f, 1},
     "Professor Pixel's questionable research happens here."},
    {"Transformation Tower", {1800, 500,  2700, 1200}, {0.95f, 0.90f, 0.70f, 1},
     "Buildings rotate, scale, and occasionally wander off."},
    {"Glitch Factory",       {2700, 500,  3600, 1200}, {0.70f, 0.70f, 0.75f, 1},
     "Ground zero for reality's crash. Enter at your own risk."},
};

// A lightweight decorative prop (tree, lamp, bench, cloud, mailbox...) that
// gives districts visual identity beyond flat tint rectangles. Purely
// cosmetic - no gameplay interaction - but essential for the "living
// world" feel described in the design doc.
enum class PropKind { Tree, Lamp, Bench, Mailbox, Cloud, Bush, RoadTile, Fountain, Sign };

struct Prop {
    PropKind kind;
    Vec2     pos;
    float    scale = 1.0f;
    float    phase = 0.0f;  // per-instance animation offset

    void update(float /*dt*/) {}

    void draw(const struct Camera& cam, float time) const;  // defined after Camera
};

inline std::vector<Prop> generateProps() {
    std::vector<Prop> props;
    std::srand(1337);  // deterministic layout so the world feels authored, not random each run
    for (auto& d : gDistricts) {
        int count = (int)(d.bounds.width() * d.bounds.height() / 90000.0f);
        for (int i = 0; i < count; ++i) {
            Prop p;
            float rx = randf(d.bounds.x0 + 40, d.bounds.x1 - 40);
            float ry = randf(d.bounds.y0 + 40, d.bounds.y1 - 40);
            p.pos = {rx, ry};
            p.phase = randf(0.0f, 6.28f);
            p.scale = randf(0.8f, 1.3f);

            if (d.name == "Curve Garden")            p.kind = PropKind::Bush;
            else if (d.name == "RGB Park")           p.kind = (i % 2 == 0) ? PropKind::Tree : PropKind::Bench;
            else if (d.name == "Reflection Lake")    p.kind = PropKind::Fountain;
            else if (d.name == "Debug Laboratory")   p.kind = PropKind::Sign;
            else if (d.name == "Transformation Tower") p.kind = PropKind::Lamp;
            else if (d.name == "Glitch Factory")     p.kind = PropKind::Mailbox;
            else if (d.name == "Line Street")        p.kind = PropKind::RoadTile;
            else                                     p.kind = (i % 3 == 0) ? PropKind::Tree : PropKind::Lamp;
            props.push_back(p);
        }
    }
    // A handful of clouds drifting above everything.
    for (int i = 0; i < 10; ++i) {
        Prop p;
        p.kind = PropKind::Cloud;
        p.pos = {randf(0, Config::kWorldWidth), randf(20, 160)};
        p.phase = randf(0.0f, 6.28f);
        p.scale = randf(0.9f, 1.8f);
        props.push_back(p);
    }
    std::srand((unsigned)std::time(nullptr));  // restore true randomness for gameplay RNG
    return props;
}

// ---- Camera -----------------------------------------------------------------
struct Camera {
    Vec2  pos{Config::kWorldWidth * 0.5f, Config::kWorldHeight * 0.5f};
    float shakeTime = 0.0f;
    float shakeMagnitude = 0.0f;

    void follow(Vec2 target, float dt) {
        pos.x = lerp(pos.x, target.x, clampf(Config::kCameraEase * dt, 0.0f, 1.0f));
        pos.y = lerp(pos.y, target.y, clampf(Config::kCameraEase * dt, 0.0f, 1.0f));
    }

    void shake(float magnitude, float duration) {
        shakeMagnitude = std::max(shakeMagnitude, magnitude);
        shakeTime = std::max(shakeTime, duration);
    }

    void update(float dt) {
        if (shakeTime > 0.0f) shakeTime -= dt;
        else shakeMagnitude = 0.0f;
    }

    Vec2 shakeOffset() const {
        if (shakeTime <= 0.0f) return {0, 0};
        return {randf(-shakeMagnitude, shakeMagnitude), randf(-shakeMagnitude, shakeMagnitude)};
    }

    Vec2 worldToScreen(Vec2 world) const {
        Vec2 off = shakeOffset();
        return {world.x - pos.x + Config::kWindowWidth * 0.5f + off.x,
                world.y - pos.y + Config::kWindowHeight * 0.5f + off.y};
    }
};

inline void Particle::draw(const Camera& cam) const {
    Vec2 s = cam.worldToScreen(pos);
    float t = clampf(life / std::max(0.001f, maxLife), 0.0f, 1.0f);
    Color c = color.withAlpha(color.a * t);
    if (square) {
        float half = size * 0.5f;
        glPushMatrix();
        glTranslatef(s.x, s.y, 0);
        glRotatef(spin * 40.0f, 0, 0, 1);
        Draw::filledRect({-half, -half, half, half}, c);
        glPopMatrix();
    } else {
        Draw::fillCircle(s, size * t + 1.0f, c, 8);
    }
}

inline void Prop::draw(const Camera& cam, float time) const {
    Vec2 s = cam.worldToScreen(pos);
    float bob = std::sin(time * 1.2f + phase) * 2.0f;
    switch (kind) {
        case PropKind::Tree: {
            Draw::filledRect({s.x - 3 * scale, s.y, s.x + 3 * scale, s.y + 18 * scale}, {0.45f, 0.30f, 0.20f, 1});
            Draw::fillCircle({s.x, s.y - 4 * scale + bob}, 14 * scale, {0.35f, 0.65f, 0.35f, 1}, 10);
            break;
        }
        case PropKind::Lamp: {
            Draw::filledRect({s.x - 2, s.y - 30 * scale, s.x + 2, s.y}, {0.3f, 0.3f, 0.35f, 1});
            Draw::fillCircle({s.x, s.y - 32 * scale}, 6 * scale, {1.0f, 0.9f, 0.5f, 0.9f}, 10);
            break;
        }
        case PropKind::Bench: {
            Draw::filledRect({s.x - 14 * scale, s.y - 6, s.x + 14 * scale, s.y}, {0.55f, 0.4f, 0.25f, 1});
            Draw::filledRect({s.x - 14 * scale, s.y - 14, s.x + 14 * scale, s.y - 10}, {0.55f, 0.4f, 0.25f, 1});
            break;
        }
        case PropKind::Mailbox: {
            Draw::filledRect({s.x - 5, s.y - 16, s.x + 5, s.y}, {0.7f, 0.2f, 0.2f, 1});
            Draw::fillCircle({s.x, s.y - 16}, 5, {0.7f, 0.2f, 0.2f, 1}, 8);
            break;
        }
        case PropKind::Cloud: {
            float cx = std::fmod(s.x + time * 6.0f, (float)Config::kWindowWidth + 200.0f) - 100.0f;
            Draw::fillCircle({cx - 10 * scale, s.y, }, 12 * scale, {1, 1, 1, 0.85f}, 10);
            Draw::fillCircle({cx + 6 * scale, s.y - 4 * scale}, 15 * scale, {1, 1, 1, 0.85f}, 10);
            Draw::fillCircle({cx + 22 * scale, s.y}, 11 * scale, {1, 1, 1, 0.85f}, 10);
            break;
        }
        case PropKind::Bush: {
            Draw::fillCircle({s.x, s.y + bob}, 10 * scale, {0.4f, 0.7f, 0.4f, 1}, 8);
            break;
        }
        case PropKind::RoadTile: {
            Draw::filledRect({s.x - 20, s.y - 3, s.x + 20, s.y + 3}, {0.6f, 0.6f, 0.62f, 0.5f});
            break;
        }
        case PropKind::Fountain: {
            Draw::ringCircle(s, 16 * scale, {0.5f, 0.7f, 0.9f, 1}, 3.0f);
            Draw::fillCircle({s.x, s.y - bob}, 4 * scale, {0.7f, 0.85f, 1.0f, 0.8f}, 8);
            break;
        }
        case PropKind::Sign: {
            Draw::filledRect({s.x - 1, s.y - 20, s.x + 1, s.y}, {0.4f, 0.4f, 0.4f, 1});
            Draw::filledRect({s.x - 14, s.y - 28, s.x + 14, s.y - 18}, {0.9f, 0.9f, 0.3f, 1});
            break;
        }
    }
}

// ---- Minimap ------------------------------------------------------------------
inline void drawMinimap(const Camera& cam, Vec2 playerWorldPos,
                          const std::vector<struct Glitch>& glitches);  // fwd, implemented after Glitch

// ============================================================================
// SECTION 7 — PLAYER ENTITY (Patch, the Graphics Engineer)
// ============================================================================

enum class PlayerAnim { Idle, Walk, Slide, Victory, Panic, Trip };

struct Player {
    Vec2  pos{Config::kWorldWidth * 0.5f, Config::kWorldHeight * 0.5f};
    Vec2  velocity{0, 0};          // normalized input direction
    Vec2  actualVelocity{0, 0};    // smoothed velocity used for slide detection
    float facing = 0.0f;           // degrees, 0 = facing right
    bool  moving = false;
    bool  running = false;
    float animTimer = 0.0f;
    PlayerAnim anim = PlayerAnim::Idle;
    float animLockTimer = 0.0f;    // when > 0, anim can't be overridden (victory/panic/trip)

    int   repairsCompleted = 0;
    int   totalGlitchesSeen = 0;
    float distanceTraveled = 0.0f;
    float sessionTime = 0.0f;

    // "trip over loose pixels" is a small random comedic event
    float tripCooldown = 3.0f;

    void triggerVictory() { anim = PlayerAnim::Victory; animLockTimer = 0.9f; animTimer = 0.0f; }
    void triggerPanic()   { anim = PlayerAnim::Panic;   animLockTimer = 0.7f; animTimer = 0.0f; }
    void triggerTrip()    { anim = PlayerAnim::Trip;    animLockTimer = 0.6f; animTimer = 0.0f; }

    void update(float dt, bool invertedControls, bool frozen) {
        sessionTime += dt;
        if (frozen) return;

        float dir = invertedControls ? -1.0f : 1.0f;
        float speedMult = running ? Config::kPlayerRunMult : 1.0f;
        Vec2 targetVel = velocity * (Config::kPlayerSpeed * speedMult * dir);

        // smooth acceleration so movement doesn't feel instant/robotic
        actualVelocity = lerpVec(actualVelocity, targetVel, clampf(dt * 10.0f, 0.0f, 1.0f));

        Vec2 prevPos = pos;
        pos += actualVelocity * dt;
        pos.x = clampf(pos.x, 20.0f, Config::kWorldWidth - 20.0f);
        pos.y = clampf(pos.y, 20.0f, Config::kWorldHeight - 20.0f);
        distanceTraveled += (pos - prevPos).length();

        moving = velocity.length() > 0.01f;
        if (moving) {
            facing = std::atan2(velocity.y, velocity.x) * 180.0f / (float)M_PI;
            animTimer += dt * (running ? 1.6f : 1.0f);
        }

        // decide which animation to show, unless a locked one-shot anim is playing
        if (animLockTimer > 0.0f) {
            animLockTimer -= dt;
        } else {
            if (moving) {
                // sudden direction reversal reads as a "slide to a stop"
                bool reversing = actualVelocity.dot(targetVel) < -0.1f;
                anim = reversing ? PlayerAnim::Slide : PlayerAnim::Walk;
            } else {
                anim = PlayerAnim::Idle;
            }
        }

        // Random comedic trip over "loose pixels" while walking.
        if (moving && animLockTimer <= 0.0f) {
            tripCooldown -= dt;
            if (tripCooldown <= 0.0f) {
                tripCooldown = randf(14.0f, 26.0f);
                if (randf(0, 1) < 0.5f) triggerTrip();
            }
        }
    }

    void draw(const Camera& cam) const {
        Vec2 s = cam.worldToScreen(pos);
        float bob = 0.0f, squash = 1.0f, tilt = 0.0f, capeFlap = 0.0f;

        switch (anim) {
            case PlayerAnim::Idle:
                bob = std::sin(animTimer * 2.0f) * 1.0f;
                break;
            case PlayerAnim::Walk:
                bob = std::sin(animTimer * 12.0f) * 3.0f;
                capeFlap = std::sin(animTimer * 12.0f) * 6.0f;
                break;
            case PlayerAnim::Slide:
                squash = 0.8f;
                tilt = 12.0f;
                break;
            case PlayerAnim::Victory: {
                float t = 1.0f - clampf(animLockTimer / 0.9f, 0.0f, 1.0f);
                bob = std::sin(t * (float)M_PI * 4.0f) * 8.0f * (1.0f - t);
                break;
            }
            case PlayerAnim::Panic:
                bob = std::sin(animTimer * 30.0f) * 2.0f;
                tilt = std::sin(animTimer * 25.0f) * 10.0f;
                break;
            case PlayerAnim::Trip:
                tilt = 70.0f;
                squash = 0.7f;
                break;
        }

        glPushMatrix();
        glTranslatef(s.x, s.y + bob, 0);
        glRotatef(tilt, 0, 0, 1);
        glScalef(squash, 1.0f / squash, 1.0f);

        // Cape (graph-paper texture approximated with a grid overlay)
        Draw::filledRect({-10, 8, 10 + capeFlap * 0.2f, 24}, {1, 1, 1, 0.75f});
        for (int i = -8; i <= 8; i += 4) {
            Draw::drawLineDDA({(float)i, 8}, {(float)i, 24}, {0.7f, 0.8f, 0.9f, 0.5f}, 1.0f);
        }

        // Backpack
        Draw::filledRect({-22, -6, -12, 14}, {0.4f, 0.3f, 0.2f, 1});
        // Floating holographic wrench (little accessory near the hand)
        Draw::filledRect({14, -2, 26, 4}, {0.3f, 0.9f, 1.0f, 0.6f});

        // Body (orange repair suit)
        Draw::fillCircle({0, 0}, 16, {1.0f, 0.55f, 0.15f, 1});
        // Goggles
        Draw::fillCircle({-6, -8}, 5, {0.9f, 0.95f, 1.0f, 1});
        Draw::fillCircle({6, -8}, 5, {0.9f, 0.95f, 1.0f, 1});
        Draw::ringCircle({-6, -8}, 5, {0.2f, 0.2f, 0.25f, 1}, 1.5f);
        Draw::ringCircle({6, -8}, 5, {0.2f, 0.2f, 0.25f, 1}, 1.5f);

        // Panic sweat drop
        if (anim == PlayerAnim::Panic) {
            Draw::fillCircle({12, -16 + std::sin(animTimer * 20.0f) * 3.0f}, 2.5f, {0.5f, 0.8f, 1.0f, 0.9f}, 6);
        }

        glPopMatrix();
    }
};

// ============================================================================
// SECTION 8 — NPC ENTITIES & BEHAVIOR
// ============================================================================

enum class NpcKind { CaptainCircle, RgbWizard, PolygonDog, BezierGrandma, DebugPigeon, ProfessorPixel };

// A tiny finite-state machine driving each NPC's per-frame "quirk" without
// needing a full behavior-tree library. Each NPC also wanders slowly within
// a leash radius of its home position so districts feel populated without
// characters drifting into the wrong neighborhood.
struct Npc {
    NpcKind     kind;
    std::string displayName;
    Vec2        pos;
    Vec2        home;
    float       leash = 90.0f;
    float       timer = 0.0f;
    float       wanderAngle = 0.0f;
    int         state = 0;           // generic state index (shape stage, color stage, etc.)
    float       chatCooldown = 0.0f;
    int         linesSaidCount = 0;
    bool        talkedToOnce = false;

    static Npc make(NpcKind k, std::string name, Vec2 position) {
        Npc n;
        n.kind = k;
        n.displayName = std::move(name);
        n.pos = position;
        n.home = position;
        n.wanderAngle = randf(0.0f, 6.28f);
        n.chatCooldown = randf(0.0f, 3.0f);
        return n;
    }

    void update(float dt, bool polygonFluActive) {
        timer += dt;
        chatCooldown = std::max(0.0f, chatCooldown - dt);

        // gentle wander, pulled back toward home if it strays too far
        wanderAngle += dt * randf(0.3f, 0.7f);
        Vec2 wanderDir{std::cos(wanderAngle), std::sin(wanderAngle * 1.3f)};
        Vec2 toHome = home - pos;
        float distFromHome = toHome.length();
        Vec2 pullBack = distFromHome > leash ? toHome.normalized() : Vec2{0, 0};
        pos += (wanderDir * 6.0f + pullBack * 30.0f) * dt;

        switch (kind) {
            case NpcKind::CaptainCircle:
                if (timer > 1.2f) { timer = 0; state = (state + 1) % 5; }  // circle->oval->egg->potato->circle
                break;
            case NpcKind::RgbWizard:
                if (timer > 2.2f) { timer = 0; state = randi(0, 2); }     // blue/red/green spell
                break;
            case NpcKind::PolygonDog:
                if (timer > 1.6f) {
                    timer = 0;
                    state = polygonFluActive ? std::max(0, state - 1) : randi(0, 3);  // flu forces fewer sides
                }
                break;
            case NpcKind::DebugPigeon:
                if (timer > 3.0f) { timer = 0; state = randi(0, 1); }  // idle vs "stealing pixel" pose
                break;
            default:
                break;
        }
    }

    // Returns true if the NPC has something new to say right now (used to
    // pop a speech bubble without the player needing to press a button).
    bool wantsToSpeak() const { return chatCooldown <= 0.0f; }

    void markSpoke() {
        chatCooldown = randf(6.0f, 12.0f);
        linesSaidCount++;
        talkedToOnce = true;
    }

    void draw(const Camera& cam, float globalTime) const {
        Vec2 s = cam.worldToScreen(pos);
        switch (kind) {
            case NpcKind::CaptainCircle: {
                float rx = 14, ry = 14;
                if (state == 1) { rx = 18; ry = 11; }                       // oval
                if (state == 2) { rx = 12; ry = 18; }                       // egg
                if (state == 3) { rx = 16 + std::sin(timer * 9) * 3; ry = 13; }  // potato wobble
                glPushMatrix();
                glTranslatef(s.x, s.y, 0);
                glScalef(rx / 14.0f, ry / 14.0f, 1.0f);
                Draw::fillCircle({0, 0}, 14, {0.2f, 0.4f, 0.9f, 1});
                glPopMatrix();
                // little security badge
                Draw::filledRect({s.x - 3, s.y - 2, s.x + 3, s.y + 3}, {1.0f, 0.85f, 0.2f, 1});
                break;
            }
            case NpcKind::RgbWizard: {
                Color spell = state == 0 ? Color{0.2f, 0.3f, 1.0f, 1}
                            : state == 1 ? Color{1.0f, 0.2f, 0.2f, 1}
                                         : Color{0.2f, 1.0f, 0.3f, 1};
                Draw::fillCircle(s, 15, spell);
                Draw::filledRect({s.x - 3, s.y - 26, s.x + 3, s.y - 15}, {0.5f, 0.2f, 0.6f, 1});  // hat brim
                Draw::filledTriangle({s.x - 8, s.y - 15}, {s.x + 8, s.y - 15}, {s.x, s.y - 34}, {0.5f, 0.2f, 0.6f, 1});
                break;
            }
            case NpcKind::PolygonDog: {
                int sides = state == 0 ? 3 : state == 1 ? 6 : state == 2 ? 32 : 1;
                Color fur{0.9f, 0.7f, 0.4f, 1};
                if (sides == 1) {
                    Draw::point(s.x, s.y, fur, 10.0f);
                } else {
                    Draw::fillCircle(s, 13, fur, std::max(3, sides));
                    Draw::drawPolygonOutline(Draw::regularPolygon(13, sides),
                                              {s, 0, {1, 1}, 0}, {0.5f, 0.35f, 0.15f, 1}, 1.5f);
                }
                break;
            }
            case NpcKind::BezierGrandma: {
                Draw::fillCircle(s, 13, {0.8f, 0.8f, 0.9f, 1});
                Draw::fillCircle({s.x, s.y - 16}, 8, {0.9f, 0.9f, 0.92f, 1});  // grey bun
                // knitting needles
                Draw::drawLineDDA({s.x - 14, s.y + 4}, {s.x - 2, s.y - 4}, {0.6f, 0.5f, 0.3f, 1}, 2.0f);
                Draw::drawLineDDA({s.x + 14, s.y + 4}, {s.x + 2, s.y - 4}, {0.6f, 0.5f, 0.3f, 1}, 2.0f);
                break;
            }
            case NpcKind::DebugPigeon: {
                float bob = std::sin(globalTime * 6.0f + timer) * 4.0f;
                Draw::fillCircle({s.x, s.y + bob}, 8, {0.6f, 0.6f, 0.65f, 1});
                if (state == 1) {
                    // "stealing pixel" - a little stolen square in its beak
                    Draw::filledRect({s.x + 6, s.y + bob - 2, s.x + 10, s.y + bob + 2}, {1, 0, 1, 1});
                }
                break;
            }
            case NpcKind::ProfessorPixel: {
                Draw::fillCircle(s, 15, {0.95f, 0.9f, 0.8f, 1});
                Draw::filledRect({s.x - 10, s.y - 8, s.x + 10, s.y - 2}, {0.1f, 0.1f, 0.1f, 1});  // glasses
                // sine-wave hair
                std::vector<Vec2> hair;
                for (int i = -10; i <= 10; ++i) {
                    hair.emplace_back(s.x + i, s.y - 14 + std::sin((i + globalTime * 40.0f) * 0.4f) * 3.0f);
                }
                Draw::drawPolyline(hair, {0.3f, 0.2f, 0.15f, 1}, 2.0f);
                break;
            }
        }
    }

    // Each NPC has a small pool of in-character lines it can say when the
    // player is nearby. Lines rotate so repeated visits stay fresh.
    std::string nextLine() const {
        static const std::vector<std::string> captain = {
            "DON'T LAUGH. THIS IS A MEDICAL CONDITION.",
            "I WAS A PERFECT CIRCLE ONCE. ONCE.",
            "SECURITY REPORT: EVERYTHING IS FINE. PROBABLY.",
        };
        static const std::vector<std::string> wizard = {
            "OOPS... WRONG SLIDER.",
            "RED SPELL MAKES EVERYONE ANGRY. WHOOPS.",
            "I MEANT TO DYE MY ROBE, NOT THE SKY.",
        };
        static const std::vector<std::string> dog = {
            "*BARKS IN 32 SIDES*",
            "*BECOMES ONE PIXEL. BARKS LOUDLY.*",
            "*CHASES A DEBUG PIGEON*",
        };
        static const std::vector<std::string> grandma = {
            "BACK IN MY DAY, ROADS WERE STRAIGHT!",
            "THIS RIVER NEEDS TWO MORE CONTROL POINTS, DEARIE.",
            "KNIT ONE, CURVE TWO.",
        };
        static const std::vector<std::string> pigeon = {
            "*STEALS A PIXEL FROM A ROOFTOP*",
            "*WINS CITIZEN OF THE MONTH, AGAIN*",
            "*DROPS CORRUPTED CODE ON A PASSERBY*",
        };
        static const std::vector<std::string> professor = {
            "INTERESTING... THE TRIANGLE HAS DEVELOPED EMOTIONS.",
            "MY HYPOTHESIS: THE BUG LIKES COFFEE. UNTESTED.",
            "ACCORDING TO MY NOTES, GRAVITY IS OPTIONAL TODAY.",
        };
        const std::vector<std::string>* pool = &captain;
        switch (kind) {
            case NpcKind::CaptainCircle:   pool = &captain; break;
            case NpcKind::RgbWizard:       pool = &wizard; break;
            case NpcKind::PolygonDog:      pool = &dog; break;
            case NpcKind::BezierGrandma:   pool = &grandma; break;
            case NpcKind::DebugPigeon:     pool = &pigeon; break;
            case NpcKind::ProfessorPixel:  pool = &professor; break;
        }
        return (*pool)[linesSaidCount % pool->size()];
    }
};

// ---- NPC-to-NPC "living world" interactions ---------------------------------
// Rather than scripting fixed cutscenes, we detect proximity between two
// specific NPC kinds each frame and, on a cooldown, fire a small comedic
// vignette (a speech bubble pair) - satisfying "Polygon Dog chases Debug
// Pigeons," "Professor Pixel argues with traffic lights," etc. at low
// implementation cost while keeping it dynamic.
struct LivingWorldInteraction {
    float cooldown = 0.0f;
    std::string speakerA, speakerB, lineA, lineB;
    float displayTimer = 0.0f;

    void update(std::vector<Npc>& npcs, float dt) {
        cooldown = std::max(0.0f, cooldown - dt);
        displayTimer = std::max(0.0f, displayTimer - dt);
        if (cooldown > 0.0f) return;

        for (size_t i = 0; i < npcs.size(); ++i) {
            for (size_t j = i + 1; j < npcs.size(); ++j) {
                if ((npcs[i].pos - npcs[j].pos).length() < 50.0f) {
                    fireVignette(npcs[i], npcs[j]);
                    cooldown = randf(10.0f, 18.0f);
                    return;
                }
            }
        }
    }

    void fireVignette(const Npc& a, const Npc& b) {
        speakerA = a.displayName;
        speakerB = b.displayName;
        if (a.kind == NpcKind::PolygonDog && b.kind == NpcKind::DebugPigeon) {
            lineA = "*CHASES*"; lineB = "*FLEES, DROPS A PIXEL*";
        } else if (a.kind == NpcKind::ProfessorPixel) {
            lineA = "THIS TRAFFIC LIGHT IS WRONG."; lineB = "*BLINKS RED DEFIANTLY*";
        } else {
            lineA = "OH, HELLO."; lineB = "WATCH OUT FOR GLITCHES TODAY.";
        }
        displayTimer = 3.0f;
    }

    bool active() const { return displayTimer > 0.0f; }
};

// ============================================================================
// SECTION 9 — DIALOGUE / SPEECH BUBBLE SYSTEM
// ============================================================================

struct SpeechBubble {
    std::string text;
    Vec2        anchorWorldPos;
    float       timeLeft = 0.0f;
    float       maxTime = 2.6f;

    bool active() const { return timeLeft > 0.0f; }

    void update(float dt) { timeLeft = std::max(0.0f, timeLeft - dt); }

    void draw(const Camera& cam) const {
        if (!active()) return;
        Vec2 s = cam.worldToScreen(anchorWorldPos) + Vec2{0, -50};
        float fade = clampf(timeLeft / 0.3f, 0.0f, 1.0f) * clampf((maxTime - timeLeft) / 0.2f, 0.0f, 1.0f);
        std::string wrapped = Font::wordWrap(text, 22);

        // measure box size from wrapped text
        std::stringstream ss(wrapped);
        std::string line;
        float maxW = 0.0f;
        int lineCount = 0;
        while (std::getline(ss, line, '\n')) {
            maxW = std::max(maxW, Font::textWidth(line, 2.0f));
            lineCount++;
        }
        float boxW = maxW + 16.0f;
        float boxH = lineCount * 14.0f + 12.0f;
        Draw::Rect box{s.x - boxW * 0.5f, s.y - boxH, s.x + boxW * 0.5f, s.y};
        Draw::filledRoundedRectApprox(box, 6.0f, {1, 1, 1, 0.9f * fade});
        Draw::drawRectOutline(box, {0.2f, 0.2f, 0.25f, fade}, 1.5f);
        // little tail pointing down to the speaker
        Draw::filledTriangle({s.x - 6, s.y}, {s.x + 6, s.y}, {s.x, s.y + 8}, {1, 1, 1, 0.9f * fade});

        Font::drawParagraph(wrapped, box.x0 + 8.0f, box.y0 + 6.0f, 2.0f, {0.15f, 0.15f, 0.2f, fade}, 14.0f);
    }
};

// A small rotating queue of speech bubbles so several NPCs can "talk" near
// the player without their text overlapping; only the most recent few are
// shown, oldest fades first.
class DialogueManager {
public:
    void say(const std::string& speakerTag, const std::string& text, Vec2 worldPos) {
        SpeechBubble b;
        b.text = "[" + speakerTag + "] " + text;
        b.anchorWorldPos = worldPos;
        b.timeLeft = b.maxTime;
        bubbles_.push_back(b);
        if (bubbles_.size() > 3) bubbles_.pop_front();
    }

    void update(float dt) {
        for (auto& b : bubbles_) b.update(dt);
        while (!bubbles_.empty() && !bubbles_.front().active()) bubbles_.pop_front();
    }

    void draw(const Camera& cam) const {
        for (auto& b : bubbles_) b.draw(cam);
    }

private:
    std::deque<SpeechBubble> bubbles_;
};

// ============================================================================
// SECTION 10 — GLITCH SYSTEM
// ----------------------------------------------------------------------------
//  Every glitch type now runs a small, self-contained "minigame" driven by
//  the same handful of inputs (movement into a target zone + the repair
//  key), rather than a single generic progress bar. This keeps input
//  handling simple (no mouse-driven UI required, so it plays fine with just
//  a keyboard) while still making each graphics algorithm *felt* rather
//  than just labeled.
//
//  Common pattern per glitch type:
//    - `progress`   drives the repair completion (0..1)
//    - `phase`      an internal float/int used for that type's specific
//                    mechanic (approach angle, target radius, slider value…)
//    - `repairTick` is called every frame the player holds the repair key
//                    while standing close enough; each type interprets the
//                    held direction / timing slightly differently so the
//                    "feel" differs even though the control scheme is
//                    shared.
// ============================================================================

enum class GlitchType {
    StrayPixel, BrokenRoad, WobblyWheel, DriftedBuilding,
    NoodleTower, MirrorTwin, TangledRiver, LeakingObject, ColorSickness
};

inline const char* glitchTypeName(GlitchType t) {
    switch (t) {
        case GlitchType::StrayPixel:      return "STRAY PIXEL";
        case GlitchType::BrokenRoad:      return "BROKEN ROAD";
        case GlitchType::WobblyWheel:     return "WOBBLY WHEEL";
        case GlitchType::DriftedBuilding: return "DRIFTED BUILDING";
        case GlitchType::NoodleTower:     return "NOODLE TOWER";
        case GlitchType::MirrorTwin:      return "MIRROR TWIN";
        case GlitchType::TangledRiver:    return "TANGLED RIVER";
        case GlitchType::LeakingObject:   return "LEAKING OBJECT";
        case GlitchType::ColorSickness:   return "COLOR SICKNESS";
    }
    return "UNKNOWN";
}

inline const char* glitchAlgorithmName(GlitchType t) {
    switch (t) {
        case GlitchType::StrayPixel:      return "POINT PLOTTING";
        case GlitchType::BrokenRoad:      return "BRESENHAM LINE ALGORITHM";
        case GlitchType::WobblyWheel:     return "MIDPOINT CIRCLE ALGORITHM";
        case GlitchType::DriftedBuilding: return "2D TRANSLATION / ROTATION";
        case GlitchType::NoodleTower:     return "SHEARING TRANSFORM";
        case GlitchType::MirrorTwin:      return "REFLECTION";
        case GlitchType::TangledRiver:    return "BEZIER CURVE FITTING";
        case GlitchType::LeakingObject:   return "COHEN-SUTHERLAND CLIPPING";
        case GlitchType::ColorSickness:   return "RGB COLOR BALANCING";
    }
    return "";
}

inline const char* glitchHint(GlitchType t) {
    switch (t) {
        case GlitchType::StrayPixel:      return "HOLD [E] TO RE-PLOT THE PIXEL.";
        case GlitchType::BrokenRoad:      return "HOLD [E] TO LAY DOWN ROAD PIXELS STEP BY STEP.";
        case GlitchType::WobblyWheel:     return "HOLD [E] TO GROW THE WHEEL TO THE RIGHT RADIUS.";
        case GlitchType::DriftedBuilding: return "HOLD [E] TO PUSH THE BUILDING BACK ONTO ITS SLOT.";
        case GlitchType::NoodleTower:     return "HOLD [E] TO UN-SHEAR THE LEANING TOWER.";
        case GlitchType::MirrorTwin:      return "HOLD [E] TO REALIGN THE REFLECTION.";
        case GlitchType::TangledRiver:    return "HOLD [E] TO SMOOTH THE CURVE'S CONTROL POINT.";
        case GlitchType::LeakingObject:   return "HOLD [E] TO CLIP THE LEAK BACK INSIDE THE WALL.";
        case GlitchType::ColorSickness:   return "HOLD [E] TO BALANCE THE RGB CHANNELS.";
    }
    return "";
}

struct Glitch {
    GlitchType type;
    Vec2       pos;
    float      progress = 0.0f;   // 0 = broken, 1 = repaired
    bool       resolved = false;
    bool       celebrated = false;
    float      wobble = 0.0f;
    float      age = 0.0f;
    float      resolvedTimer = 0.0f;  // countdown before removal, for celebration VFX

    // per-type extra state (reused loosely across types to avoid a huge union)
    float phaseA = 0.0f;   // e.g. current wheel radius, shear amount, hue mix
    float phaseB = 0.0f;   // e.g. target wheel radius, target shear, target hue
    float phaseC = 0.0f;   // e.g. secondary channel value
    int   difficulty = 1;  // scales target precision / distance

    static Glitch spawn(GlitchType t, Vec2 pos, int difficulty) {
        Glitch g;
        g.type = t;
        g.pos = pos;
        g.difficulty = difficulty;
        switch (t) {
            case GlitchType::WobblyWheel:
                g.phaseA = randf(4.0f, 12.0f);                 // current (wrong) radius
                g.phaseB = randf(20.0f, 30.0f);                 // target radius
                break;
            case GlitchType::DriftedBuilding:
                g.phaseA = randf(30.0f, 70.0f);                 // drift distance
                g.phaseB = randf(-40.0f, 40.0f);                // drift rotation
                break;
            case GlitchType::NoodleTower:
                g.phaseA = randf(0.5f, 1.1f);                   // shear amount
                break;
            case GlitchType::MirrorTwin:
                g.phaseA = randf(10.0f, 35.0f);                 // reflection offset error
                break;
            case GlitchType::TangledRiver:
                g.phaseA = randf(-50.0f, 50.0f);                // control point X error
                g.phaseB = randf(-50.0f, 50.0f);                // control point Y error
                break;
            case GlitchType::LeakingObject:
                g.phaseA = randf(-60.0f, 60.0f);                // leak extension beyond wall
                break;
            case GlitchType::ColorSickness:
                g.phaseA = randf(0.0f, 1.0f);                   // current R bias
                g.phaseB = randf(0.0f, 1.0f);                   // current G bias
                g.phaseC = randf(0.0f, 1.0f);                   // current B bias
                break;
            default:
                break;
        }
        return g;
    }

    const char* label() const { return glitchTypeName(type); }

    void update(float dt) {
        wobble += dt;
        age += dt;
        if (resolved) {
            resolvedTimer += dt;
            return;
        }
        if (progress > 0.0f) {
            // slowly regress if the player walks away mid-repair, adding
            // light time pressure without punishing exploration harshly
            progress = clampf(progress - dt * 0.05f, 0.0f, 1.0f);
        }
    }

    // Called continuously while the player holds the repair key near the
    // glitch. `dt` drives the base rate; each type nudges its own phase
    // value toward its target as a visible side-effect of repairing.
    void repairTick(float dt, ParticleSystem& particles, Camera& camera) {
        float rate = 0.55f / (0.6f + 0.15f * difficulty);
        progress = clampf(progress + dt * rate, 0.0f, 1.0f);

        switch (type) {
            case GlitchType::WobblyWheel:
                phaseA = lerp(phaseA, phaseB, dt * 2.0f);
                break;
            case GlitchType::DriftedBuilding:
                phaseA = lerp(phaseA, 0.0f, dt * 2.0f);
                phaseB = lerp(phaseB, 0.0f, dt * 2.0f);
                break;
            case GlitchType::NoodleTower:
                phaseA = lerp(phaseA, 0.0f, dt * 2.0f);
                break;
            case GlitchType::MirrorTwin:
                phaseA = lerp(phaseA, 0.0f, dt * 2.0f);
                break;
            case GlitchType::TangledRiver:
                phaseA = lerp(phaseA, 0.0f, dt * 2.0f);
                phaseB = lerp(phaseB, 0.0f, dt * 2.0f);
                break;
            case GlitchType::LeakingObject:
                phaseA = lerp(phaseA, 0.0f, dt * 2.5f);
                break;
            case GlitchType::ColorSickness:
                phaseA = lerp(phaseA, 0.5f, dt * 1.5f);
                phaseB = lerp(phaseB, 0.5f, dt * 1.5f);
                phaseC = lerp(phaseC, 0.5f, dt * 1.5f);
                break;
            default:
                break;
        }

        if (std::fmod(progress, 0.18f) < dt * rate) {
            particles.spawnSparkle(pos, {0.3f, 0.9f, 1.0f, 1});
        }

        if (progress >= 1.0f && !resolved) {
            resolved = true;
            particles.spawnConfetti(pos);
            camera.shake(4.0f, 0.2f);
        }
    }

    void draw(const Camera& cam) const {
        Vec2 s = cam.worldToScreen(pos);
        float shake = resolved ? 0.0f : std::sin(wobble * 20.0f) * (1.0f - progress) * 3.0f;
        Vec2 sd{s.x + shake, s.y};
        float glow = resolved ? clampf(1.0f - resolvedTimer / 1.2f, 0.0f, 1.0f) : 0.0f;

        switch (type) {
            case GlitchType::StrayPixel: {
                float size = 8.0f + 4.0f * (1 - progress);
                Draw::point(sd.x, sd.y, {1, 0, 1, 1}, size);
                Draw::ringCircle(sd, size + 3.0f, {1, 0, 1, 0.3f}, 1.0f);
                break;
            }
            case GlitchType::BrokenRoad: {
                auto pts = Draw::bresenhamLine((int)sd.x - 60, (int)sd.y, (int)sd.x + 60, (int)sd.y + 20);
                int drawCount = (int)(pts.size() * progress);
                for (int i = 0; i < (int)pts.size(); ++i) {
                    Color c = i < drawCount ? Color{0.3f, 0.3f, 0.3f, 1} : Color{1, 0.3f, 0.3f, 0.6f};
                    Draw::point(pts[i].x, pts[i].y, c, 3);
                }
                break;
            }
            case GlitchType::WobblyWheel: {
                Draw::strokeCircleFromMidpoint(sd, phaseA, {0.15f, 0.15f, 0.15f, 1});
                Draw::ringCircle(sd, phaseB, {0.2f, 0.8f, 0.3f, 0.35f}, 1.0f);  // ghost target radius
                Draw::fillCircle(sd, 4, {0.7f, 0.7f, 0.7f, 1});
                break;
            }
            case GlitchType::DriftedBuilding: {
                std::vector<Vec2> box = {{-20, -20}, {20, -20}, {20, 20}, {-20, 20}};
                Draw::Transform t;
                t.translation = {sd.x + phaseA, sd.y};
                t.rotationDeg = phaseB;
                Draw::drawPolygon(box, t, {0.6f, 0.75f, 0.95f, 1});
                Draw::drawPolygonOutline(box, t, {0.2f, 0.3f, 0.5f, 1});
                Draw::drawPolygonOutline(box, {sd, 0, {1, 1}, 0}, {0.2f, 0.7f, 0.3f, 0.35f});  // target slot ghost
                break;
            }
            case GlitchType::NoodleTower: {
                std::vector<Vec2> tower = {{-10, -40}, {10, -40}, {10, 40}, {-10, 40}};
                Draw::Transform t;
                t.translation = sd;
                t.shearX = phaseA;
                Draw::drawPolygon(tower, t, {0.85f, 0.7f, 0.9f, 1});
                Draw::drawPolygonOutline(tower, t, {0.4f, 0.2f, 0.5f, 1});
                break;
            }
            case GlitchType::MirrorTwin: {
                Draw::fillCircle({sd.x - 30, sd.y}, 12, {0.9f, 0.6f, 0.3f, 1});
                Draw::fillCircle({sd.x + 30 + phaseA, sd.y}, 12, {0.9f, 0.6f, 0.3f, 0.8f});
                Draw::drawLineDDA({sd.x, sd.y - 30}, {sd.x, sd.y + 30}, {0.5f, 0.5f, 0.5f, 0.4f}, 1.0f);  // mirror axis
                break;
            }
            case GlitchType::TangledRiver: {
                Vec2 p0{sd.x - 60, sd.y + 30};
                Vec2 p2{sd.x + 60, sd.y - 30};
                Vec2 p1{sd.x + phaseA, sd.y + phaseB};
                Draw::drawBezierQuad(p0, p1, p2, {0.3f, 0.6f, 0.9f, 1});
                Draw::point(p1.x, p1.y, {1, 0.5f, 0, 0.6f}, 5.0f);  // control point marker
                break;
            }
            case GlitchType::LeakingObject: {
                Draw::Rect wallBounds{sd.x - 40, sd.y - 30, sd.x + 40, sd.y + 30};
                Draw::drawRectOutline(wallBounds, {0.5f, 0.5f, 0.5f, 1});
                Vec2 a{sd.x - 70, sd.y - 50 - phaseA * 0.3f};
                Vec2 b{sd.x + 70, sd.y + 50 + phaseA * 0.3f};
                Vec2 ca, cb;
                if (Draw::clipSegment(a, b, wallBounds, ca, cb)) {
                    Draw::drawLineDDA(ca, cb, {1.0f, 0.5f, 0.0f, 1}, 4.0f);
                }
                if (std::abs(phaseA) > 1.0f) {
                    Draw::drawLineDDA(a, ca, {1.0f, 0.2f, 0.2f, 0.5f}, 2.0f);  // visible "leak" outside wall
                    Draw::drawLineDDA(b, cb, {1.0f, 0.2f, 0.2f, 0.5f}, 2.0f);
                }
                break;
            }
            case GlitchType::ColorSickness: {
                Color sick{phaseA, phaseB, phaseC, 1};
                Draw::fillCircle(sd, 16, sick);
                // little RGB channel bars beneath, showing the mix visually
                Draw::filledRect({sd.x - 18, sd.y + 22, sd.x - 18 + 36 * phaseA, sd.y + 25}, {1, 0.3f, 0.3f, 1});
                Draw::filledRect({sd.x - 18, sd.y + 26, sd.x - 18 + 36 * phaseB, sd.y + 29}, {0.3f, 1, 0.3f, 1});
                Draw::filledRect({sd.x - 18, sd.y + 30, sd.x - 18 + 36 * phaseC, sd.y + 33}, {0.3f, 0.3f, 1, 1});
                break;
            }
        }

        if (glow > 0.0f) {
            Draw::ringCircle(sd, 30.0f + (1.0f - glow) * 20.0f, {1.0f, 0.9f, 0.3f, glow * 0.6f}, 2.0f);
        }
    }
};

// ============================================================================
// SECTION 11 — DYNAMIC WORLD EVENTS
// ============================================================================

enum class WorldEvent {
    None, CoffeeOverflow, RgbFestival, PolygonFlu,
    InvertedGravity, GiantCursorAttack, WindowsUpdate, LowFpsStorm
};

struct EventState {
    WorldEvent current = WorldEvent::None;
    float      timeLeft = 0.0f;
    float      totalDuration = 0.0f;
    int        eventsTriggeredCount = 0;
    WorldEvent lastEvent = WorldEvent::None;

    const char* name() const {
        switch (current) {
            case WorldEvent::CoffeeOverflow:    return "COFFEE OVERFLOW";
            case WorldEvent::RgbFestival:       return "RGB FESTIVAL";
            case WorldEvent::PolygonFlu:        return "POLYGON FLU";
            case WorldEvent::InvertedGravity:   return "INVERTED GRAVITY";
            case WorldEvent::GiantCursorAttack: return "GIANT CURSOR ATTACK";
            case WorldEvent::WindowsUpdate:     return "WINDOWS UPDATE";
            case WorldEvent::LowFpsStorm:       return "LOW FPS STORM";
            default: return "";
        }
    }

    const char* description() const {
        switch (current) {
            case WorldEvent::CoffeeOverflow:    return "PROFESSOR PIXEL SPILLED COFFEE. GRAVITY IS SIDEWAYS!";
            case WorldEvent::RgbFestival:       return "THE RGB WIZARD RANDOMIZED EVERY COLOR IN TOWN!";
            case WorldEvent::PolygonFlu:        return "EVERYONE IS LOSING POLYGONS. STAY BLOCKY, STAY STRONG.";
            case WorldEvent::InvertedGravity:   return "CATS FLY. TREES HANG UPSIDE DOWN. CONTROLS: REVERSED.";
            case WorldEvent::GiantCursorAttack: return "A GIANT CURSOR IS DRAGGING BUILDINGS AROUND!";
            case WorldEvent::WindowsUpdate:     return "PLEASE DO NOT TURN OFF THE CITY.";
            case WorldEvent::LowFpsStorm:       return "THE WHOLE CITY IS PRETENDING TO LAG.";
            default: return "";
        }
    }

    void trigger() {
        WorldEvent choice;
        do { choice = (WorldEvent)randi(1, 7); } while (choice == lastEvent);
        current = choice;
        lastEvent = choice;
        timeLeft = totalDuration = randf(7.0f, 12.0f);
        eventsTriggeredCount++;
        std::printf("[EVENT] %s - %s\n", name(), description());
    }

    void update(float dt) {
        if (current == WorldEvent::None) return;
        timeLeft -= dt;
        if (timeLeft <= 0.0f) {
            std::printf("[EVENT] ...and it's over. Reality resumes (mostly).\n");
            current = WorldEvent::None;
        }
    }

    float progressRemaining() const {
        return totalDuration > 0.0f ? clampf(timeLeft / totalDuration, 0.0f, 1.0f) : 0.0f;
    }

    bool invertsControls() const { return current == WorldEvent::InvertedGravity; }
    bool isFrozen()        const { return current == WorldEvent::WindowsUpdate; }
    bool isLagStorm()      const { return current == WorldEvent::LowFpsStorm; }
    bool isColorChaos()    const { return current == WorldEvent::RgbFestival; }
    bool isPolygonFlu()    const { return current == WorldEvent::PolygonFlu; }
    bool isCursorAttack()  const { return current == WorldEvent::GiantCursorAttack; }
    bool isCoffeeOverflow() const { return current == WorldEvent::CoffeeOverflow; }
};

// The Giant Cursor Attack event spawns a wandering oversized cursor that
// visually "grabs" a random district tint and drags it a short distance,
// purely as a cosmetic gag layered on top of normal rendering.
struct GiantCursor {
    Vec2  pos{0, 0};
    Vec2  target{0, 0};
    bool  visible = false;
    float retargetTimer = 0.0f;

    void activate() {
        visible = true;
        pos = {randf(200, Config::kWorldWidth - 200), randf(200, Config::kWorldHeight - 200)};
        target = pos;
        retargetTimer = 0.0f;
    }

    void deactivate() { visible = false; }

    void update(float dt) {
        if (!visible) return;
        retargetTimer -= dt;
        if (retargetTimer <= 0.0f) {
            target = {randf(100, Config::kWorldWidth - 100), randf(100, Config::kWorldHeight - 100)};
            retargetTimer = randf(1.5f, 3.0f);
        }
        pos = lerpVec(pos, target, dt * 1.5f);
    }

    void draw(const Camera& cam) const {
        if (!visible) return;
        Vec2 s = cam.worldToScreen(pos);
        Color c{0.1f, 0.1f, 0.15f, 0.85f};
        std::vector<Vec2> arrow = {{0, 0}, {0, 34}, {8, 26}, {13, 36}, {17, 34}, {12, 24}, {22, 24}};
        Draw::drawPolygon(arrow, {s, 0, {1.4f, 1.4f}, 0}, c);
        Draw::drawPolygonOutline(arrow, {s, 0, {1.4f, 1.4f}, 0}, {1, 1, 1, 0.6f}, 1.5f);
    }
};

// ============================================================================
// SECTION 12 — HUD & UI SCREENS
// ============================================================================

enum class ScreenState { Title, Playing, Paused, Help, GameStats };

// ---- Minimap implementation (declared earlier in world section) -----------
inline void drawMinimap(const Camera& cam, Vec2 playerWorldPos, const std::vector<Glitch>& glitches) {
    float mapX = Config::kWindowWidth - Config::kMinimapSize - Config::kMinimapMargin;
    float mapY = Config::kMinimapMargin;
    Draw::Rect mapRect{mapX, mapY, mapX + Config::kMinimapSize, mapY + Config::kMinimapSize};

    Draw::filledRoundedRectApprox(mapRect, 8.0f, {0.08f, 0.08f, 0.12f, 0.7f});
    Draw::drawRectOutline(mapRect, {1, 1, 1, 0.5f}, 1.5f);

    auto worldToMap = [&](Vec2 w) -> Vec2 {
        float nx = w.x / Config::kWorldWidth;
        float ny = w.y / Config::kWorldHeight;
        return {mapRect.x0 + nx * Config::kMinimapSize, mapRect.y0 + ny * Config::kMinimapSize};
    };

    for (auto& d : gDistricts) {
        Vec2 tl = worldToMap({d.bounds.x0, d.bounds.y0});
        Vec2 br = worldToMap({d.bounds.x1, d.bounds.y1});
        Draw::filledRect({tl.x, tl.y, br.x, br.y}, d.tint.withAlpha(0.55f));
    }

    for (auto& g : glitches) {
        if (g.resolved) continue;
        Vec2 gp = worldToMap(g.pos);
        Draw::fillCircle(gp, 3.0f, {1, 0.2f, 0.6f, 1}, 6);
    }

    Vec2 pp = worldToMap(playerWorldPos);
    Draw::fillCircle(pp, 4.0f, {1.0f, 0.6f, 0.1f, 1}, 8);
    Draw::ringCircle(pp, 6.0f, {1, 1, 1, 0.8f}, 1.5f);

    (void)cam;
}

// ---- HUD (top bar: district name, repair counter, event banner) -----------
struct Hud {
    std::string enteredDistrictBanner;
    float       bannerTimer = 0.0f;

    void notifyDistrict(const std::string& name, const std::string& flavor) {
        enteredDistrictBanner = name + " - " + flavor;
        bannerTimer = 3.0f;
    }

    void update(float dt) { bannerTimer = std::max(0.0f, bannerTimer - dt); }

    void drawTopBar(int repairs, int glitchesActive, const EventState& ev) const {
        Draw::filledRect({0, 0, (float)Config::kWindowWidth, 34}, {0.08f, 0.08f, 0.12f, 0.55f});
        Font::drawText("REPAIRS: " + std::to_string(repairs), 12, 10, 2.2f, {1, 1, 1, 1});
        Font::drawText("ACTIVE GLITCHES: " + std::to_string(glitchesActive), 230, 10, 2.2f, {1, 0.85f, 0.85f, 1});

        if (ev.current != WorldEvent::None) {
            std::string label = std::string(ev.name());
            float w = Font::textWidth(label, 2.4f);
            float x = Config::kWindowWidth * 0.5f - w * 0.5f;
            Draw::filledRoundedRectApprox({x - 10, 38, x + w + 10, 62}, 6.0f, {0.8f, 0.15f, 0.2f, 0.7f});
            Font::drawTextCentered(label, Config::kWindowWidth * 0.5f, 44, 2.4f, {1, 1, 0.9f, 1});
        }
    }

    void drawDistrictBanner() const {
        if (bannerTimer <= 0.0f) return;
        float fade = clampf(bannerTimer / 0.5f, 0.0f, 1.0f) * clampf((3.0f - bannerTimer) / 0.5f + 1.0f, 0.0f, 1.0f);
        fade = clampf(std::min(1.0f, bannerTimer), 0.0f, 1.0f);
        std::string wrapped = Font::wordWrap(enteredDistrictBanner, 46);
        Font::drawTextCentered(wrapped, Config::kWindowWidth * 0.5f, 74, 2.0f, {0.15f, 0.15f, 0.2f, fade});
    }

    void drawInteractPrompt(const char* hint, Vec2 screenAnchor) const {
        Draw::filledRoundedRectApprox({screenAnchor.x - 90, screenAnchor.y - 40, screenAnchor.x + 90, screenAnchor.y - 14},
                                       5.0f, {0.05f, 0.05f, 0.08f, 0.75f});
        Font::drawTextCentered(hint, screenAnchor.x, screenAnchor.y - 34, 1.6f, {0.6f, 1.0f, 0.7f, 1});
    }
};

// ---- Title Screen -----------------------------------------------------------
inline void drawTitleScreen(float time) {
    glClearColor(0.10f, 0.10f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // animated background of drifting little squares to feel like a broken
    // graphics engine even before gameplay starts
    for (int i = 0; i < 40; ++i) {
        float seed = (float)i * 71.0f;
        float x = std::fmod(seed * 3.1f + time * 20.0f, (float)Config::kWindowWidth);
        float y = std::fmod(seed * 5.7f + time * 12.0f, (float)Config::kWindowHeight);
        Color c{0.3f + 0.3f * std::sin(seed), 0.3f + 0.3f * std::cos(seed * 1.3f), 0.9f, 0.25f};
        Draw::filledRect({x, y, x + 5, y + 5}, c);
    }

    float titleBounce = std::sin(time * 2.0f) * 6.0f;
    Font::drawTextCentered("PIXEL PANIC", Config::kWindowWidth * 0.5f, 220 + titleBounce, 7.0f, {1.0f, 0.55f, 0.15f, 1});
    Font::drawTextCentered("SAVING REALITY.EXE", Config::kWindowWidth * 0.5f, 300 + titleBounce, 3.2f, {0.9f, 0.95f, 1.0f, 1});

    Font::drawTextCentered("PRESS ENTER TO START", Config::kWindowWidth * 0.5f, 470,
                             2.4f, {1, 1, 1, 0.6f + 0.4f * std::sin(time * 4.0f)});
    Font::drawTextCentered("PRESS H FOR HELP", Config::kWindowWidth * 0.5f, 510, 2.0f, {0.8f, 0.8f, 0.85f, 1});
    Font::drawTextCentered("WASD/ARROWS MOVE - SHIFT RUN - E REPAIR - ESC PAUSE",
                             Config::kWindowWidth * 0.5f, 560, 1.6f, {0.6f, 0.6f, 0.7f, 1});
}

// ---- Help / Instructions Screen ---------------------------------------------
inline void drawHelpScreen() {
    Draw::filledRect({0, 0, (float)Config::kWindowWidth, (float)Config::kWindowHeight}, {0.06f, 0.06f, 0.1f, 0.92f});
    Font::drawTextCentered("HOW TO PLAY", Config::kWindowWidth * 0.5f, 60, 3.5f, {1, 0.8f, 0.3f, 1});

    struct Line { std::string text; Color c; };
    std::vector<Line> lines = {
        {"MOVE: W A S D OR ARROW KEYS", {1,1,1,1}},
        {"RUN: HOLD LEFT SHIFT", {1,1,1,1}},
        {"REPAIR A NEARBY GLITCH: HOLD E OR SPACE", {1,1,1,1}},
        {"PAUSE: ESCAPE", {1,1,1,1}},
        {"", {1,1,1,1}},
        {"EVERY GLITCH TEACHES A REAL GRAPHICS ALGORITHM:", {0.6f, 1.0f, 0.7f, 1}},
        {"STRAY PIXEL - POINT PLOTTING", {0.85f,0.85f,0.9f,1}},
        {"BROKEN ROAD - BRESENHAM LINE ALGORITHM", {0.85f,0.85f,0.9f,1}},
        {"WOBBLY WHEEL - MIDPOINT CIRCLE ALGORITHM", {0.85f,0.85f,0.9f,1}},
        {"DRIFTED BUILDING - TRANSLATION AND ROTATION", {0.85f,0.85f,0.9f,1}},
        {"NOODLE TOWER - SHEARING", {0.85f,0.85f,0.9f,1}},
        {"MIRROR TWIN - REFLECTION", {0.85f,0.85f,0.9f,1}},
        {"TANGLED RIVER - BEZIER CURVES", {0.85f,0.85f,0.9f,1}},
        {"LEAKING OBJECT - COHEN-SUTHERLAND CLIPPING", {0.85f,0.85f,0.9f,1}},
        {"COLOR SICKNESS - RGB COLOR BALANCING", {0.85f,0.85f,0.9f,1}},
        {"", {1,1,1,1}},
        {"PRESS H OR ESCAPE TO RETURN", {0.7f, 0.7f, 0.8f, 1}},
    };
    float y = 130;
    for (auto& l : lines) {
        if (!l.text.empty()) Font::drawTextCentered(l.text, Config::kWindowWidth * 0.5f, y, 1.9f, l.c);
        y += 28;
    }
}

// ---- Pause Screen -------------------------------------------------------------
inline void drawPauseOverlay(int repairsCompleted, float sessionTime, float distanceTraveled) {
    Draw::filledRect({0, 0, (float)Config::kWindowWidth, (float)Config::kWindowHeight}, {0, 0, 0, 0.65f});
    Font::drawTextCentered("PAUSED", Config::kWindowWidth * 0.5f, 220, 4.5f, {1, 1, 1, 1});

    int minutes = (int)(sessionTime / 60.0f);
    int seconds = (int)sessionTime % 60;
    char timeBuf[32];
    std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", minutes, seconds);

    Font::drawTextCentered(std::string("REPAIRS COMPLETED: ") + std::to_string(repairsCompleted),
                             Config::kWindowWidth * 0.5f, 320, 2.2f, {0.8f, 1, 0.85f, 1});
    Font::drawTextCentered(std::string("SESSION TIME: ") + timeBuf,
                             Config::kWindowWidth * 0.5f, 355, 2.2f, {0.8f, 0.9f, 1, 1});
    Font::drawTextCentered(std::string("DISTANCE WALKED: ") + std::to_string((int)distanceTraveled) + "M",
                             Config::kWindowWidth * 0.5f, 390, 2.2f, {1, 0.9f, 0.8f, 1});

    Font::drawTextCentered("PRESS ESCAPE TO RESUME", Config::kWindowWidth * 0.5f, 460, 2.0f, {1, 1, 1, 0.8f});
    Font::drawTextCentered("PRESS H FOR HELP", Config::kWindowWidth * 0.5f, 495, 1.8f, {0.8f, 0.8f, 0.85f, 1});
    Font::drawTextCentered("PRESS Q TO QUIT", Config::kWindowWidth * 0.5f, 530, 1.8f, {1, 0.6f, 0.6f, 1});
}

// ---- Milestone / soft-win celebration banner ---------------------------------
inline void drawMilestoneBanner(int repairs, float time) {
    if (repairs < Config::kTargetGlitchesForWin) return;
    static bool announced = false;
    if (repairs == Config::kTargetGlitchesForWin) announced = true;
    if (!announced) return;
    float pulse = 0.7f + 0.3f * std::sin(time * 3.0f);
    Font::drawTextCentered("REALITY.EXE STABILIZED! KEEP GOING FOR FUN.",
                             Config::kWindowWidth * 0.5f, Config::kWindowHeight - 40,
                             1.8f, {1.0f, 0.9f, 0.3f, pulse});
}

// ============================================================================
// SECTION 13 — SESSION STATISTICS TRACKING
// ----------------------------------------------------------------------------
//  A lightweight in-memory stats tracker (no file I/O required, to keep the
//  project a true single self-contained file with zero external state).
//  Tracks per-glitch-type repair counts so the end-of-session stats screen
//  can show which graphics algorithm the player practiced most.
// ============================================================================

struct SessionStats {
    std::map<GlitchType, int> repairsByType;
    int   totalRepairs = 0;
    int   eventsWitnessed = 0;
    float bestSingleRepairTime = 999999.0f;
    float worldEntryTime = 0.0f;

    void recordRepair(GlitchType t, float repairDuration) {
        repairsByType[t]++;
        totalRepairs++;
        bestSingleRepairTime = std::min(bestSingleRepairTime, repairDuration);
    }

    void recordEvent() { eventsWitnessed++; }

    GlitchType mostRepairedType() const {
        GlitchType best = GlitchType::StrayPixel;
        int bestCount = -1;
        for (auto& [type, count] : repairsByType) {
            if (count > bestCount) { bestCount = count; best = type; }
        }
        return best;
    }

    std::string summaryText() const {
        std::ostringstream oss;
        oss << "TOTAL REPAIRS: " << totalRepairs << "\n";
        oss << "WORLD EVENTS SURVIVED: " << eventsWitnessed << "\n";
        if (totalRepairs > 0) {
            oss << "FAVORITE ALGORITHM: " << glitchAlgorithmName(mostRepairedType()) << "\n";
        }
        return oss.str();
    }
};

// ============================================================================
// SECTION 14 — GAME STATE AGGREGATION & UPDATE LOOP
// ============================================================================

struct GlitchRepairTiming {
    // tracks how long the *current* nearest glitch has been actively
    // repaired, purely so SessionStats can record a "best repair time"
    float activeRepairElapsed = 0.0f;
    const Glitch* activeGlitch = nullptr;
};

struct GameState {
    Player               player;
    Camera               camera;
    std::vector<Npc>      npcs;
    std::vector<Prop>     props;
    std::deque<Glitch>    glitches;
    EventState             worldEvent;
    ParticleSystem          particles;
    DialogueManager           dialogue;
    LivingWorldInteraction     livingWorld;
    GiantCursor                  cursor;
    Hud                            hud;
    SessionStats                   stats;
    GlitchRepairTiming              repairTiming;

    ScreenState screen = ScreenState::Title;
    ScreenState screenBeforeHelp = ScreenState::Title;

    float glitchSpawnTimer = 0.0f;
    float eventSpawnTimer  = 0.0f;
    bool  repairKeyHeld    = false;
    float globalTime       = 0.0f;

    District* lastDistrict = nullptr;

    void init() {
        npcs.push_back(Npc::make(NpcKind::CaptainCircle,  "CAPTAIN CIRCLE",  {700, 300}));
        npcs.push_back(Npc::make(NpcKind::RgbWizard,       "RGB WIZARD",      {2300, 250}));
        npcs.push_back(Npc::make(NpcKind::PolygonDog,      "POLYGON DOG",     {1600, 300}));
        npcs.push_back(Npc::make(NpcKind::BezierGrandma,   "BEZIER GRANDMA",  {3100, 300}));
        npcs.push_back(Npc::make(NpcKind::DebugPigeon,     "DEBUG PIGEON",    {400, 700}));
        npcs.push_back(Npc::make(NpcKind::ProfessorPixel,  "PROFESSOR PIXEL", {1300, 800}));
        props = generateProps();
        spawnGlitch();
        spawnGlitch();
    }

    void spawnGlitch() {
        if ((int)glitches.size() >= Config::kMaxGlitches) return;
        GlitchType t = (GlitchType)randi(0, 8);
        Vec2 pos = {randf(80, Config::kWorldWidth - 80), randf(80, Config::kWorldHeight - 80)};
        int difficulty = 1 + (stats.totalRepairs / 8);  // gently ramps up over a session
        Glitch g = Glitch::spawn(t, pos, std::min(difficulty, 4));
        glitches.push_back(g);
        std::printf("[GLITCH] New problem: %s (%s)\n", g.label(), glitchAlgorithmName(t));
    }

    District* districtAt(Vec2 p) {
        for (auto& d : gDistricts) {
            if (p.x >= d.bounds.x0 && p.x <= d.bounds.x1 &&
                p.y >= d.bounds.y0 && p.y <= d.bounds.y1) return &d;
        }
        return nullptr;
    }

    void updatePlaying(float dt) {
        globalTime += dt;
        hud.update(dt);
        dialogue.update(dt);

        bool frozen = worldEvent.isFrozen();
        camera.update(dt);

        if (!frozen) {
            float simDt = worldEvent.isLagStorm() ? dt * 0.35f : dt;  // dramatic "lag"
            player.update(simDt, worldEvent.invertsControls(), false);
            camera.follow(player.pos, dt);

            for (auto& n : npcs) n.update(simDt, worldEvent.isPolygonFlu());
            for (auto& p : props) p.update(simDt);
            for (auto& g : glitches) g.update(simDt);
            particles.update(simDt);
            livingWorld.update(npcs, simDt);

            if (worldEvent.isCursorAttack()) {
                if (!cursor.visible) cursor.activate();
                cursor.update(simDt);
            } else if (cursor.visible) {
                cursor.deactivate();
            }

            handleDistrictTransition();
            handleNpcChatter();
            handleGlitchRepair(dt);
            purgeResolvedGlitches(dt);

            glitchSpawnTimer += dt;
            if (glitchSpawnTimer >= Config::kGlitchSpawnEvery) {
                glitchSpawnTimer = 0.0f;
                spawnGlitch();
            }

            eventSpawnTimer += dt;
            if (eventSpawnTimer >= Config::kEventSpawnEvery && worldEvent.current == WorldEvent::None) {
                eventSpawnTimer = 0.0f;
                worldEvent.trigger();
                stats.recordEvent();
                if (worldEvent.isCoffeeOverflow()) camera.shake(6.0f, 0.4f);
            }
        } else {
            player.update(dt, false, true);  // still ticks internal timers, no movement
        }
        worldEvent.update(dt);
    }

    void handleDistrictTransition() {
        District* here = districtAt(player.pos);
        if (here != lastDistrict) {
            lastDistrict = here;
            if (here) hud.notifyDistrict(here->name, here->flavorText);
        }
    }

    void handleNpcChatter() {
        for (auto& n : npcs) {
            float d = (n.pos - player.pos).length();
            if (d < 90.0f && n.wantsToSpeak()) {
                dialogue.say(n.displayName, n.nextLine(), n.pos);
                n.markSpoke();
            }
        }
    }

    Glitch* nearestActiveGlitch() {
        Glitch* nearest = nullptr;
        float bestDist = Config::kInteractRadius;
        for (auto& g : glitches) {
            if (g.resolved) continue;
            float d = (g.pos - player.pos).length();
            if (d < bestDist) { bestDist = d; nearest = &g; }
        }
        return nearest;
    }

    void handleGlitchRepair(float dt) {
        Glitch* nearest = nearestActiveGlitch();

        if (repairKeyHeld && nearest) {
            bool wasResolved = nearest->resolved;
            float startProgress = nearest->progress;
            nearest->repairTick(dt, particles, camera);

            if (repairTiming.activeGlitch != nearest) {
                repairTiming.activeGlitch = nearest;
                repairTiming.activeRepairElapsed = 0.0f;
            }
            repairTiming.activeRepairElapsed += dt;

            if (!wasResolved && nearest->resolved) {
                player.repairsCompleted++;
                player.triggerVictory();
                stats.recordRepair(nearest->type, repairTiming.activeRepairElapsed);
                dialogue.say("PATCH", "FIXED! " + std::string(glitchAlgorithmName(nearest->type)) + " RESTORED.", player.pos);
                repairTiming.activeGlitch = nullptr;
            }
            (void)startProgress;
        } else {
            repairTiming.activeGlitch = nullptr;
            repairTiming.activeRepairElapsed = 0.0f;
        }

        // If a glitch has been actively worsening near the player without
        // being repaired, occasionally trigger a comedic panic animation.
        if (nearest && !repairKeyHeld && (nearest->pos - player.pos).length() < 40.0f) {
            if (randf(0, 1) < 0.002f) player.triggerPanic();
        }
    }

    void purgeResolvedGlitches(float dt) {
        static float purgeAccumulator = 0.0f;
        purgeAccumulator += dt;
        if (purgeAccumulator > 1.4f) {
            purgeAccumulator = 0.0f;
            glitches.erase(std::remove_if(glitches.begin(), glitches.end(),
                                           [](const Glitch& g) { return g.resolved && g.resolvedTimer > 1.2f; }),
                            glitches.end());
        }
    }
};

static GameState gGame;

// ============================================================================
// SECTION 15 — TOP-LEVEL RENDERING
// ============================================================================

void setupOrtho2D(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);  // origin top-left, y-down (screen space)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderDistricts(const Camera& cam) {
    for (auto& d : gDistricts) {
        Vec2 tl = cam.worldToScreen({d.bounds.x0, d.bounds.y0});
        Vec2 br = cam.worldToScreen({d.bounds.x1, d.bounds.y1});
        Draw::filledRect({tl.x, tl.y, br.x, br.y}, d.tint);
        Draw::drawRectOutline({tl.x, tl.y, br.x, br.y}, {0.6f, 0.6f, 0.6f, 0.5f});
    }
}

void renderDistrictLabels(const Camera& cam) {
    for (auto& d : gDistricts) {
        Vec2 topLeft = cam.worldToScreen({d.bounds.x0, d.bounds.y0});
        // only draw labels roughly on-screen, to avoid wasted draw calls
        if (topLeft.x < -200 || topLeft.x > Config::kWindowWidth + 200) continue;
        if (topLeft.y < -60 || topLeft.y > Config::kWindowHeight + 60) continue;
        Font::drawText(d.name, topLeft.x + 8, topLeft.y + 8, 1.6f, {0.2f, 0.2f, 0.25f, 0.55f});
    }
}

void renderWorldEventOverlay(const EventState& ev) {
    if (ev.current == WorldEvent::None) return;
    Color tint{0, 0, 0, 0};
    switch (ev.current) {
        case WorldEvent::RgbFestival:       tint = {1, 0, 1, 0.08f}; break;
        case WorldEvent::CoffeeOverflow:    tint = {0.4f, 0.2f, 0.1f, 0.10f}; break;
        case WorldEvent::InvertedGravity:   tint = {0.1f, 0.1f, 0.4f, 0.12f}; break;
        case WorldEvent::GiantCursorAttack: tint = {0, 0, 0, 0.05f}; break;
        case WorldEvent::WindowsUpdate:     tint = {0.05f, 0.4f, 0.7f, 0.20f}; break;
        case WorldEvent::LowFpsStorm:       tint = {0.3f, 0.3f, 0.3f, 0.15f}; break;
        default: break;
    }
    if (tint.a > 0.0f) {
        Draw::filledRect({0, 0, (float)Config::kWindowWidth, (float)Config::kWindowHeight}, tint);
    }

    // "Windows Update" gets its own loading-bar overlay, since it freezes
    // the whole city rather than just tinting the screen.
    if (ev.isFrozen()) {
        Draw::filledRect({0, 0, (float)Config::kWindowWidth, (float)Config::kWindowHeight}, {0.05f, 0.4f, 0.7f, 0.55f});
        Font::drawTextCentered("WINDOWS UPDATE", Config::kWindowWidth * 0.5f, Config::kWindowHeight * 0.5f - 40,
                                 3.0f, {1, 1, 1, 1});
        float barW = 400.0f;
        Draw::Rect barBg{Config::kWindowWidth * 0.5f - barW * 0.5f, Config::kWindowHeight * 0.5f,
                          Config::kWindowWidth * 0.5f + barW * 0.5f, Config::kWindowHeight * 0.5f + 24};
        Draw::filledRoundedRectApprox(barBg, 4.0f, {1, 1, 1, 0.3f});
        float fill = 1.0f - ev.progressRemaining();
        Draw::filledRoundedRectApprox({barBg.x0, barBg.y0, barBg.x0 + barW * fill, barBg.y1}, 4.0f, {1, 1, 1, 0.9f});
        Font::drawTextCentered("PLEASE DO NOT TURN OFF THE CITY", Config::kWindowWidth * 0.5f,
                                 Config::kWindowHeight * 0.5f + 40, 1.8f, {1, 1, 1, 0.85f});
    }
}

void renderInteractPrompt(GameState& game) {
    Glitch* nearest = game.nearestActiveGlitch();
    if (!nearest) return;
    Vec2 s = game.camera.worldToScreen(nearest->pos);
    game.hud.drawInteractPrompt(glitchHint(nearest->type), s);

    // label the algorithm above the hint, small and unobtrusive
    Font::drawTextCentered(glitchAlgorithmName(nearest->type), s.x, s.y - 58, 1.4f, {1, 1, 1, 0.7f});
}

void render(GLFWwindow* window, GameState& game) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    setupOrtho2D(w, h);

    if (game.screen == ScreenState::Title) {
        drawTitleScreen(game.globalTime);
        glfwSwapBuffers(window);
        return;
    }

    glClearColor(0.95f, 0.96f, 0.98f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderDistricts(game.camera);
    renderDistrictLabels(game.camera);

    for (auto& p : game.props) p.draw(game.camera, game.globalTime);
    for (auto& g : game.glitches) g.draw(game.camera);
    for (auto& n : game.npcs)     n.draw(game.camera, game.globalTime);
    game.cursor.draw(game.camera);
    game.player.draw(game.camera);

    for (auto& particle : game.particles.particles()) particle.draw(game.camera);

    renderInteractPrompt(game);
    game.dialogue.draw(game.camera);

    renderWorldEventOverlay(game.worldEvent);

    game.hud.drawTopBar(game.player.repairsCompleted, (int)game.glitches.size(), game.worldEvent);
    game.hud.drawDistrictBanner();
    drawMinimap(game.camera, game.player.pos, std::vector<Glitch>(game.glitches.begin(), game.glitches.end()));
    drawMilestoneBanner(game.player.repairsCompleted, game.globalTime);

    if (game.screen == ScreenState::Paused) {
        drawPauseOverlay(game.player.repairsCompleted, game.player.sessionTime, game.player.distanceTraveled);
    } else if (game.screen == ScreenState::Help) {
        drawHelpScreen();
    } else if (game.screen == ScreenState::GameStats) {
        Draw::filledRect({0, 0, (float)Config::kWindowWidth, (float)Config::kWindowHeight}, {0.05f, 0.05f, 0.08f, 0.9f});
        Font::drawTextCentered("SESSION STATS", Config::kWindowWidth * 0.5f, 80, 3.5f, {1, 0.85f, 0.3f, 1});
        std::string wrapped = Font::wordWrap(game.stats.summaryText(), 60);
        Font::drawParagraph(wrapped, Config::kWindowWidth * 0.5f - 220, 160, 2.0f, {1, 1, 1, 1}, 30.0f);
        Font::drawTextCentered("PRESS TAB TO RETURN", Config::kWindowWidth * 0.5f, Config::kWindowHeight - 60,
                                 1.8f, {0.7f, 0.7f, 0.8f, 1});
    }

    glfwSwapBuffers(window);
}

// ============================================================================
// SECTION 16 — INPUT HANDLING & GLFW CALLBACKS
// ============================================================================

// Simple "was this key just pressed this frame" edge-detection, needed for
// menu navigation (title screen, pause toggle, help toggle) so holding a
// key doesn't rapidly re-trigger a menu transition every frame.
struct KeyEdgeTracker {
    std::unordered_map<int, bool> previousState;

    bool pressedThisFrame(GLFWwindow* window, int key) {
        bool now = glfwGetKey(window, key) == GLFW_PRESS;
        bool was = previousState.count(key) ? previousState[key] : false;
        previousState[key] = now;
        return now && !was;
    }
};

static KeyEdgeTracker gKeyEdges;

void updateInputVelocity(GLFWwindow* window) {
    Vec2 v{0, 0};
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    v.y -= 1;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  v.y += 1;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  v.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) v.x += 1;
    gGame.player.velocity = v.normalized();
    gGame.player.running = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    gGame.repairKeyHeld = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ||
                          glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
}

void handleMenuInput(GLFWwindow* window) {
    switch (gGame.screen) {
        case ScreenState::Title:
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_ENTER)) {
                gGame.screen = ScreenState::Playing;
                gGame.stats.worldEntryTime = gGame.globalTime;
            }
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_H)) {
                gGame.screenBeforeHelp = ScreenState::Title;
                gGame.screen = ScreenState::Help;
            }
            break;
        case ScreenState::Playing:
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_ESCAPE)) {
                gGame.screen = ScreenState::Paused;
            }
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_TAB)) {
                gGame.screen = ScreenState::GameStats;
            }
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_H)) {
                gGame.screenBeforeHelp = ScreenState::Playing;
                gGame.screen = ScreenState::Help;
            }
            break;
        case ScreenState::Paused:
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_ESCAPE)) {
                gGame.screen = ScreenState::Playing;
            }
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_H)) {
                gGame.screenBeforeHelp = ScreenState::Paused;
                gGame.screen = ScreenState::Help;
            }
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_Q)) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            break;
        case ScreenState::Help:
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_H) ||
                gKeyEdges.pressedThisFrame(window, GLFW_KEY_ESCAPE)) {
                gGame.screen = gGame.screenBeforeHelp;
            }
            break;
        case ScreenState::GameStats:
            if (gKeyEdges.pressedThisFrame(window, GLFW_KEY_TAB) ||
                gKeyEdges.pressedThisFrame(window, GLFW_KEY_ESCAPE)) {
                gGame.screen = ScreenState::Playing;
            }
            break;
    }
}

void errorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW Error [%d]: %s\n", error, description);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    setupOrtho2D(width, height);
}

// ============================================================================
// SECTION 17 — main()
// ============================================================================

int main() {
    std::srand((unsigned)std::time(nullptr));

    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW.\n");
        return -1;
    }

    // Use a compatibility-friendly context since this file relies on
    // immediate-mode OpenGL for simplicity and readability, matching the
    // typical setup used in introductory computer graphics courses.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(
        Config::kWindowWidth, Config::kWindowHeight,
        "Pixel Panic: Saving Reality.exe", nullptr, nullptr);

    if (!window) {
        std::fprintf(stderr, "Failed to create GLFW window.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // vsync
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    gGame.init();
    std::printf("=== PIXEL PANIC: SAVING REALITY.exe ===\n");
    std::printf("Move: WASD / Arrow keys | Run: Shift | Repair nearby glitch: hold E or Space\n");
    std::printf("Pause: Escape | Help: H | Stats: Tab\n\n");

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = (float)(now - lastTime);
        lastTime = now;
        dt = std::min(dt, 0.05f);  // clamp to avoid huge steps on stalls

        glfwPollEvents();

        handleMenuInput(window);
        updateInputVelocity(window);

        if (gGame.screen == ScreenState::Playing) {
            gGame.updatePlaying(dt);
        } else if (gGame.screen == ScreenState::Title) {
            gGame.globalTime += dt;  // keep title screen background animated
        }

        District* here = gGame.districtAt(gGame.player.pos);
        std::string title = "Pixel Panic: Saving Reality.exe";
        if (gGame.screen == ScreenState::Playing && here) title += "  -  " + here->name;
        title += "  |  Repairs: " + std::to_string(gGame.player.repairsCompleted);
        glfwSetWindowTitle(window, title.c_str());

        render(window, gGame);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
