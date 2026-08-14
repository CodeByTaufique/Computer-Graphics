//
// Created by  TauFique Hassan on 7/19/26.
//
/*============================================================================
    DHAKA METRO RAIL SMART STATION SIMULATOR
    ----------------------------------------------------------------------
    A 2D interactive OpenGL/FreeGLUT simulation inspired by Dhaka MRT
    Line-6, built for a Computer Graphics course project.

    Course topics demonstrated:
        - DDA Line Algorithm          (rail tracks, platform markings)
        - Bresenham Line Algorithm    (overhead cables, bridge supports)
        - Midpoint Circle Algorithm   (wheels, signal lamps, clock)
        - Bezier Curves               (curved track entry/exit)
        - 2D Geometric Transformations (translation, rotation, scaling)
        - RGB Color Model             (day/night, signals, weather)
        - Frame based Animation       (train, passengers, rain, traffic)
        - Simple Clipping             (viewport bounds culling)

    Platform : C++17, OpenGL 1.x/2.x fixed pipeline, FreeGLUT, GLEW
    Build    : g++ dhaka_metro_simulator.cpp -o metro -lfreeglut -lglew32 -lopengl32   (Windows/MinGW)
               g++ dhaka_metro_simulator.cpp -o metro -lglut -lGLEW -lGL -lGLU         (Linux)

    Team division (see header comments in each section):
        Member 1 - Metro Train System            [SECTION: TRAIN]
        Member 2 - Station Infrastructure         [SECTION: STATION]
        Member 3 - Passenger & Traffic Simulation [SECTION: PASSENGERS, TRAFFIC]
        Member 4 - Environment & Effects          [SECTION: ENVIRONMENT, HUD]
============================================================================*/

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>
#include <string>
#include <deque>
#include <algorithm>

/*============================================================================
    SECTION 0: GLOBAL CONSTANTS & CONFIGURATION
============================================================================*/
namespace Cfg {
    const int   WINDOW_W = 1280;
    const int   WINDOW_H = 720;
    const double WORLD_W = 1280.0;
    const double WORLD_H = 720.0;

    // Vertical layout bands (y coordinates, 0 = bottom of window)
    const double SKY_TOP           = 720.0;
    const double SKY_BOTTOM        = 560.0;
    const double BRIDGE_TOP        = 560.0;
    const double BRIDGE_BOTTOM     = 520.0;
    const double TRACK_Y           = 500.0;
    const double PLATFORM_TOP      = 480.0;
    const double PLATFORM_BOTTOM   = 360.0;
    const double CONCOURSE_TOP     = 360.0;
    const double CONCOURSE_BOTTOM  = 240.0;
    const double ROAD_TOP          = 150.0;
    const double ROAD_BOTTOM       = 60.0;
    const double GROUND_BOTTOM     = 0.0;

    const int STATION_COUNT = 5;
    const char* STATION_NAMES_EN[STATION_COUNT] = {
        "Uttara North", "Agargaon", "Farmgate", "Shahbagh", "Motijheel"
    };
    const char* STATION_NAMES_BN[STATION_COUNT] = {
        "Uttara North", "Agargaon", "Farmgate", "Shahbagh", "Motijheel"
    };
}

/*============================================================================
    SECTION 0.1: SMALL MATH / DRAWING PRIMITIVES
    (DDA line, Bresenham line, Midpoint circle - implemented explicitly to
     satisfy the Computer Graphics syllabus requirement instead of relying
     purely on glVertex convenience calls for these primitives.)
============================================================================*/
struct RGB { float r, g, b; };

inline void setColor(const RGB& c, float a = 1.0f) {
    glColor4f(c.r, c.g, c.b, a);
}

// ---- DDA Line Algorithm ----
void ddaLine(double x0, double y0, double x1, double y1) {
    double dx = x1 - x0, dy = y1 - y0;
    int steps = (int)std::max(std::fabs(dx), std::fabs(dy));
    if (steps == 0) { glBegin(GL_POINTS); glVertex2d(x0, y0); glEnd(); return; }
    double xInc = dx / steps, yInc = dy / steps;
    double x = x0, y = y0;
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2d(x, y);
        x += xInc; y += yInc;
    }
    glEnd();
}

// ---- Bresenham Line Algorithm ----
void bresenhamLine(int x0, int y0, int x1, int y1) {
    int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
    glEnd();
}

// ---- Midpoint Circle Algorithm ----
void midpointCircle(double cx, double cy, int r, bool filled = false) {
    int x = 0, y = r;
    int d = 1 - r;
    glBegin(filled ? GL_POINTS : GL_POINTS);
    auto plot8 = [&](int px, int py) {
        if (filled) {
            // horizontal scan fill using symmetry
            glVertex2d(cx - px, cy + py); glVertex2d(cx + px, cy + py);
            glVertex2d(cx - px, cy - py); glVertex2d(cx + px, cy - py);
            glVertex2d(cx - py, cy + px); glVertex2d(cx + py, cy + px);
            glVertex2d(cx - py, cy - px); glVertex2d(cx + py, cy - px);
        } else {
            glVertex2d(cx + px, cy + py); glVertex2d(cx - px, cy + py);
            glVertex2d(cx + px, cy - py); glVertex2d(cx - px, cy - py);
            glVertex2d(cx + py, cy + px); glVertex2d(cx - py, cy + px);
            glVertex2d(cx + py, cy - px); glVertex2d(cx - py, cy - px);
        }
    };
    while (x <= y) {
        plot8(x, y);
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; y--; }
        x++;
    }
    glEnd();
    if (filled) {
        // fill interior with triangle fan (fast, visually solid)
        glBegin(GL_TRIANGLE_FAN);
        glVertex2d(cx, cy);
        for (int a = 0; a <= 360; a += 6) {
            double rad = a * 3.14159265 / 180.0;
            glVertex2d(cx + r * cos(rad), cy + r * sin(rad));
        }
        glEnd();
    }
}

// ---- Filled rectangle helper (transformations applied by caller) ----
void filledRect(double x, double y, double w, double h) {
    glBegin(GL_QUADS);
    glVertex2d(x, y);
    glVertex2d(x + w, y);
    glVertex2d(x + w, y + h);
    glVertex2d(x, y + h);
    glEnd();
}

void outlineRect(double x, double y, double w, double h) {
    glBegin(GL_LINE_LOOP);
    glVertex2d(x, y);
    glVertex2d(x + w, y);
    glVertex2d(x + w, y + h);
    glVertex2d(x, y + h);
    glEnd();
}

// ---- Cubic Bezier curve (used for curved track entry/exit & road ramps) ----
void bezierCurve(double x0, double y0, double x1, double y1,
                  double x2, double y2, double x3, double y3, int segments = 40) {
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; i++) {
        double t = (double)i / segments;
        double u = 1 - t;
        double x = u*u*u*x0 + 3*u*u*t*x1 + 3*u*t*t*x2 + t*t*t*x3;
        double y = u*u*u*y0 + 3*u*u*t*y1 + 3*u*t*t*y2 + t*t*t*y3;
        glVertex2d(x, y);
    }
    glEnd();
}

// ---- Simple text rendering (bitmap fonts via FreeGLUT) ----
void drawText(double x, double y, const std::string& text,
              void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2d(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

/*============================================================================
    SECTION 1: GLOBAL SIMULATION STATE
============================================================================*/
enum class TimeOfDay { MORNING, DAY, EVENING, NIGHT };
enum class WeatherType { CLEAR, RAIN, HEAVY_RAIN, FOG };
enum class SignalState { GREEN, YELLOW, RED, EMERGENCY_FLASH };
enum class TrainState { APPROACHING, BRAKING, STOPPED, DOORS_OPEN, DOORS_CLOSING, DEPARTING, OFFSCREEN };

struct SimState {
    // time
    TimeOfDay   timeOfDay      = TimeOfDay::DAY;
    float       clockHours     = 9.0f;     // 0-24 simulated station clock
    float       simSeconds     = 0.0f;     // accumulated seconds since launch

    // weather
    WeatherType weather        = WeatherType::CLEAR;

    // signal
    SignalState signal         = SignalState::GREEN;
    float       signalBlinkTimer = 0.0f;
    bool        signalBlinkOn  = true;

    // train
    TrainState  trainState     = TrainState::APPROACHING;
    double      trainX         = -400.0;
    double      trainSpeed     = 0.0;
    bool        doorsOpen      = false;
    float       doorOpenAmount = 0.0f; // 0..1
    int         currentStationIdx = 1; // Agargaon
    float       dwellTimer     = 0.0f;
    bool        emergencyStop  = false;
    bool        hornActive     = false;
    float       hornTimer      = 0.0f;

    // display / hud
    int         passengerCountBoarded  = 0;
    int         passengerCountOnPlatform = 0;
    bool        paused = false;
    float       fps = 60.0f;

    // emergency events
    bool        powerOutage = false;
    bool        fireAlarm = false;
    float       eventMessageTimer = 0.0f;
    std::string eventMessage;

    // arrival countdown for display
    float       nextTrainEtaSeconds = 15.0f;
} G;

// forward decls used across sections
void pushEvent(const std::string& msg, float seconds = 4.0f);

RGB lerpColor(const RGB& a, const RGB& b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
}

/*============================================================================
    SECTION 2: ENVIRONMENT — SKY, DAY/NIGHT CYCLE, WEATHER
    (Member 4 — Environment & Effects)
============================================================================*/
namespace Env {

    RGB skyColorFor(TimeOfDay t) {
        switch (t) {
            case TimeOfDay::MORNING: return {1.00f, 0.75f, 0.55f};
            case TimeOfDay::DAY:     return {0.53f, 0.80f, 0.98f};
            case TimeOfDay::EVENING: return {0.95f, 0.45f, 0.35f};
            case TimeOfDay::NIGHT:   return {0.05f, 0.07f, 0.20f};
        }
        return {0.5f, 0.7f, 0.9f};
    }

    void drawSky() {
        RGB top = skyColorFor(G.timeOfDay);
        RGB bottom = lerpColor(top, {1,1,1}, 0.25f);
        glBegin(GL_QUADS);
        setColor(top);
        glVertex2d(0, Cfg::WORLD_H);
        glVertex2d(Cfg::WORLD_W, Cfg::WORLD_H);
        setColor(bottom);
        glVertex2d(Cfg::WORLD_W, Cfg::SKY_BOTTOM - 20);
        glVertex2d(0, Cfg::SKY_BOTTOM - 20);
        glEnd();
    }

    void drawSun() {
        if (G.timeOfDay == TimeOfDay::NIGHT) return;
        double cx = 1080, cy = 640;
        setColor({1.0f, 0.95f, 0.4f});
        midpointCircle(cx, cy, 34, true);
        setColor({1.0f, 1.0f, 0.7f});
        for (int i = 0; i < 8; i++) {
            double ang = i * 3.14159265 / 4.0;
            double x1 = cx + 40 * cos(ang), y1 = cy + 40 * sin(ang);
            double x2 = cx + 55 * cos(ang), y2 = cy + 55 * sin(ang);
            bresenhamLine((int)x1, (int)y1, (int)x2, (int)y2);
        }
    }

    void drawMoon() {
        if (G.timeOfDay != TimeOfDay::NIGHT) return;
        double cx = 1080, cy = 640;
        setColor({0.95f, 0.95f, 0.85f});
        midpointCircle(cx, cy, 28, true);
        setColor(skyColorFor(TimeOfDay::NIGHT));
        midpointCircle(cx + 12, cy + 4, 24, true);
        // stars
        srand(42);
        setColor({1,1,1});
        glBegin(GL_POINTS);
        for (int i = 0; i < 60; i++) {
            double sx = (rand() % (int)Cfg::WORLD_W);
            double sy = 580 + (rand() % 130);
            glVertex2d(sx, sy);
        }
        glEnd();
    }

    struct Cloud { double x, y, speed; float scale; };
    std::vector<Cloud> clouds = {
        {150, 660, 6.0, 1.0f}, {520, 690, 4.5, 1.3f}, {860, 650, 5.5, 0.8f}, {1150, 675, 5.0, 1.1f}
    };

    void drawCloudShape(double x, double y, float scale) {
        RGB c = (G.timeOfDay == TimeOfDay::NIGHT) ? RGB{0.5f,0.5f,0.6f} : RGB{1,1,1};
        setColor(c, 0.9f);
        midpointCircle(x, y, (int)(18*scale), true);
        midpointCircle(x + 20*scale, y + 6*scale, (int)(22*scale), true);
        midpointCircle(x + 42*scale, y, (int)(16*scale), true);
    }

    void updateClouds(float dt) {
        for (auto& c : clouds) {
            c.x += c.speed * dt;
            if (c.x > Cfg::WORLD_W + 60) c.x = -60;
        }
    }

    void drawClouds() {
        for (auto& c : clouds) drawCloudShape(c.x, c.y, c.scale);
    }

    // Birds - simple animated 'M' shapes
    struct Bird { double x, y; float phase; };
    std::vector<Bird> birds = { {300, 700, 0}, {330, 690, 1}, {360, 705, 2} };

    void updateBirds(float dt) {
        for (auto& b : birds) {
            b.x += 25.0f * dt;
            b.phase += dt * 6.0f;
            if (b.x > Cfg::WORLD_W + 20) b.x = -20;
        }
    }

    void drawBirds() {
        if (G.timeOfDay == TimeOfDay::NIGHT) return;
        setColor({0.15f, 0.15f, 0.15f});
        for (auto& b : birds) {
            float flap = (float)sin(b.phase) * 6.0f;
            glBegin(GL_LINE_STRIP);
            glVertex2d(b.x - 10, b.y + flap);
            glVertex2d(b.x, b.y);
            glVertex2d(b.x + 10, b.y + flap);
            glEnd();
        }
    }

    // ---- Rain ----
    struct RainDrop { double x, y, speed; };
    std::vector<RainDrop> raindrops;
    bool rainInit = false;

    void initRain() {
        raindrops.clear();
        for (int i = 0; i < 400; i++) {
            RainDrop d;
            d.x = rand() % (int)Cfg::WORLD_W;
            d.y = rand() % (int)Cfg::WORLD_H;
            d.speed = 300 + rand() % 200;
            raindrops.push_back(d);
        }
        rainInit = true;
    }

    void updateRain(float dt) {
        if (G.weather != WeatherType::RAIN && G.weather != WeatherType::HEAVY_RAIN) return;
        if (!rainInit) initRain();
        float mult = (G.weather == WeatherType::HEAVY_RAIN) ? 1.8f : 1.0f;
        for (auto& d : raindrops) {
            d.y -= d.speed * mult * dt;
            d.x -= 40 * mult * dt; // wind slant
            if (d.y < 0) { d.y = Cfg::WORLD_H; d.x = rand() % (int)Cfg::WORLD_W; }
            if (d.x < 0) d.x = Cfg::WORLD_W;
        }
    }

    void drawRain() {
        if (G.weather != WeatherType::RAIN && G.weather != WeatherType::HEAVY_RAIN) return;
        setColor({0.7f, 0.8f, 1.0f}, 0.6f);
        glLineWidth(1.0f);
        int count = (G.weather == WeatherType::HEAVY_RAIN) ? (int)raindrops.size() : (int)raindrops.size()/2;
        glBegin(GL_LINES);
        for (int i = 0; i < count; i++) {
            auto& d = raindrops[i];
            glVertex2d(d.x, d.y);
            glVertex2d(d.x - 6, d.y - 18);
        }
        glEnd();
    }

    // ---- Lightning (heavy rain only, occasional flash) ----
    float lightningTimer = 0.0f;
    bool  lightningFlash = false;

    void updateLightning(float dt) {
        if (G.weather != WeatherType::HEAVY_RAIN) { lightningFlash = false; return; }
        lightningTimer -= dt;
        if (lightningTimer <= 0.0f) {
            lightningFlash = (rand() % 100) < 8;
            lightningTimer = 0.4f;
        }
    }

    void drawLightningOverlay() {
        if (!lightningFlash) return;
        setColor({1,1,1}, 0.25f);
        filledRect(0, 0, Cfg::WORLD_W, Cfg::WORLD_H);
    }

    // ---- Fog ----
    void drawFog() {
        if (G.weather != WeatherType::FOG) return;
        setColor({0.85f, 0.85f, 0.88f}, 0.35f);
        filledRect(0, Cfg::PLATFORM_BOTTOM - 40, Cfg::WORLD_W, 260);
    }

    // ---- Wet platform reflection strip ----
    void drawWetReflection() {
        if (G.weather != WeatherType::RAIN && G.weather != WeatherType::HEAVY_RAIN) return;
        setColor({0.6f, 0.7f, 0.9f}, 0.15f);
        filledRect(0, Cfg::PLATFORM_BOTTOM, Cfg::WORLD_W, 6);
    }

    void updateTimeOfDay(float dt) {
        G.clockHours += dt * 0.02f; // slow simulated day
        if (G.clockHours >= 24.0f) G.clockHours -= 24.0f;
        if (G.clockHours >= 5 && G.clockHours < 8) G.timeOfDay = TimeOfDay::MORNING;
        else if (G.clockHours >= 8 && G.clockHours < 17) G.timeOfDay = TimeOfDay::DAY;
        else if (G.clockHours >= 17 && G.clockHours < 20) G.timeOfDay = TimeOfDay::EVENING;
        else G.timeOfDay = TimeOfDay::NIGHT;
    }

    std::string clockString() {
        int h = (int)G.clockHours;
        int m = (int)((G.clockHours - h) * 60);
        char buf[16];
        int displayH = h % 12; if (displayH == 0) displayH = 12;
        snprintf(buf, sizeof(buf), "%02d:%02d %s", displayH, m, (h < 12 ? "AM" : "PM"));
        return std::string(buf);
    }
}

/*============================================================================
    SECTION 3: ELEVATED BRIDGE & TRACK STRUCTURE
    (Member 2 — Station Infrastructure)
============================================================================*/
namespace Bridge {

    void drawSupportPillars() {
        setColor({0.55f, 0.55f, 0.58f});
        for (double x = 40; x < Cfg::WORLD_W; x += 220) {
            filledRect(x, Cfg::ROAD_BOTTOM, 22, Cfg::BRIDGE_BOTTOM - Cfg::ROAD_BOTTOM);
            setColor({0.35f, 0.35f, 0.38f});
            outlineRect(x, Cfg::ROAD_BOTTOM, 22, Cfg::BRIDGE_BOTTOM - Cfg::ROAD_BOTTOM);
            setColor({0.55f, 0.55f, 0.58f});
        }
    }

    void drawDeck() {
        setColor({0.60f, 0.60f, 0.64f});
        filledRect(0, Cfg::BRIDGE_BOTTOM, Cfg::WORLD_W, Cfg::BRIDGE_TOP - Cfg::BRIDGE_BOTTOM);
        setColor({0.40f, 0.40f, 0.44f});
        outlineRect(0, Cfg::BRIDGE_BOTTOM, Cfg::WORLD_W, Cfg::BRIDGE_TOP - Cfg::BRIDGE_BOTTOM);

        // Bresenham-drawn diagonal truss pattern along the underside
        setColor({0.30f, 0.30f, 0.34f});
        for (int x = 0; x < (int)Cfg::WORLD_W; x += 40) {
            bresenhamLine(x, (int)Cfg::BRIDGE_BOTTOM, x + 40, (int)Cfg::BRIDGE_TOP);
            bresenhamLine(x + 40, (int)Cfg::BRIDGE_BOTTOM, x, (int)Cfg::BRIDGE_TOP);
        }
    }

    void drawOverheadCables() {
        setColor({0.2f, 0.2f, 0.2f});
        for (int x = 0; x < (int)Cfg::WORLD_W; x += 6) {
            bresenhamLine(x, (int)Cfg::TRACK_Y + 20, x, (int)Cfg::TRACK_Y + 22);
        }
        // catenary cable using DDA for a slight sag
        setColor({0.15f, 0.15f, 0.15f});
        ddaLine(0, Cfg::TRACK_Y + 24, Cfg::WORLD_W, Cfg::TRACK_Y + 24);
    }

    void drawRailTrack() {
        setColor({0.25f, 0.25f, 0.27f});
        filledRect(0, Cfg::TRACK_Y - 6, Cfg::WORLD_W, 10);
        setColor({0.75f, 0.75f, 0.78f});
        // rails via DDA (two parallel lines)
        ddaLine(0, Cfg::TRACK_Y - 2, Cfg::WORLD_W, Cfg::TRACK_Y - 2);
        ddaLine(0, Cfg::TRACK_Y + 2, Cfg::WORLD_W, Cfg::TRACK_Y + 2);
        // sleepers
        setColor({0.35f, 0.25f, 0.15f});
        for (int x = 0; x < (int)Cfg::WORLD_W; x += 26) {
            filledRect(x, Cfg::TRACK_Y - 8, 8, 14);
        }
    }

    void drawCurvedEntryExit() {
        // Bezier curves suggesting the track curving away at both ends
        setColor({0.75f, 0.75f, 0.78f});
        bezierCurve(0, Cfg::TRACK_Y - 2, 40, Cfg::TRACK_Y + 30, 20, Cfg::TRACK_Y + 60, 60, Cfg::TRACK_Y + 90);
        bezierCurve(Cfg::WORLD_W, Cfg::TRACK_Y - 2, Cfg::WORLD_W - 40, Cfg::TRACK_Y + 30,
                     Cfg::WORLD_W - 20, Cfg::TRACK_Y + 60, Cfg::WORLD_W - 60, Cfg::TRACK_Y + 90);
    }

    void draw() {
        drawSupportPillars();
        drawDeck();
        drawRailTrack();
        drawOverheadCables();
        drawCurvedEntryExit();
    }
}

/*============================================================================
    SECTION 4: METRO TRAIN SYSTEM
    (Member 1 — Metro Train System)
    Implements a finite state machine:
      APPROACHING -> BRAKING -> STOPPED -> DOORS_OPEN -> DOORS_CLOSING
      -> DEPARTING -> OFFSCREEN -> (reset) APPROACHING
============================================================================*/
namespace Train {

    const double CAR_W = 170.0;
    const int    CAR_COUNT = 4;
    const double TRAIN_LEN = CAR_W * CAR_COUNT;
    const double STOP_X = 420.0; // x where train front aligns for boarding

    double frontX() { return G.trainX + TRAIN_LEN; }

    void resetToApproaching() {
        G.trainState = TrainState::APPROACHING;
        G.trainX = -TRAIN_LEN - 40;
        G.trainSpeed = 140.0;
        G.doorsOpen = false;
        G.doorOpenAmount = 0.0f;
        G.dwellTimer = 0.0f;
    }

    void triggerHorn() {
        G.hornActive = true;
        G.hornTimer = 1.2f;
    }

    void openDoors() {
        if (G.trainState == TrainState::STOPPED) {
            G.trainState = TrainState::DOORS_OPEN;
            G.doorsOpen = true;
            G.dwellTimer = 0.0f;
        }
    }

    void closeDoors() {
        if (G.trainState == TrainState::DOORS_OPEN) {
            G.trainState = TrainState::DOORS_CLOSING;
        }
    }

    void emergencyStopToggle() {
        G.emergencyStop = !G.emergencyStop;
        if (G.emergencyStop) {
            pushEvent("EMERGENCY STOP ACTIVATED", 3.0f);
            G.signal = SignalState::EMERGENCY_FLASH;
        } else {
            pushEvent("Emergency stop released", 2.5f);
            G.signal = SignalState::GREEN;
        }
    }

    void update(float dt) {
        if (G.paused) return;

        if (G.emergencyStop) {
            G.trainSpeed = std::max(0.0, G.trainSpeed - 400.0 * dt);
            return;
        }

        switch (G.trainState) {
            case TrainState::APPROACHING: {
                G.trainX += G.trainSpeed * dt;
                if (frontX() >= STOP_X - 260) {
                    G.trainState = TrainState::BRAKING;
                }
                if (G.trainX > -TRAIN_LEN - 200 && !G.hornActive && G.trainX < -TRAIN_LEN + 20) {
                    triggerHorn();
                }
                break;
            }
            case TrainState::BRAKING: {
                double dist = STOP_X - frontX();
                double decel = 90.0;
                G.trainSpeed = std::max(20.0, G.trainSpeed - decel * dt);
                G.trainX += G.trainSpeed * dt;
                if (frontX() >= STOP_X) {
                    G.trainX = STOP_X - TRAIN_LEN;
                    G.trainSpeed = 0.0;
                    G.trainState = TrainState::STOPPED;
                    G.dwellTimer = 0.0f;
                    G.currentStationIdx = (G.currentStationIdx + 1) % Cfg::STATION_COUNT;
                }
                break;
            }
            case TrainState::STOPPED: {
                G.dwellTimer += dt;
                if (G.dwellTimer > 1.2f) {
                    openDoors();
                }
                break;
            }
            case TrainState::DOORS_OPEN: {
                G.doorOpenAmount = std::min(1.0f, G.doorOpenAmount + dt * 2.0f);
                G.dwellTimer += dt;
                if (G.dwellTimer > 6.0f) {
                    closeDoors();
                }
                break;
            }
            case TrainState::DOORS_CLOSING: {
                G.doorOpenAmount = std::max(0.0f, G.doorOpenAmount - dt * 2.5f);
                if (G.doorOpenAmount <= 0.0f) {
                    G.doorsOpen = false;
                    G.trainState = TrainState::DEPARTING;
                    triggerHorn();
                }
                break;
            }
            case TrainState::DEPARTING: {
                G.trainSpeed = std::min(180.0, G.trainSpeed + 70.0 * dt);
                G.trainX += G.trainSpeed * dt;
                if (G.trainX > Cfg::WORLD_W + 40) {
                    G.trainState = TrainState::OFFSCREEN;
                }
                break;
            }
            case TrainState::OFFSCREEN: {
                G.nextTrainEtaSeconds = 12.0f + rand() % 8;
                resetToApproaching();
                break;
            }
        }

        if (G.hornActive) {
            G.hornTimer -= dt;
            if (G.hornTimer <= 0) G.hornActive = false;
        }
    }

    void drawWheel(double x, double y, float rotationDeg) {
        setColor({0.1f, 0.1f, 0.1f});
        midpointCircle(x, y, 12, true);
        glPushMatrix();
        glTranslated(x, y, 0);
        glRotatef(rotationDeg, 0, 0, 1);
        setColor({0.7f, 0.7f, 0.7f});
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2d(-10, 0); glVertex2d(10, 0);
        glVertex2d(0, -10); glVertex2d(0, 10);
        glEnd();
        glPopMatrix();
    }

    void drawCar(double x, int carIndex, float wheelRot) {
        bool night = (G.timeOfDay == TimeOfDay::NIGHT || G.timeOfDay == TimeOfDay::EVENING);
        double y = Cfg::TRACK_Y + 4;
        double h = 62;

        // body
        setColor({0.85f, 0.15f, 0.20f});
        filledRect(x, y, CAR_W - 6, h);
        setColor({1.0f, 1.0f, 1.0f});
        filledRect(x, y, CAR_W - 6, 12);
        setColor({0.6f, 0.05f, 0.10f});
        outlineRect(x, y, CAR_W - 6, h);

        // interior lights glow through windows at night
        int winCount = 5;
        double winW = (CAR_W - 30) / winCount;
        for (int i = 0; i < winCount; i++) {
            double wx = x + 12 + i * winW;
            if (night) setColor({1.0f, 0.95f, 0.6f}, 0.9f);
            else setColor({0.55f, 0.75f, 0.90f}, 0.85f);
            filledRect(wx, y + 30, winW - 6, 20);
        }

        // door(s) - one per car, opens with G.doorOpenAmount
        double doorX = x + CAR_W/2 - 12;
        double gap = 22.0 * G.doorOpenAmount;
        setColor({0.9f, 0.9f, 0.92f});
        filledRect(doorX - gap - 10, y + 2, 10, h - 4);
        filledRect(doorX + gap + 2, y + 2, 10, h - 4);

        // wheels
        drawWheel(x + 24, y - 6, wheelRot);
        drawWheel(x + CAR_W - 40, y - 6, wheelRot);

        // headlights on the very first car's front only (handled by caller)
        // car number label
        setColor({1,1,1});
        char buf[8]; snprintf(buf, sizeof(buf), "L6");
        drawText(x + CAR_W/2 - 8, y + h - 10, buf, GLUT_BITMAP_HELVETICA_10);
    }

    void drawHeadlights(bool front) {
        bool night = (G.timeOfDay == TimeOfDay::NIGHT || G.timeOfDay == TimeOfDay::EVENING);
        if (!night) return;
        double x = front ? frontX() : G.trainX;
        double y = Cfg::TRACK_Y + 30;
        setColor({1.0f, 1.0f, 0.75f}, 0.85f);
        midpointCircle(x, y, 5, true);
        // light cone
        setColor({1.0f, 1.0f, 0.7f}, 0.15f);
        glBegin(GL_TRIANGLES);
        glVertex2d(x, y);
        glVertex2d(x + (front ? 60 : -60), y + 18);
        glVertex2d(x + (front ? 60 : -60), y - 18);
        glEnd();
    }

    void drawHornIndicator() {
        if (!G.hornActive) return;
        setColor({1.0f, 0.9f, 0.2f});
        drawText(frontX() - 30, Cfg::TRACK_Y + 70, "TUUT!", GLUT_BITMAP_HELVETICA_18);
    }

    void draw() {
        if (G.trainState == TrainState::OFFSCREEN) return;
        float wheelRot = (float)fmod(G.simSeconds * (G.trainSpeed > 0 ? 220.0 : 0.0), 360.0);
        for (int i = 0; i < CAR_COUNT; i++) {
            drawCar(G.trainX + i * CAR_W, i, wheelRot);
        }
        drawHeadlights(true);
        drawHornIndicator();
    }
}

/*============================================================================
    SECTION 5: SIGNAL SYSTEM
    (Member 1 — Metro Train System, signal side)
============================================================================*/
namespace Signal {

    void cycle() {
        switch (G.signal) {
            case SignalState::GREEN:  G.signal = SignalState::YELLOW; break;
            case SignalState::YELLOW: G.signal = SignalState::RED; break;
            case SignalState::RED:    G.signal = SignalState::GREEN; break;
            case SignalState::EMERGENCY_FLASH: break; // controlled separately
        }
    }

    void update(float dt) {
        G.signalBlinkTimer += dt;
        if (G.signalBlinkTimer > 0.4f) {
            G.signalBlinkTimer = 0.0f;
            G.signalBlinkOn = !G.signalBlinkOn;
        }
    }

    void drawPost(double x, double y) {
        setColor({0.2f, 0.2f, 0.2f});
        filledRect(x - 3, y, 6, 60);

        // housing
        setColor({0.1f, 0.1f, 0.1f});
        filledRect(x - 12, y + 60, 24, 66);

        RGB dim = {0.25f, 0.05f, 0.05f};
        RGB red = {1.0f, 0.1f, 0.1f};
        RGB yellow = {1.0f, 0.85f, 0.1f};
        RGB green = {0.1f, 0.9f, 0.2f};

        bool showRed = (G.signal == SignalState::RED) ||
                        (G.signal == SignalState::EMERGENCY_FLASH && G.signalBlinkOn);
        bool showYellow = (G.signal == SignalState::YELLOW);
        bool showGreen = (G.signal == SignalState::GREEN);

        setColor(showRed ? red : dim);
        midpointCircle(x, y + 108, 8, true);
        setColor(showYellow ? yellow : RGB{0.25f,0.22f,0.05f});
        midpointCircle(x, y + 88, 8, true);
        setColor(showGreen ? green : RGB{0.05f,0.2f,0.08f});
        midpointCircle(x, y + 68, 8, true);
    }

    void draw() {
        drawPost(60, Cfg::TRACK_Y + 10);
        drawPost(Cfg::WORLD_W - 60, Cfg::TRACK_Y + 10);
    }
}

/*============================================================================
    SECTION 6: STATION INFRASTRUCTURE
    (Member 2 — Station Infrastructure)
============================================================================*/
namespace Station {

    void drawPlatformFloor() {
        setColor({0.72f, 0.70f, 0.68f});
        filledRect(0, Cfg::PLATFORM_BOTTOM, Cfg::WORLD_W, Cfg::PLATFORM_TOP - Cfg::PLATFORM_BOTTOM);

        // Safety line (yellow, DDA)
        setColor({1.0f, 0.85f, 0.0f});
        glLineWidth(3.0f);
        ddaLine(0, Cfg::PLATFORM_TOP - 14, Cfg::WORLD_W, Cfg::PLATFORM_TOP - 14);
        glLineWidth(1.0f);

        // tactile platform markings (tile pattern)
        setColor({0.55f, 0.53f, 0.50f});
        for (int x = 0; x < (int)Cfg::WORLD_W; x += 40) {
            ddaLine(x, Cfg::PLATFORM_BOTTOM, x, Cfg::PLATFORM_TOP);
        }
    }

    void drawBench(double x, double y) {
        setColor({0.45f, 0.30f, 0.18f});
        filledRect(x, y, 46, 6);
        filledRect(x, y + 16, 46, 6);
        setColor({0.25f, 0.25f, 0.25f});
        filledRect(x + 2, y, 4, 16);
        filledRect(x + 40, y, 4, 16);
    }

    void drawTicketCounter(double x, double y) {
        setColor({0.30f, 0.35f, 0.55f});
        filledRect(x, y, 90, 40);
        setColor({0.85f, 0.85f, 0.9f});
        filledRect(x + 8, y + 22, 74, 12);
        setColor({0.1f,0.1f,0.1f});
        drawText(x + 6, y + 46, "TICKET COUNTER", GLUT_BITMAP_HELVETICA_10);
        // clerk (simple figure)
        setColor({0.9f, 0.75f, 0.6f});
        midpointCircle(x + 45, y + 52, 8, true);
        setColor({0.2f, 0.4f, 0.7f});
        filledRect(x + 35, y + 30, 20, 20);
    }

    void drawSmartGate(double x, double y, bool open, float scanPulse) {
        setColor({0.75f, 0.78f, 0.82f});
        filledRect(x, y, 10, 46);
        filledRect(x + 34, y, 10, 46);
        setColor(open ? RGB{0.1f, 0.9f, 0.2f} : RGB{0.9f, 0.15f, 0.15f});
        midpointCircle(x + 22, y + 52, 5, true);
        // card scan pad pulse
        setColor({0.2f, 0.6f, 1.0f}, 0.4f + 0.3f*scanPulse);
        filledRect(x + 12, y + 20, 20, 8);
        setColor({0.4f,0.4f,0.4f});
        outlineRect(x, y, 44, 46);
    }

    void drawEscalator(double x, double y) {
        setColor({0.5f, 0.5f, 0.55f});
        for (int i = 0; i < 8; i++) {
            filledRect(x + i*7, y + i*4, 20, 8);
        }
        setColor({0.3f,0.3f,0.3f});
        outlineRect(x, y, 20 + 7*7, 8 + 4*7 + 8);
    }

    void drawLift(double x, double y) {
        setColor({0.65f, 0.65f, 0.70f});
        filledRect(x, y, 40, 70);
        setColor({0.85f, 0.85f, 0.9f});
        filledRect(x + 4, y + 4, 32, 62);
        setColor({0.2f,0.2f,0.2f});
        drawText(x + 4, y + 74, "LIFT", GLUT_BITMAP_HELVETICA_10);
    }

    // ---- Station wall clock: hands drawn with rotation transform ----
    void drawClock(double cx, double cy, double r) {
        setColor({0.95f, 0.95f, 0.95f});
        midpointCircle(cx, cy, (int)r, true);
        setColor({0.1f,0.1f,0.1f});
        midpointCircle(cx, cy, (int)r, false);

        float hourAngle = -(fmod(G.clockHours, 12.0f) / 12.0f) * 360.0f;
        float minAngle = -(fmod(G.clockHours * 60.0f, 60.0f) / 60.0f) * 360.0f;

        glPushMatrix();
        glTranslated(cx, cy, 0);
        glRotatef(hourAngle, 0, 0, 1);
        glLineWidth(3.0f);
        glBegin(GL_LINES); glVertex2d(0,0); glVertex2d(0, r*0.5); glEnd();
        glPopMatrix();

        glPushMatrix();
        glTranslated(cx, cy, 0);
        glRotatef(minAngle, 0, 0, 1);
        glLineWidth(2.0f);
        glBegin(GL_LINES); glVertex2d(0,0); glVertex2d(0, r*0.75); glEnd();
        glPopMatrix();
        glLineWidth(1.0f);
    }

    void drawRouteMap(double x, double y) {
        setColor({1,1,1});
        filledRect(x, y, 130, 60);
        setColor({0.2f,0.2f,0.2f});
        outlineRect(x, y, 130, 60);
        drawText(x + 6, y + 46, "MRT LINE-6", GLUT_BITMAP_HELVETICA_10);
        double lx = x + 10, rx = x + 120, ly = y + 20;
        setColor({0.85f,0.15f,0.2f});
        glLineWidth(2.0f);
        ddaLine(lx, ly, rx, ly);
        glLineWidth(1.0f);
        for (int i = 0; i < Cfg::STATION_COUNT; i++) {
            double sx = lx + (rx - lx) * i / (Cfg::STATION_COUNT - 1);
            bool isCurrent = (i == G.currentStationIdx);
            setColor(isCurrent ? RGB{1.0f,0.85f,0.1f} : RGB{1,1,1});
            midpointCircle(sx, ly, isCurrent ? 5 : 3, true);
        }
    }

    void drawCCTV(double x, double y) {
        setColor({0.2f,0.2f,0.2f});
        filledRect(x, y, 22, 10);
        midpointCircle(x + 20, y + 5, 4, true);
        setColor({0.9f, 0.1f, 0.1f});
        midpointCircle(x + 2, y + 8, 1, true); // rec light
    }

    void drawFireExtinguisher(double x, double y) {
        setColor({0.85f, 0.1f, 0.1f});
        filledRect(x, y, 10, 22);
        setColor({0.2f,0.2f,0.2f});
        filledRect(x + 2, y + 22, 6, 5);
    }

    void drawTrashBin(double x, double y) {
        setColor({0.2f, 0.45f, 0.25f});
        filledRect(x, y, 16, 22);
        setColor({0.1f,0.3f,0.15f});
        outlineRect(x, y, 16, 22);
    }

    void drawEmergencyExitSign(double x, double y) {
        setColor({0.05f, 0.6f, 0.15f});
        filledRect(x, y, 60, 18);
        setColor({1,1,1});
        drawText(x + 4, y + 5, "EMERGENCY EXIT", GLUT_BITMAP_HELVETICA_10);
    }

    void draw() {
        drawPlatformFloor();
        drawBench(90, 400);
        drawBench(90, 425);
        drawBench(980, 400);
        drawBench(980, 425);

        drawTicketCounter(40, 250);
        drawSmartGate(220, 250, G.doorsOpen, (float)fabs(sin(G.simSeconds*3)));
        drawSmartGate(280, 250, G.doorsOpen, (float)fabs(cos(G.simSeconds*3)));
        drawEscalator(420, 250);
        drawLift(560, 250);
        drawRouteMap(650, 400);
        drawClock(1150, 430, 22);
        drawCCTV(200, 470);
        drawCCTV(900, 470);
        drawFireExtinguisher(700, 250);
        drawTrashBin(760, 250);
        drawEmergencyExitSign(1000, 250);
    }
}

/*============================================================================
    SECTION 7: PASSENGER SIMULATION
    (Member 3 — Passenger & Traffic Simulation)
    Each passenger runs an individual finite state machine:
      ENTER_STATION -> QUEUE_TICKET -> BUY_TICKET -> WALK_TO_GATE ->
      SCAN_GATE -> WALK_TO_PLATFORM -> WAIT_ON_PLATFORM ->
      BOARD_TRAIN -> (removed) | EXIT_TRAIN -> WALK_TO_EXIT -> (removed)
============================================================================*/
namespace Passengers {

    enum class PType { STUDENT, OFFICE, ELDERLY, TOURIST, CHILD };
    enum class PState {
        ENTER_STATION, QUEUE_TICKET, BUY_TICKET, WALK_TO_GATE,
        SCAN_GATE, WALK_TO_PLATFORM, WAIT_ON_PLATFORM,
        BOARD_TRAIN, RIDING, EXIT_TRAIN, WALK_TO_EXIT_DOOR, DONE
    };

    struct Passenger {
        double x, y;
        double targetX;
        PType type;
        PState state;
        float stateTimer;
        float walkSpeed;
        float bob; // idle animation phase
        int   activity; // 0 idle, 1 phone, 2 newspaper, 3 water
        bool  boardedThisTrain;
    };

    std::vector<Passenger> people;
    float spawnTimer = 0.0f;

    RGB skinTone()  { return {0.85f, 0.68f, 0.52f}; }

    RGB shirtColorFor(PType t) {
        switch (t) {
            case PType::STUDENT: return {0.2f, 0.4f, 0.8f};
            case PType::OFFICE:  return {0.15f, 0.15f, 0.15f};
            case PType::ELDERLY: return {0.5f, 0.5f, 0.35f};
            case PType::TOURIST: return {0.9f, 0.5f, 0.1f};
            case PType::CHILD:   return {0.9f, 0.2f, 0.5f};
        }
        return {0.5f,0.5f,0.5f};
    }

    float heightFor(PType t) {
        switch (t) {
            case PType::CHILD: return 26.0f;
            case PType::ELDERLY: return 34.0f;
            default: return 40.0f;
        }
    }

    void spawnPassenger() {
        Passenger p;
        p.x = -20.0 - (rand() % 40);
        p.y = Cfg::CONCOURSE_BOTTOM + 4;
        p.type = (PType)(rand() % 5);
        p.state = PState::ENTER_STATION;
        p.stateTimer = 0.0f;
        p.walkSpeed = 30.0f + (rand() % 20);
        if (p.type == PType::ELDERLY) p.walkSpeed *= 0.6f;
        if (p.type == PType::CHILD) p.walkSpeed *= 1.1f;
        p.bob = (float)(rand() % 100) / 20.0f;
        p.activity = rand() % 4;
        p.boardedThisTrain = false;
        p.targetX = 240 + (rand() % 60);
        people.push_back(p);
    }

    void moveToward(Passenger& p, double tx, double ty, float dt) {
        double dx = tx - p.x, dy = ty - p.y;
        double dist = sqrt(dx*dx + dy*dy);
        if (dist < 2.0) { p.x = tx; p.y = ty; return; }
        double step = p.walkSpeed * dt;
        if (step > dist) step = dist;
        p.x += dx / dist * step;
        p.y += dy / dist * step;
    }

    void updateOne(Passenger& p, float dt) {
        p.stateTimer += dt;
        p.bob += dt * 4.0f;

        switch (p.state) {
            case PState::ENTER_STATION:
                moveToward(p, 60 + (rand()%20), Cfg::CONCOURSE_BOTTOM + 4, dt);
                if (p.stateTimer > 1.0f) { p.state = PState::QUEUE_TICKET; p.stateTimer = 0; }
                break;

            case PState::QUEUE_TICKET: {
                double qx = 60 + (int)(p.x) % 20;
                moveToward(p, qx, Cfg::CONCOURSE_BOTTOM + 4, dt);
                if (p.stateTimer > 1.5f) { p.state = PState::BUY_TICKET; p.stateTimer = 0; }
                break;
            }
            case PState::BUY_TICKET:
                if (p.stateTimer > 1.2f) { p.state = PState::WALK_TO_GATE; p.stateTimer = 0; }
                break;

            case PState::WALK_TO_GATE:
                moveToward(p, 240, Cfg::CONCOURSE_BOTTOM + 4, dt);
                if (fabs(p.x - 240) < 3.0) { p.state = PState::SCAN_GATE; p.stateTimer = 0; }
                break;

            case PState::SCAN_GATE:
                if (p.stateTimer > 0.6f) {
                    p.state = PState::WALK_TO_PLATFORM;
                    p.stateTimer = 0;
                    p.targetX = 300 + (rand() % 700);
                }
                break;

            case PState::WALK_TO_PLATFORM:
                moveToward(p, p.targetX, Cfg::PLATFORM_BOTTOM + 30, dt);
                if (fabs(p.x - p.targetX) < 3.0 && fabs(p.y - (Cfg::PLATFORM_BOTTOM+30)) < 3.0) {
                    p.state = PState::WAIT_ON_PLATFORM;
                    p.stateTimer = 0;
                }
                break;

            case PState::WAIT_ON_PLATFORM:
                // idle - waits for train doors to open nearby
                if (G.trainState == TrainState::DOORS_OPEN && !p.boardedThisTrain) {
                    if (p.stateTimer > 0.3f + (rand()%10)/10.0f) {
                        p.state = PState::BOARD_TRAIN;
                        p.stateTimer = 0;
                    }
                }
                break;

            case PState::BOARD_TRAIN: {
                double doorX = Train::frontX() - 85;
                moveToward(p, doorX, Cfg::TRACK_Y + 20, dt);
                if (fabs(p.x - doorX) < 4.0) {
                    p.boardedThisTrain = true;
                    p.state = PState::DONE; // considered boarded & removed from concourse view
                    G.passengerCountBoarded++;
                }
                if (G.trainState != TrainState::DOORS_OPEN && G.trainState != TrainState::STOPPED) {
                    // missed the train, go back to waiting for next
                    p.state = PState::WAIT_ON_PLATFORM;
                }
                break;
            }

            default: break;
        }
    }

    void spawnAlighting() {
        // A passenger who exits the train onto the platform when doors open
        if (G.trainState != TrainState::DOORS_OPEN) return;
        if (rand() % 100 < 3) {
            Passenger p;
            p.x = Train::frontX() - 85 + (rand()%2==0 ? -1:1)*10;
            p.y = Cfg::TRACK_Y + 20;
            p.type = (PType)(rand()%5);
            p.state = PState::WALK_TO_EXIT_DOOR;
            p.stateTimer = 0;
            p.walkSpeed = 32.0f;
            p.bob = 0; p.activity = rand()%4;
            p.boardedThisTrain = true;
            p.targetX = 60 + rand()%40;
            people.push_back(p);
        }
    }

    void updateAlighting(Passenger& p, float dt) {
        // Distinct simple path: walk off platform toward concourse exit
        static const double exitY = Cfg::CONCOURSE_BOTTOM + 4;
        if (p.y > exitY + 2) {
            moveToward(p, p.x, exitY, dt);
        } else {
            moveToward(p, p.targetX, exitY, dt);
            if (fabs(p.x - p.targetX) < 3.0) p.state = PState::DONE;
        }
    }

    void update(float dt) {
        if (G.paused) return;
        spawnTimer -= dt;
        if (spawnTimer <= 0 && people.size() < 40) {
            spawnPassenger();
            spawnTimer = 0.6f + (rand()%15)/10.0f;
        }
        spawnAlighting();

        for (auto& p : people) {
            if (p.state == PState::WALK_TO_EXIT_DOOR) updateAlighting(p, dt);
            else updateOne(p, dt);
        }

        people.erase(std::remove_if(people.begin(), people.end(),
            [](const Passenger& p){ return p.state == PState::DONE; }), people.end());

        G.passengerCountOnPlatform = (int)std::count_if(people.begin(), people.end(),
            [](const Passenger& p){ return p.state == PState::WAIT_ON_PLATFORM ||
                                             p.state == PState::WALK_TO_PLATFORM; });
    }

    void drawOne(const Passenger& p) {
        float h = heightFor(p.type);
        float legSwing = (float)sin(p.bob) * 4.0f;
        RGB shirt = shirtColorFor(p.type);

        // legs
        setColor({0.2f,0.2f,0.25f});
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2d(p.x - 3, p.y); glVertex2d(p.x - 3 + legSwing*0.2, p.y - h*0.4);
        glVertex2d(p.x + 3, p.y); glVertex2d(p.x + 3 - legSwing*0.2, p.y - h*0.4);
        glEnd();
        glLineWidth(1.0f);

        // body
        setColor(shirt);
        filledRect(p.x - 7, p.y - h*0.4, 14, h*0.4);

        // head
        setColor(skinTone());
        midpointCircle(p.x, p.y - h*0.05, (int)(h*0.13), true);

        // elderly cane
        if (p.type == PType::ELDERLY) {
            setColor({0.4f,0.3f,0.2f});
            glBegin(GL_LINES);
            glVertex2d(p.x + 8, p.y - h*0.35); glVertex2d(p.x + 8, p.y - h*0.05);
            glEnd();
        }
        // tourist hat
        if (p.type == PType::TOURIST) {
            setColor({0.9f,0.85f,0.2f});
            filledRect(p.x - 9, p.y + h*0.02, 18, 4);
        }
        // activity indicator icons
        if (p.activity == 1) { setColor({0.1f,0.1f,0.1f}); filledRect(p.x+6, p.y-h*0.1, 5, 8); } // phone
        if (p.activity == 2) { setColor({0.95f,0.95f,0.85f}); filledRect(p.x-12, p.y-h*0.12, 10, 8); } // newspaper
    }

    void draw() {
        for (auto& p : people) {
            if (p.state == PState::DONE) continue;
            drawOne(p);
        }
    }
}

/*============================================================================
    SECTION 8: ROAD TRAFFIC SIMULATION
    (Member 3 — Passenger & Traffic Simulation, traffic side)
============================================================================*/
namespace Traffic {

    enum class VType { RICKSHAW, CNG, BUS, CAR, MOTORCYCLE, BICYCLE };

    struct Vehicle {
        double x, y;
        double speed;
        VType type;
        bool movingRight;
    };

    std::vector<Vehicle> vehicles;
    float spawnTimer = 0.0f;

    RGB colorFor(VType t) {
        switch (t) {
            case VType::RICKSHAW:   return {0.9f, 0.6f, 0.1f};
            case VType::CNG:        return {0.1f, 0.55f, 0.2f};
            case VType::BUS:        return {0.75f, 0.1f, 0.15f};
            case VType::CAR:        return {0.2f, 0.3f, 0.7f};
            case VType::MOTORCYCLE: return {0.15f, 0.15f, 0.15f};
            case VType::BICYCLE:    return {0.3f, 0.6f, 0.6f};
        }
        return {0.5f,0.5f,0.5f};
    }

    double widthFor(VType t) {
        switch (t) {
            case VType::BUS: return 90;
            case VType::CAR: return 45;
            case VType::RICKSHAW: return 34;
            case VType::CNG: return 30;
            case VType::MOTORCYCLE: return 22;
            case VType::BICYCLE: return 20;
        }
        return 30;
    }

    void spawn() {
        Vehicle v;
        v.type = (VType)(rand() % 6);
        v.movingRight = (rand() % 2 == 0);
        v.y = v.movingRight ? Cfg::ROAD_BOTTOM + 20 : Cfg::ROAD_BOTTOM + 55;
        v.x = v.movingRight ? -80 : Cfg::WORLD_W + 80;
        v.speed = (40 + rand()%50) * (v.movingRight ? 1 : -1);
        if (v.type == VType::BUS) v.speed *= 0.7;
        if (v.type == VType::BICYCLE) v.speed *= 0.5;
        vehicles.push_back(v);
    }

    void update(float dt) {
        if (G.paused) return;
        spawnTimer -= dt;
        if (spawnTimer <= 0 && vehicles.size() < 14) {
            spawn();
            spawnTimer = 0.5f + (rand()%15)/10.0f;
        }
        for (auto& v : vehicles) v.x += v.speed * dt;
        vehicles.erase(std::remove_if(vehicles.begin(), vehicles.end(),
            [](const Vehicle& v){ return v.x < -150 || v.x > Cfg::WORLD_W + 150; }), vehicles.end());
    }

    void drawVehicle(const Vehicle& v) {
        double w = widthFor(v.type);
        RGB c = colorFor(v.type);
        setColor(c);

        switch (v.type) {
            case VType::BUS:
                filledRect(v.x, v.y, w, 26);
                setColor({0.7f,0.85f,0.95f});
                for (int i=0;i<4;i++) filledRect(v.x + 6 + i*20, v.y + 14, 14, 8);
                break;
            case VType::RICKSHAW:
                filledRect(v.x, v.y, w, 16);
                setColor({0.9f,0.9f,0.2f});
                midpointCircle(v.x + w*0.7, v.y + 18, 8, true);
                break;
            case VType::CNG:
                filledRect(v.x, v.y, w, 18);
                setColor({0.1f,0.35f,0.15f});
                midpointCircle(v.x + w/2, v.y + 22, 7, true);
                break;
            case VType::CAR:
                filledRect(v.x, v.y, w, 18);
                setColor({0.7f,0.85f,0.95f});
                filledRect(v.x + 8, v.y + 12, w - 16, 8);
                break;
            case VType::MOTORCYCLE:
                filledRect(v.x, v.y, w, 8);
                break;
            case VType::BICYCLE:
                setColor({0.1f,0.1f,0.1f});
                midpointCircle(v.x + 4, v.y, 6, false);
                midpointCircle(v.x + w - 4, v.y, 6, false);
                break;
        }
        // wheels for wheeled vehicles
        if (v.type != VType::BICYCLE) {
            setColor({0.1f,0.1f,0.1f});
            midpointCircle(v.x + 6, v.y, 5, true);
            midpointCircle(v.x + w - 6, v.y, 5, true);
        }
    }

    void draw() {
        for (auto& v : vehicles) drawVehicle(v);
    }

    void drawRoadSurface() {
        setColor({0.25f, 0.25f, 0.27f});
        filledRect(0, Cfg::ROAD_BOTTOM, Cfg::WORLD_W, Cfg::ROAD_TOP - Cfg::ROAD_BOTTOM);
        // lane markings (DDA dashed)
        setColor({1.0f, 0.95f, 0.6f});
        for (int x = 0; x < (int)Cfg::WORLD_W; x += 40) {
            ddaLine(x, Cfg::ROAD_BOTTOM + 45, x + 20, Cfg::ROAD_BOTTOM + 45);
        }
        // pedestrian crossing
        setColor({1,1,1});
        for (int i = 0; i < 8; i++) {
            filledRect(560 + i*14, Cfg::ROAD_BOTTOM, 8, Cfg::ROAD_TOP - Cfg::ROAD_BOTTOM);
        }
    }

    // Traffic light for the road (separate from metro Signal)
    float roadLightTimer = 0.0f;
    int roadLightState = 0; // 0 green 1 yellow 2 red

    void updateRoadLight(float dt) {
        roadLightTimer += dt;
        float duration = (roadLightState == 0) ? 6.0f : (roadLightState == 1) ? 1.5f : 5.0f;
        if (roadLightTimer > duration) {
            roadLightTimer = 0.0f;
            roadLightState = (roadLightState + 1) % 3;
        }
    }

    void drawRoadLight(double x, double y) {
        setColor({0.15f,0.15f,0.15f});
        filledRect(x, y, 16, 46);
        RGB dimR={0.25f,0.05f,0.05f}, dimY={0.25f,0.22f,0.05f}, dimG={0.05f,0.2f,0.08f};
        setColor(roadLightState==2 ? RGB{1,0.1f,0.1f} : dimR);
        midpointCircle(x+8, y+38, 5, true);
        setColor(roadLightState==1 ? RGB{1,0.85f,0.1f} : dimY);
        midpointCircle(x+8, y+24, 5, true);
        setColor(roadLightState==0 ? RGB{0.1f,0.9f,0.2f} : dimG);
        midpointCircle(x+8, y+10, 5, true);
    }
}

/*============================================================================
    SECTION 9: GROUND-LEVEL ENVIRONMENT OBJECTS
    (Member 4 — Environment & Effects)
============================================================================*/
namespace GroundEnv {

    void drawGround() {
        setColor({0.30f, 0.22f, 0.15f});
        filledRect(0, 0, Cfg::WORLD_W, Cfg::ROAD_BOTTOM);
    }

    void drawTree(double x, double y, float swayPhase) {
        float sway = (float)sin(G.simSeconds * 1.5 + swayPhase) * 4.0f;
        setColor({0.35f, 0.22f, 0.12f});
        filledRect(x - 3, y, 6, 24);
        setColor({0.15f, 0.55f, 0.2f});
        midpointCircle(x + sway, y + 34, 16, true);
        midpointCircle(x - 10 + sway*0.7, y + 26, 11, true);
        midpointCircle(x + 10 + sway*0.7, y + 26, 11, true);
    }

    void drawStreetLight(double x, double y) {
        bool night = (G.timeOfDay == TimeOfDay::NIGHT || G.timeOfDay == TimeOfDay::EVENING);
        setColor({0.25f,0.25f,0.25f});
        filledRect(x, y, 4, 50);
        setColor(night ? RGB{1.0f, 0.95f, 0.6f} : RGB{0.6f,0.6f,0.55f});
        midpointCircle(x + 2, y + 54, 6, true);
        if (night) {
            setColor({1.0f,0.95f,0.6f}, 0.12f);
            midpointCircle(x + 2, y + 54, 30, true);
        }
    }

    void drawBillboard(double x, double y, const std::string& text) {
        setColor({0.95f, 0.95f, 0.9f});
        filledRect(x, y, 110, 40);
        setColor({0.1f,0.1f,0.1f});
        outlineRect(x, y, 110, 40);
        drawText(x + 6, y + 20, text, GLUT_BITMAP_HELVETICA_12);
        setColor({0.4f,0.4f,0.4f});
        filledRect(x + 50, y - 20, 6, 20);
    }

    void drawFlag(double x, double y) {
        setColor({0.3f,0.3f,0.3f});
        filledRect(x, y, 3, 60);
        float wave = (float)sin(G.simSeconds * 3.0) * 3.0f;
        // Bangladesh flag: green field with red circle
        setColor({0.0f, 0.4f, 0.15f});
        glBegin(GL_QUADS);
        glVertex2d(x+3, y+45);
        glVertex2d(x+40, y+48+wave);
        glVertex2d(x+40, y+30+wave);
        glVertex2d(x+3, y+33);
        glEnd();
        setColor({0.85f, 0.05f, 0.1f});
        midpointCircle(x + 18, y + 39 + wave*0.5, 6, true);
    }

    void drawDivider() {
        setColor({0.6f,0.6f,0.6f});
        filledRect(0, Cfg::ROAD_BOTTOM - 10, Cfg::WORLD_W, 6);
    }

    void draw() {
        drawGround();
        drawDivider();
        drawTree(20, Cfg::ROAD_BOTTOM - 10, 0.2f);
        drawTree(1230, Cfg::ROAD_BOTTOM - 10, 1.1f);
        drawTree(650, Cfg::ROAD_BOTTOM - 10, 2.0f);
        drawStreetLight(120, Cfg::ROAD_BOTTOM - 12);
        drawStreetLight(950, Cfg::ROAD_BOTTOM - 12);
        drawBillboard(760, Cfg::ROAD_BOTTOM + 60, "Dhaka MRT L-6");
        drawFlag(1200, Cfg::ROAD_BOTTOM - 10);
    }
}

/*============================================================================
    SECTION 10: DIGITAL INFORMATION DISPLAY BOARD
    (Member 2 — Station Infrastructure)
============================================================================*/
namespace DigitalDisplay {

    std::string weatherString() {
        switch (G.weather) {
            case WeatherType::CLEAR: return "Clear";
            case WeatherType::RAIN: return "Rain";
            case WeatherType::HEAVY_RAIN: return "Heavy Rain";
            case WeatherType::FOG: return "Fog";
        }
        return "";
    }

    std::string trainStateString() {
        switch (G.trainState) {
            case TrainState::APPROACHING: return "Approaching";
            case TrainState::BRAKING: return "Arriving";
            case TrainState::STOPPED: return "Stopped";
            case TrainState::DOORS_OPEN: return "Boarding";
            case TrainState::DOORS_CLOSING: return "Doors Closing";
            case TrainState::DEPARTING: return "Departing";
            case TrainState::OFFSCREEN: return "In Transit";
        }
        return "";
    }

    void draw() {
        double x = 780, y = 400, w = 250, h = 70;
        setColor({0.05f, 0.05f, 0.08f});
        filledRect(x, y, w, h);
        setColor({0.2f, 0.9f, 0.3f});
        outlineRect(x, y, w, h);

        setColor({0.2f, 1.0f, 0.35f});
        drawText(x + 8, y + h - 14, "NEXT TRAIN", GLUT_BITMAP_HELVETICA_12);

        int destIdx = (G.currentStationIdx + 1) % Cfg::STATION_COUNT;
        std::string dest = std::string("To: ") + Cfg::STATION_NAMES_EN[destIdx];
        drawText(x + 8, y + h - 30, dest, GLUT_BITMAP_HELVETICA_12);

        char etaBuf[48];
        if (G.trainState == TrainState::APPROACHING || G.trainState == TrainState::BRAKING) {
            double remaining = std::max(0.0, (Train::STOP_X - Train::frontX()) / std::max(1.0, G.trainSpeed));
            snprintf(etaBuf, sizeof(etaBuf), "ETA: %d sec", (int)remaining);
        } else if (G.trainState == TrainState::STOPPED || G.trainState == TrainState::DOORS_OPEN) {
            snprintf(etaBuf, sizeof(etaBuf), "ETA: Arrived");
        } else {
            snprintf(etaBuf, sizeof(etaBuf), "ETA: %d sec", (int)G.nextTrainEtaSeconds);
        }
        drawText(x + 8, y + h - 46, etaBuf, GLUT_BITMAP_HELVETICA_12);

        std::string status = "Status: " + trainStateString();
        drawText(x + 8, y + h - 62, status, GLUT_BITMAP_HELVETICA_10);

        std::string plat = "Platform 1   Wx: " + weatherString();
        drawText(x + 130, y + h - 62, plat, GLUT_BITMAP_HELVETICA_10);

        if (!G.eventMessage.empty() && G.eventMessageTimer > 0) {
            setColor({1.0f, 0.3f, 0.2f});
            drawText(x + 8, y + 2, G.eventMessage, GLUT_BITMAP_HELVETICA_10);
        }
    }
}

/*============================================================================
    SECTION 11: STATION NAME SIGNAGE
============================================================================*/
namespace Signage {
    void draw() {
        setColor({0.05f, 0.15f, 0.35f});
        filledRect(340, 495, 260, 34);
        setColor({1,1,1});
        drawText(360, 512, Cfg::STATION_NAMES_EN[G.currentStationIdx], GLUT_BITMAP_HELVETICA_18);
        drawText(360, 498, "Dhaka MRT Line-6", GLUT_BITMAP_HELVETICA_10);
    }
}

/*============================================================================
    SECTION 12: SECURITY FEATURES
    (Member 2/3 shared — Station Infrastructure & Passenger safety)
============================================================================*/
namespace Security {

    void drawGuard(double x, double y) {
        setColor({0.85f, 0.68f, 0.52f});
        midpointCircle(x, y + 34, 6, true);
        setColor({0.05f, 0.25f, 0.05f});
        filledRect(x - 6, y + 6, 12, 26);
        setColor({0.1f,0.1f,0.1f});
        filledRect(x - 3, y, 3, 8);
        filledRect(x + 1, y, 3, 8);
    }

    void drawMetalDetector(double x, double y) {
        setColor({0.6f, 0.6f, 0.65f});
        filledRect(x, y, 6, 60);
        filledRect(x + 34, y, 6, 60);
        filledRect(x, y + 54, 40, 6);
        if (G.fireAlarm) {
            setColor({1.0f, 0.2f, 0.2f}, (float)fabs(sin(G.simSeconds*8)));
            filledRect(x, y, 40, 60);
        }
    }

    void drawEmergencyButton(double x, double y) {
        setColor({0.9f, 0.1f, 0.1f});
        midpointCircle(x, y, 8, true);
        setColor({1,1,1});
        drawText(x - 18, y - 22, "SOS", GLUT_BITMAP_HELVETICA_10);
    }

    void draw() {
        drawGuard(900, 260);
        drawMetalDetector(160, 250);
        drawEmergencyButton(1100, 400);
    }
}

/*============================================================================
    SECTION 13: EMERGENCY EVENT SYSTEM
    (Member 4 — Environment & Effects)
============================================================================*/
void pushEvent(const std::string& msg, float seconds) {
    G.eventMessage = msg;
    G.eventMessageTimer = seconds;
}

namespace EmergencyEvents {

    float randomEventTimer = 25.0f;

    void triggerRandomEvent() {
        int r = rand() % 6;
        switch (r) {
            case 0: pushEvent("Train delay: signal congestion ahead", 4.0f); break;
            case 1:
                G.powerOutage = true;
                pushEvent("POWER OUTAGE - backup power engaged", 4.0f);
                break;
            case 2:
                G.fireAlarm = true;
                pushEvent("FIRE ALARM TRIGGERED - evacuate calmly", 4.0f);
                break;
            case 3:
                Train::emergencyStopToggle();
                break;
            case 4: pushEvent("Passenger medical assistance requested", 4.0f); break;
            case 5: G.weather = WeatherType::HEAVY_RAIN; pushEvent("Heavy rain warning issued", 4.0f); break;
        }
    }

    void update(float dt) {
        if (G.eventMessageTimer > 0) G.eventMessageTimer -= dt;
        if (G.powerOutage && G.eventMessageTimer <= 0) G.powerOutage = false;
        if (G.fireAlarm && G.eventMessageTimer <= 0) G.fireAlarm = false;

        randomEventTimer -= dt;
        if (randomEventTimer <= 0 && !G.paused) {
            randomEventTimer = 30.0f + rand() % 30;
            triggerRandomEvent();
        }
    }

    void drawOverlay() {
        if (G.powerOutage) {
            setColor({0,0,0}, 0.5f);
            filledRect(0, 0, Cfg::WORLD_W, Cfg::WORLD_H);
            setColor({1,0.2f,0.2f});
            drawText(Cfg::WORLD_W/2 - 80, Cfg::WORLD_H/2, "POWER OUTAGE", GLUT_BITMAP_HELVETICA_18);
        }
    }
}

/*============================================================================
    SECTION 14: HEADS-UP DISPLAY (HUD)
    (Member 4 — Environment & Effects)
============================================================================*/
namespace HUD {

    std::string weatherStr() {
        switch (G.weather) {
            case WeatherType::CLEAR: return "Clear";
            case WeatherType::RAIN: return "Rain";
            case WeatherType::HEAVY_RAIN: return "Heavy Rain";
            case WeatherType::FOG: return "Fog";
        }
        return "";
    }

    std::string signalStr() {
        switch (G.signal) {
            case SignalState::GREEN: return "GREEN";
            case SignalState::YELLOW: return "YELLOW";
            case SignalState::RED: return "RED";
            case SignalState::EMERGENCY_FLASH: return "EMERGENCY";
        }
        return "";
    }

    void drawPanel(double x, double y, double w, double h) {
        setColor({0,0,0}, 0.45f);
        filledRect(x, y, w, h);
    }

    void draw() {
        // Top-left panel
        drawPanel(8, Cfg::WORLD_H - 66, 230, 58);
        setColor({1,1,1});
        drawText(16, Cfg::WORLD_H - 20, std::string("Station: ") + Cfg::STATION_NAMES_EN[G.currentStationIdx]);
        drawText(16, Cfg::WORLD_H - 36, std::string("Weather: ") + weatherStr());
        drawText(16, Cfg::WORLD_H - 52, std::string("Time: ") + Env::clockString());

        // Top-right panel
        drawPanel(Cfg::WORLD_W - 200, Cfg::WORLD_H - 66, 192, 58);
        char buf[64];
        snprintf(buf, sizeof(buf), "FPS: %.0f", G.fps);
        drawText(Cfg::WORLD_W - 192, Cfg::WORLD_H - 20, buf);
        snprintf(buf, sizeof(buf), "Passengers: %d", (int)Passengers::people.size());
        drawText(Cfg::WORLD_W - 192, Cfg::WORLD_H - 36, buf);
        snprintf(buf, sizeof(buf), "Train Speed: %.0f km/h", G.trainSpeed * 0.3);
        drawText(Cfg::WORLD_W - 192, Cfg::WORLD_H - 52, buf);

        // Bottom panel
        drawPanel(8, 6, 320, 44);
        drawText(16, 36, std::string("Signal: ") + signalStr());
        drawText(16, 22, std::string("Train: ") + DigitalDisplay::trainStateString());
        drawText(16, 8, std::string("Doors: ") + (G.doorsOpen ? "OPEN" : "CLOSED"));

        if (G.paused) {
            setColor({1,1,0.2f});
            drawText(Cfg::WORLD_W/2 - 30, Cfg::WORLD_H/2, "PAUSED", GLUT_BITMAP_HELVETICA_18);
        }

        if (G.emergencyStop) {
            setColor({1.0f, 0.15f, 0.15f});
            drawText(Cfg::WORLD_W/2 - 90, Cfg::WORLD_H - 90, "EMERGENCY STOP ENGAGED", GLUT_BITMAP_HELVETICA_18);
        }
    }
}

/*============================================================================
    SECTION 15: INPUT HANDLING — KEYBOARD CONTROLS
    ----------------------------------------------------------------------
    A  Train Arrives     D  Train Departs     O  Open Doors
    C  Close Doors        N  Night Mode        M  Morning Mode
    R  Rain                T  Heavy Rain        S  Emergency Stop
    G  Change Signal       P  Pause             L  Platform Lights (toggle overlay)
    H  Horn                F  Fog               ESC Exit
============================================================================*/
bool platformLightsOn = true;

void handleKeyboard(unsigned char key, int, int) {
    switch (key) {
        case 'a': case 'A':
            if (G.trainState == TrainState::OFFSCREEN) Train::resetToApproaching();
            pushEvent("Manual: Train arrival requested", 2.0f);
            break;
        case 'd': case 'D':
            if (G.trainState == TrainState::STOPPED || G.trainState == TrainState::DOORS_OPEN) {
                Train::closeDoors();
            }
            pushEvent("Manual: Train departure requested", 2.0f);
            break;
        case 'o': case 'O':
            Train::openDoors();
            break;
        case 'c': case 'C':
            Train::closeDoors();
            break;
        case 'n': case 'N':
            G.timeOfDay = TimeOfDay::NIGHT;
            G.clockHours = 21.0f;
            pushEvent("Night mode enabled", 2.0f);
            break;
        case 'm': case 'M':
            G.timeOfDay = TimeOfDay::MORNING;
            G.clockHours = 7.0f;
            pushEvent("Morning mode enabled", 2.0f);
            break;
        case 'r': case 'R':
            G.weather = (G.weather == WeatherType::RAIN) ? WeatherType::CLEAR : WeatherType::RAIN;
            pushEvent(G.weather == WeatherType::RAIN ? "Rain started" : "Rain stopped", 2.0f);
            break;
        case 't': case 'T':
            G.weather = (G.weather == WeatherType::HEAVY_RAIN) ? WeatherType::CLEAR : WeatherType::HEAVY_RAIN;
            pushEvent(G.weather == WeatherType::HEAVY_RAIN ? "Heavy rain warning" : "Rain stopped", 2.0f);
            break;
        case 's': case 'S':
            Train::emergencyStopToggle();
            break;
        case 'g': case 'G':
            Signal::cycle();
            pushEvent("Signal changed manually", 1.5f);
            break;
        case 'p': case 'P':
            G.paused = !G.paused;
            break;
        case 'l': case 'L':
            platformLightsOn = !platformLightsOn;
            break;
        case 'h': case 'H':
            Train::triggerHorn();
            break;
        case 'f': case 'F':
            G.weather = (G.weather == WeatherType::FOG) ? WeatherType::CLEAR : WeatherType::FOG;
            pushEvent(G.weather == WeatherType::FOG ? "Fog rolling in" : "Fog cleared", 2.0f);
            break;
        case 27: // ESC
            exit(0);
            break;
        default: break;
    }
}

/*============================================================================
    SECTION 16: PLATFORM LIGHTING
============================================================================*/
namespace PlatformLights {
    void draw() {
        if (!platformLightsOn) return;
        bool night = (G.timeOfDay == TimeOfDay::NIGHT || G.timeOfDay == TimeOfDay::EVENING);
        if (!night && G.weather != WeatherType::FOG) return;

        double positions[] = {150, 450, 750, 1050};
        for (double x : positions) {
            setColor({1.0f, 0.95f, 0.7f}, 0.10f);
            midpointCircle(x, Cfg::PLATFORM_TOP + 10, 60, true);
            setColor({0.6f, 0.6f, 0.6f});
            filledRect(x - 3, Cfg::PLATFORM_TOP, 6, 30);
            setColor({1.0f, 0.95f, 0.7f});
            midpointCircle(x, Cfg::PLATFORM_TOP + 32, 6, true);
        }
    }
}

/*============================================================================
    SECTION 17: MASTER UPDATE & RENDER PIPELINE
============================================================================*/
int lastTimeMs = 0;
int frameCount = 0;
float fpsAccum = 0.0f;

void updateSimulation(float dt) {
    if (dt > 0.1f) dt = 0.1f; // clamp to avoid spiral-of-death on stalls

    G.simSeconds += dt;

    Env::updateTimeOfDay(dt);
    Env::updateClouds(dt);
    Env::updateBirds(dt);
    Env::updateRain(dt);
    Env::updateLightning(dt);

    Signal::update(dt);
    Train::update(dt);
    Passengers::update(dt);
    Traffic::update(dt);
    Traffic::updateRoadLight(dt);
    EmergencyEvents::update(dt);

    if (G.eventMessageTimer <= 0) G.eventMessage.clear();
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // --- Sky & celestial layer ---
    Env::drawSky();
    Env::drawSun();
    Env::drawMoon();
    Env::drawClouds();
    Env::drawBirds();

    // --- Ground level: road, traffic, environment objects ---
    GroundEnv::draw();
    Traffic::drawRoadSurface();
    Traffic::draw();
    Traffic::drawRoadLight(300, Cfg::ROAD_TOP + 4);
    Traffic::drawRoadLight(980, Cfg::ROAD_TOP + 4);

    // --- Elevated bridge & track ---
    Bridge::draw();
    Signal::draw();
    Train::draw();

    // --- Station platform & concourse ---
    Station::draw();
    Security::draw();
    Signage::draw();
    DigitalDisplay::draw();
    PlatformLights::draw();

    // --- Passengers on top of platform/concourse ---
    Passengers::draw();

    // --- Weather overlays (drawn last, above everything) ---
    Env::drawWetReflection();
    Env::drawFog();
    Env::drawRain();
    Env::drawLightningOverlay();
    EmergencyEvents::drawOverlay();

    // --- HUD (always on top) ---
    HUD::draw();

    glutSwapBuffers();
}

void onDisplay() {
    renderScene();
}

void onIdle() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    if (lastTimeMs == 0) lastTimeMs = now;
    float dt = (now - lastTimeMs) / 1000.0f;
    lastTimeMs = now;

    frameCount++;
    fpsAccum += dt;
    if (fpsAccum >= 0.5f) {
        G.fps = frameCount / fpsAccum;
        frameCount = 0;
        fpsAccum = 0.0f;
    }

    updateSimulation(dt);
    glutPostRedisplay();
}

void onReshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, Cfg::WORLD_W, 0, Cfg::WORLD_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void initGL() {
    glClearColor(0.53f, 0.80f, 0.98f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, Cfg::WORLD_W, 0, Cfg::WORLD_H);
    glMatrixMode(GL_MODELVIEW);
}

/*============================================================================
    SECTION 18: PROGRAM ENTRY POINT
============================================================================*/
int main(int argc, char** argv) {
    srand((unsigned)time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(Cfg::WINDOW_W, Cfg::WINDOW_H);
    glutInitWindowPosition(80, 40);
    glutCreateWindow("Dhaka Metro Rail Smart Station Simulator - MRT Line-6");

    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        fprintf(stderr, "GLEW initialization failed: %s\n", glewGetErrorString(glewStatus));
        // Continue anyway: fixed-function pipeline used here does not strictly require GLEW.
    }

    initGL();
    Train::resetToApproaching();

    glutDisplayFunc(onDisplay);
    glutReshapeFunc(onReshape);
    glutIdleFunc(onIdle);
    glutKeyboardFunc(handleKeyboard);

    printf("========================================================\n");
    printf(" DHAKA METRO RAIL SMART STATION SIMULATOR (MRT Line-6)\n");
    printf("========================================================\n");
    printf(" Controls:\n");
    printf("   A - Train Arrives     D - Train Departs\n");
    printf("   O - Open Doors        C - Close Doors\n");
    printf("   N - Night Mode        M - Morning Mode\n");
    printf("   R - Rain              T - Heavy Rain\n");
    printf("   S - Emergency Stop    G - Change Signal\n");
    printf("   P - Pause             L - Platform Lights\n");
    printf("   H - Horn              F - Fog\n");
    printf("   ESC - Exit\n");
    printf("========================================================\n");

    glutMainLoop();
    return 0;
}