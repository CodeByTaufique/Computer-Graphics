//
// Created by  TauFique Hassan on 7/19/26.
//
/*
 * ============================================================================
 *  HAZRAT SHAHJALAL INTERNATIONAL AIRPORT — SMART OPERATIONS SIMULATOR
 * ============================================================================
 *  A real-time 2D airport management & visualization system built with
 *  C++ / OpenGL (Legacy Immediate Mode) / FreeGLUT.
 *
 *  Single-file build for CLion / g++ / MSVC.
 *
 *  Course topics demonstrated:
 *    - DDA & Bresenham line drawing        - Midpoint circle algorithm
 *    - Cubic Bezier curves (flight paths)  - 2D transformations
 *    - Rotation / Translation / Scaling    - RGB color model & blending
 *    - Real-time animation & clipping      - Scene graph style rendering
 *
 *  Build (g++):
 *      g++ AirportSim.cpp -o AirportSim -lfreeglut -lglew32 -lopengl32 -lglu32
 *  Build (Linux):
 *      g++ AirportSim.cpp -o AirportSim -lGL -lGLU -lglut -lGLEW
 *
 *  Controls:   see PrintControls() / on-screen HUD (key: '/')
 * ============================================================================
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <iomanip>
#include <algorithm>

// ============================================================================
//  GLOBAL CONSTANTS
// ============================================================================
static const double PI = 3.14159265358979323846;
static int   WIN_W = 1400;
static int   WIN_H = 850;

// World space is a fixed virtual coordinate system, independent of the
// actual window pixel size. We map it with glOrtho every reshape.
static const double WORLD_W = 1400.0;
static const double WORLD_H = 850.0;

// Vertical bands of the airport scene (see README layout diagram)
static const double SKY_TOP        = 850.0;
static const double SKY_BOTTOM     = 620.0;
static const double AIR_ZONE_TOP   = 620.0;
static const double AIR_ZONE_BOT   = 520.0;
static const double RUNWAY_TOP     = 520.0;
static const double RUNWAY_BOT     = 460.0;
static const double TAXIWAY_TOP    = 460.0;
static const double TAXIWAY_BOT    = 415.0;
static const double TOWER_BAND_TOP = 415.0;
static const double TOWER_BAND_BOT = 330.0;
static const double TERMINAL_TOP   = 330.0;
static const double TERMINAL_BOT   = 170.0;
static const double APRON_TOP      = 170.0;
static const double APRON_BOT      = 80.0;
static const double ROAD_TOP       = 80.0;
static const double ROAD_BOT       = 0.0;

// ============================================================================
//  SMALL MATH / GEOMETRY HELPERS
// ============================================================================
struct Vec2 {
    double x, y;
    Vec2() : x(0), y(0) {}
    Vec2(double _x, double _y) : x(_x), y(_y) {}
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }
};

static inline double Lerp(double a, double b, double t) { return a + (b - a) * t; }

static inline double Clamp(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline double DegToRad(double d) { return d * PI / 180.0; }

static inline double RandRange(double lo, double hi) {
    return lo + (double)rand() / (double)RAND_MAX * (hi - lo);
}

// Cubic Bezier evaluation — used for flight approach / departure trajectories
static Vec2 CubicBezier(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, double t) {
    double u = 1.0 - t;
    double b0 = u * u * u;
    double b1 = 3 * u * u * t;
    double b2 = 3 * u * t * t;
    double b3 = t * t * t;
    return Vec2(
        b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
        b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y
    );
}

// ============================================================================
//  PRIMITIVE DRAWING — DDA line, Bresenham line, Midpoint circle
//  (Implemented explicitly per Computer Graphics course requirements rather
//   than relying purely on glBegin(GL_LINES); these rasterize into immediate
//   mode point/line calls so the underlying algorithm is genuinely used.)
// ============================================================================

// DDA Line Algorithm — used for runway markings, taxiway lines, road lanes
static void DDA_Line(double x0, double y0, double x1, double y1) {
    double dx = x1 - x0;
    double dy = y1 - y0;
    int steps = (int)std::max(fabs(dx), fabs(dy));
    if (steps == 0) {
        glBegin(GL_POINTS);
        glVertex2d(x0, y0);
        glEnd();
        return;
    }
    double xInc = dx / (double)steps;
    double yInc = dy / (double)steps;
    double x = x0, y = y0;
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2d(x, y);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// Bresenham Line Algorithm — used for navigation markings / signage / guidance paths
static void Bresenham_Line(int x0, int y0, int x1, int y1) {
    int dx = std::abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;
    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    glEnd();
}

// Midpoint Circle Algorithm — used for radar rings, wheels, lights, sun/moon
static void MidpointCircle(double cx, double cy, double r) {
    int x = 0;
    int y = (int)r;
    int d = 1 - (int)r;
    glBegin(GL_POINTS);
    while (x <= y) {
        glVertex2d(cx + x, cy + y); glVertex2d(cx - x, cy + y);
        glVertex2d(cx + x, cy - y); glVertex2d(cx - x, cy - y);
        glVertex2d(cx + y, cy + x); glVertex2d(cx - y, cy + x);
        glVertex2d(cx + y, cy - x); glVertex2d(cx - y, cy - x);
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
    glEnd();
}

// Filled circle via triangle fan (used heavily for lights/wheels/sun where a
// filled disc reads more clearly on screen than a stippled midpoint outline)
static void FilledCircle(double cx, double cy, double r, int segments = 28) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(cx, cy);
    for (int i = 0; i <= segments; i++) {
        double theta = 2.0 * PI * (double)i / (double)segments;
        glVertex2d(cx + r * cos(theta), cy + r * sin(theta));
    }
    glEnd();
}

static void RingCircle(double cx, double cy, double r, double thickness) {
    glBegin(GL_TRIANGLE_STRIP);
    int segments = 40;
    for (int i = 0; i <= segments; i++) {
        double theta = 2.0 * PI * (double)i / (double)segments;
        double ox = cos(theta), oy = sin(theta);
        glVertex2d(cx + ox * (r + thickness), cy + oy * (r + thickness));
        glVertex2d(cx + ox * r, cy + oy * r);
    }
    glEnd();
}

static void FilledRect(double x, double y, double w, double h) {
    glBegin(GL_QUADS);
    glVertex2d(x, y);
    glVertex2d(x + w, y);
    glVertex2d(x + w, y + h);
    glVertex2d(x, y + h);
    glEnd();
}

static void FilledTriangle(double x0, double y0, double x1, double y1, double x2, double y2) {
    glBegin(GL_TRIANGLES);
    glVertex2d(x0, y0);
    glVertex2d(x1, y1);
    glVertex2d(x2, y2);
    glEnd();
}

static void LineRect(double x, double y, double w, double h) {
    glBegin(GL_LINE_LOOP);
    glVertex2d(x, y);
    glVertex2d(x + w, y);
    glVertex2d(x + w, y + h);
    glVertex2d(x, y + h);
    glEnd();
}

// Bitmap text helper
static void DrawText(double x, double y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2d(x, y);
    for (size_t i = 0; i < text.size(); i++) {
        glutBitmapCharacter(font, text[i]);
    }
}

static int TextWidth(const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18) {
    int w = 0;
    for (size_t i = 0; i < text.size(); i++) w += glutBitmapWidth(font, text[i]);
    return w;
}

// ============================================================================
//  ENUMS — SIMULATION STATE
// ============================================================================
enum WeatherType   { WEATHER_CLEAR, WEATHER_RAIN, WEATHER_HEAVY_RAIN, WEATHER_FOG, WEATHER_STORM };
enum TimeOfDay      { TOD_MORNING, TOD_AFTERNOON, TOD_EVENING, TOD_NIGHT };
enum CameraMode      { CAM_TOWER, CAM_RUNWAY, CAM_TERMINAL, CAM_RADAR, CAM_FOLLOW };
enum AircraftPhase  {
    PHASE_HOLDING,      // waiting at gate, engines off
    PHASE_PUSHBACK,     // being pushed back from gate
    PHASE_TAXI_OUT,     // taxiing toward runway
    PHASE_HOLDING_SHORT,// waiting for runway clearance
    PHASE_TAKEOFF_ROLL, // accelerating down runway
    PHASE_CLIMB,        // climbing away after rotation
    PHASE_DEPARTED,      // gone — to be removed
    PHASE_APPROACH,     // inbound on final approach curve
    PHASE_TOUCHDOWN,    // just landed, decelerating on runway
    PHASE_TAXI_IN,       // taxiing to gate after landing
    PHASE_AT_GATE        // docked, unloading passengers
};
enum AircraftCategory { CAT_DOMESTIC, CAT_INTERNATIONAL, CAT_CARGO };
enum EmergencyType {
    EMG_NONE, EMG_BIRD_STRIKE, EMG_ENGINE_FAILURE, EMG_FUEL_LEAK,
    EMG_MEDICAL, EMG_FIRE, EMG_RUNWAY_OBSTRUCTION, EMG_EMERGENCY_LANDING
};

// ============================================================================
//  AIRCRAFT
// ============================================================================
struct AircraftType {
    std::string name;
    AircraftCategory category;
    double bodyLength;   // world units
    double bodyWidth;
    double wingSpan;
    float  r, g, b;      // livery color
    float  tailR, tailG, tailB;
};

static std::vector<AircraftType> g_aircraftTypes;

static void InitAircraftTypes() {
    g_aircraftTypes.push_back({ "Biman ATR",     CAT_DOMESTIC,      70, 14, 60, 0.85f, 0.10f, 0.20f, 0.05f, 0.30f, 0.10f });
    g_aircraftTypes.push_back({ "US-Bangla ATR",  CAT_DOMESTIC,      70, 14, 60, 0.95f, 0.55f, 0.10f, 0.55f, 0.10f, 0.10f });
    g_aircraftTypes.push_back({ "Novoair",        CAT_DOMESTIC,      68, 13, 58, 0.90f, 0.20f, 0.20f, 0.15f, 0.15f, 0.55f });
    g_aircraftTypes.push_back({ "Boeing 737",     CAT_INTERNATIONAL, 95, 16, 82, 0.92f, 0.92f, 0.95f, 0.05f, 0.30f, 0.65f });
    g_aircraftTypes.push_back({ "Airbus A320",    CAT_INTERNATIONAL, 92, 16, 80, 0.88f, 0.90f, 0.93f, 0.80f, 0.10f, 0.10f });
    g_aircraftTypes.push_back({ "Boeing 777",     CAT_INTERNATIONAL,130, 20,120, 0.95f, 0.95f, 0.97f, 0.10f, 0.20f, 0.55f });
    g_aircraftTypes.push_back({ "Airbus A330",    CAT_INTERNATIONAL,125, 19,115, 0.93f, 0.93f, 0.95f, 0.55f, 0.10f, 0.60f });
    g_aircraftTypes.push_back({ "DHL Cargo",       CAT_CARGO,         98, 17, 84, 0.95f, 0.80f, 0.10f, 0.80f, 0.10f, 0.10f });
    g_aircraftTypes.push_back({ "FedEx Cargo",     CAT_CARGO,         98, 17, 84, 0.60f, 0.10f, 0.20f, 0.35f, 0.20f, 0.55f });
}

struct Gate {
    std::string label;
    double x, y;      // dock position (nose point)
    bool occupied;
    int aircraftId;   // -1 if free
};

struct Aircraft {
    int id;
    std::string flightNumber;
    std::string destination;
    int typeIndex;
    AircraftPhase phase;
    bool isArrival;   // true = arriving flight, false = departing flight

    Vec2 pos;
    double heading;    // degrees, 0 = facing +x (east)
    double speed;      // world units / second
    double targetSpeed;
    double altitude;   // 0 = on ground, >0 = airborne (visual scale factor)

    int gateIndex;     // assigned gate, -1 if none
    double phaseTimer; // seconds spent in current phase
    double bezierT;    // progress along an approach/departure curve [0,1]

    bool lightsOn;
    double flapAngle;
    double propSpin;   // for ATR turboprops

    std::string statusText;
};

static std::vector<Aircraft> g_aircraft;
static int g_nextAircraftId = 1;
static double g_simTime = 0.0; // seconds since sim start

// ============================================================================
//  GROUND VEHICLES
// ============================================================================
enum VehicleType { VEH_FUEL, VEH_BAGGAGE, VEH_CATERING, VEH_PUSHBACK, VEH_FOLLOWME, VEH_SHUTTLE, VEH_FIRE };

struct GroundVehicle {
    VehicleType type;
    Vec2 pos;
    Vec2 target;
    double speed;
    bool active;
    int assignedGate;
    double taskTimer;
    std::string label;
};

static std::vector<GroundVehicle> g_vehicles;

// Road traffic (background flavor vehicles on the airport road)
struct RoadVehicle {
    double x;
    int lane;         // 0..3
    double speed;
    float r, g, b;
    bool isBus;
};
static std::vector<RoadVehicle> g_roadVehicles;

// ============================================================================
//  RUNWAY / ATC QUEUE STATE
// ============================================================================
struct RunwayState {
    bool occupied;
    int occupantAircraftId; // -1 if free
    std::deque<int> takeoffQueue;   // aircraft ids waiting to depart
    std::deque<int> landingQueue;   // aircraft ids waiting to land
};
static RunwayState g_runway = { false, -1, {}, {} };

// ============================================================================
//  FLIGHT INFORMATION DISPLAY (FIDS) ENTRY
// ============================================================================
struct FIDSEntry {
    std::string flightNumber;
    std::string destinationOrOrigin;
    std::string status;
    std::string gate;
    std::string time;
    bool isArrival;
};
static std::deque<FIDSEntry> g_fids;

// ============================================================================
//  ATC MESSAGE LOG
// ============================================================================
struct LogMessage {
    std::string text;
    double timestamp;
    int severity; // 0 normal, 1 warning, 2 critical
};
static std::deque<LogMessage> g_atcLog;

static void PushLog(const std::string& text, int severity = 0) {
    LogMessage m;
    m.text = text;
    m.timestamp = g_simTime;
    m.severity = severity;
    g_atcLog.push_front(m);
    if (g_atcLog.size() > 8) g_atcLog.pop_back();
}

// ============================================================================
//  EMERGENCY STATE
// ============================================================================
struct EmergencyState {
    bool active;
    EmergencyType type;
    double timer;
    double duration;
    std::string description;
    Vec2 location;
};
static EmergencyState g_emergency = { false, EMG_NONE, 0, 0, "", Vec2() };

// ============================================================================
//  WEATHER / ENVIRONMENT STATE
// ============================================================================
static WeatherType g_weather = WEATHER_CLEAR;
static TimeOfDay   g_timeOfDay = TOD_AFTERNOON;
static double      g_dayClock = 0.35; // 0..1 fraction of a full day cycle, 0.35 ~ midday
static bool        g_autoDayNight = true;

struct RainDrop { double x, y, speed, len; };
static std::vector<RainDrop> g_rainDrops;

struct LightningBolt { double life; double x; };
static LightningBolt g_lightning = { 0, 0 };

struct Cloud { double x, y, scale, speed; };
static std::vector<Cloud> g_clouds;

// ============================================================================
//  CAMERA / VIEW STATE
// ============================================================================
static CameraMode g_camMode = CAM_TOWER;
static int  g_followAircraftId = -1;
static double g_camZoom = 1.0;
static double g_camPanX = 0.0, g_camPanY = 0.0;

// ============================================================================
//  SIMULATION CONTROL
// ============================================================================
static bool g_paused = false;
static bool g_showHelp = false;
static bool g_showDashboard = true;
static int  g_selectedGate = 0;

// Statistics
struct AirportStats {
    int flightsHandled;
    int totalDelaysMinutes;
    int emergenciesHandled;
    double runwayBusySeconds;
    double simStartWallTime;
};
static AirportStats g_stats = { 0, 0, 0, 0.0, 0.0 };


// ============================================================================
//  GATES SETUP
// ============================================================================
static std::vector<Gate> g_gates;

static void InitGates() {
    g_gates.clear();
    double startX = 300, spacing = 220;
    const char* labels[4] = { "Gate A", "Gate B", "Gate C", "Gate D" };
    for (int i = 0; i < 4; i++) {
        Gate g;
        g.label = labels[i];
        g.x = startX + i * spacing;
        g.y = TERMINAL_BOT + 6;
        g.occupied = false;
        g.aircraftId = -1;
        g_gates.push_back(g);
    }
}

// ============================================================================
//  COLOR PALETTE (varies with time of day / weather for mood lighting)
// ============================================================================
struct Palette {
    float skyTop[3], skyBottom[3];
    float groundTint[3];
    float ambient; // 0..1 overall brightness multiplier
};

static Palette GetPalette() {
    Palette p;
    switch (g_timeOfDay) {
        case TOD_MORNING:
            { float a[3] = {1.00f, 0.80f, 0.55f}; float b[3] = {1.00f, 0.90f, 0.70f}; memcpy(p.skyTop,a,sizeof a); memcpy(p.skyBottom,b,sizeof b); p.ambient = 0.85f; }
            break;
        case TOD_AFTERNOON:
            { float a[3] = {0.35f, 0.65f, 0.95f}; float b[3] = {0.75f, 0.88f, 0.98f}; memcpy(p.skyTop,a,sizeof a); memcpy(p.skyBottom,b,sizeof b); p.ambient = 1.0f; }
            break;
        case TOD_EVENING:
            { float a[3] = {0.85f, 0.40f, 0.35f}; float b[3] = {1.00f, 0.70f, 0.45f}; memcpy(p.skyTop,a,sizeof a); memcpy(p.skyBottom,b,sizeof b); p.ambient = 0.75f; }
            break;
        case TOD_NIGHT:
        default:
            { float a[3] = {0.02f, 0.03f, 0.10f}; float b[3] = {0.06f, 0.08f, 0.20f}; memcpy(p.skyTop,a,sizeof a); memcpy(p.skyBottom,b,sizeof b); p.ambient = 0.35f; }
            break;
    }
    if (g_weather == WEATHER_FOG) p.ambient *= 0.8f;
    if (g_weather == WEATHER_STORM) p.ambient *= 0.55f;
    if (g_weather == WEATHER_HEAVY_RAIN) p.ambient *= 0.7f;
    return p;
}

// ============================================================================
//  SKY / SUN / MOON / CLOUDS
// ============================================================================
static void InitClouds() {
    g_clouds.clear();
    for (int i = 0; i < 6; i++) {
        Cloud c;
        c.x = RandRange(0, WORLD_W);
        c.y = RandRange(SKY_BOTTOM + 40, SKY_TOP - 30);
        c.scale = RandRange(0.7, 1.6);
        c.speed = RandRange(4.0, 10.0);
        g_clouds.push_back(c);
    }
}

static void DrawCloud(double x, double y, double scale) {
    glColor4f(1.0f, 1.0f, 1.0f, 0.85f);
    FilledCircle(x, y, 14 * scale);
    FilledCircle(x + 16 * scale, y + 6 * scale, 18 * scale);
    FilledCircle(x + 34 * scale, y, 13 * scale);
    FilledCircle(x + 12 * scale, y - 6 * scale, 12 * scale);
    FilledRect(x - 2 * scale, y - 8 * scale, 40 * scale, 10 * scale);
}

static void DrawSkyGradient() {
    Palette pal = GetPalette();
    glBegin(GL_QUADS);
    glColor3f(pal.skyTop[0], pal.skyTop[1], pal.skyTop[2]);
    glVertex2d(0, SKY_TOP);
    glVertex2d(WORLD_W, SKY_TOP);
    glColor3f(pal.skyBottom[0], pal.skyBottom[1], pal.skyBottom[2]);
    glVertex2d(WORLD_W, SKY_BOTTOM);
    glVertex2d(0, SKY_BOTTOM);
    glEnd();

    // Storm darkening overlay
    if (g_weather == WEATHER_STORM) {
        glColor4f(0.05f, 0.05f, 0.08f, 0.35f);
        FilledRect(0, SKY_BOTTOM, WORLD_W, SKY_TOP - SKY_BOTTOM);
    }
}

static void DrawSunMoon() {
    // g_dayClock: 0.0 = midnight, 0.25 = sunrise/morning, 0.5 = noon-ish afternoon,
    // 0.75 = evening/sunset, 1.0 = midnight again
    double arc = g_dayClock; // 0..1
    double angle = arc * 2.0 * PI - PI / 2.0; // starts at bottom, arcs over top
    double cx = WORLD_W / 2.0;
    double cy = SKY_BOTTOM - 60;
    double radiusX = WORLD_W / 2.0 - 40;
    double radiusY = 260;
    double sx = cx + radiusX * cos(angle);
    double sy = cy + radiusY * fabs(sin(angle)) + (SKY_TOP - SKY_BOTTOM) * 0.15;

    bool sunVisible = (g_timeOfDay != TOD_NIGHT);
    if (sunVisible) {
        glColor3f(1.0f, 0.92f, 0.55f);
        if (g_timeOfDay == TOD_EVENING) glColor3f(1.0f, 0.55f, 0.30f);
        if (g_timeOfDay == TOD_MORNING) glColor3f(1.0f, 0.85f, 0.45f);
        FilledCircle(sx, SKY_TOP - 90, 34);
        // sun rays
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        for (int i = 0; i < 8; i++) {
            double a = i * PI / 4.0;
            glVertex2d(sx + cos(a) * 40, (SKY_TOP - 90) + sin(a) * 40);
            glVertex2d(sx + cos(a) * 52, (SKY_TOP - 90) + sin(a) * 52);
        }
        glEnd();
    } else {
        // Moon
        glColor3f(0.90f, 0.90f, 0.95f);
        FilledCircle(WORLD_W - 140, SKY_TOP - 90, 26);
        glColor3f(0.06f, 0.08f, 0.20f);
        FilledCircle(WORLD_W - 128, SKY_TOP - 96, 22);
        // Stars
        for (int i = 0; i < 40; i++) {
            double sxp = fmod(i * 137.0, WORLD_W);
            double syp = SKY_BOTTOM + 30 + fmod(i * 59.0, (SKY_TOP - SKY_BOTTOM - 40));
            double tw = 0.5 + 0.5 * sin(g_simTime * 3.0 + i);
            glColor3f((float)(0.7 + 0.3 * tw), (float)(0.7 + 0.3 * tw), 1.0f);
            FilledCircle(sxp, syp, 1.3);
        }
    }
}

static void UpdateClouds(double dt) {
    for (size_t i = 0; i < g_clouds.size(); i++) {
        g_clouds[i].x += g_clouds[i].speed * dt;
        if (g_clouds[i].x > WORLD_W + 60) g_clouds[i].x = -60;
    }
}

static void DrawClouds() {
    for (size_t i = 0; i < g_clouds.size(); i++) {
        DrawCloud(g_clouds[i].x, g_clouds[i].y, g_clouds[i].scale);
    }
}

// ============================================================================
//  WEATHER EFFECTS — RAIN / FOG / LIGHTNING
// ============================================================================
static void InitRain() {
    g_rainDrops.clear();
    for (int i = 0; i < 260; i++) {
        RainDrop d;
        d.x = RandRange(0, WORLD_W);
        d.y = RandRange(0, WORLD_H);
        d.speed = RandRange(300, 520);
        d.len = RandRange(10, 22);
        g_rainDrops.push_back(d);
    }
}

static void UpdateWeather(double dt) {
    if (g_weather == WEATHER_RAIN || g_weather == WEATHER_HEAVY_RAIN || g_weather == WEATHER_STORM) {
        double mult = (g_weather == WEATHER_HEAVY_RAIN) ? 1.6 : (g_weather == WEATHER_STORM ? 1.9 : 1.0);
        for (size_t i = 0; i < g_rainDrops.size(); i++) {
            g_rainDrops[i].y -= g_rainDrops[i].speed * mult * dt;
            g_rainDrops[i].x -= 40 * mult * dt;
            if (g_rainDrops[i].y < 0) {
                g_rainDrops[i].y = WORLD_H;
                g_rainDrops[i].x = RandRange(0, WORLD_W);
            }
        }
    }
    if (g_weather == WEATHER_STORM) {
        g_lightning.life -= dt;
        if (g_lightning.life <= 0 && (rand() % 240 == 0)) {
            g_lightning.life = 0.15;
            g_lightning.x = RandRange(100, WORLD_W - 100);
        }
    }
}

static void DrawRain() {
    if (g_weather != WEATHER_RAIN && g_weather != WEATHER_HEAVY_RAIN && g_weather != WEATHER_STORM) return;
    glColor4f(0.75f, 0.85f, 1.0f, 0.55f);
    glLineWidth(1.0f);
    for (size_t i = 0; i < g_rainDrops.size(); i++) {
        DDA_Line(g_rainDrops[i].x, g_rainDrops[i].y, g_rainDrops[i].x - 4, g_rainDrops[i].y - g_rainDrops[i].len);
    }
}

static void DrawLightning() {
    if (g_weather != WEATHER_STORM || g_lightning.life <= 0) return;
    float alpha = (float)(g_lightning.life / 0.15);
    glColor4f(1.0f, 1.0f, 1.0f, alpha * 0.6f);
    FilledRect(0, 0, WORLD_W, WORLD_H);
    glColor3f(1.0f, 1.0f, 0.9f);
    glLineWidth(3.0f);
    double x = g_lightning.x, y = SKY_TOP;
    glBegin(GL_LINE_STRIP);
    glVertex2d(x, y);
    while (y > RUNWAY_TOP) {
        x += RandRange(-25, 25);
        y -= RandRange(30, 60);
        glVertex2d(x, y);
    }
    glEnd();
}

static void DrawFogOverlay() {
    if (g_weather != WEATHER_FOG) return;
    glColor4f(0.85f, 0.85f, 0.88f, 0.32f);
    FilledRect(0, RUNWAY_BOT - 20, WORLD_W, SKY_TOP - RUNWAY_BOT + 20);
    glColor4f(0.85f, 0.85f, 0.88f, 0.5f);
    FilledRect(0, 0, WORLD_W, TAXIWAY_TOP);
}


// ============================================================================
//  RUNWAY RENDERING
// ============================================================================
static void DrawRunway() {
    // Asphalt base
    glColor3f(0.16f, 0.16f, 0.18f);
    FilledRect(0, RUNWAY_BOT, WORLD_W, RUNWAY_TOP - RUNWAY_BOT);

    // Wet sheen if raining
    if (g_weather == WEATHER_RAIN || g_weather == WEATHER_HEAVY_RAIN || g_weather == WEATHER_STORM) {
        glColor4f(0.35f, 0.40f, 0.50f, 0.25f);
        FilledRect(0, RUNWAY_BOT, WORLD_W, RUNWAY_TOP - RUNWAY_BOT);
    }

    // Threshold markings (stripes) at both ends
    glColor3f(0.95f, 0.95f, 0.95f);
    for (int end = 0; end < 2; end++) {
        double baseX = (end == 0) ? 20 : WORLD_W - 20 - 8 * 10;
        for (int i = 0; i < 8; i++) {
            FilledRect(baseX + i * 10, RUNWAY_BOT + 4, 5, RUNWAY_TOP - RUNWAY_BOT - 8);
        }
    }

    // Center line — dashed, drawn with DDA to satisfy algorithm requirement
    double midY = (RUNWAY_TOP + RUNWAY_BOT) / 2.0;
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);
    for (double x = 40; x < WORLD_W - 40; x += 50) {
        DDA_Line(x, midY, x + 28, midY);
    }

    // Runway designator numbers "14" / "32"
    glColor3f(1,1,1);
    DrawText(35, midY - 30, "14", GLUT_BITMAP_TIMES_ROMAN_24);
    DrawText(WORLD_W - 60, midY - 30, "32", GLUT_BITMAP_TIMES_ROMAN_24);

    // Edge lights
    bool nightLights = (g_timeOfDay == TOD_NIGHT || g_weather == WEATHER_FOG || g_weather == WEATHER_STORM);
    if (nightLights) {
        for (double x = 10; x < WORLD_W; x += 40) {
            float flick = 0.85f + 0.15f * (float)sin(g_simTime * 6.0 + x);
            glColor3f(1.0f * flick, 0.85f * flick, 0.2f * flick);
            FilledCircle(x, RUNWAY_TOP + 4, 2.4);
            FilledCircle(x, RUNWAY_BOT - 4, 2.4);
        }
        // Approach lighting system at both ends
        for (int end = 0; end < 2; end++) {
            double dirSign = (end == 0) ? -1.0 : 1.0;
            double baseX = (end == 0) ? 0 : WORLD_W;
            for (int i = 1; i <= 6; i++) {
                glColor3f(1.0f, 1.0f, 1.0f);
                FilledCircle(baseX + dirSign * i * 14, midY, 1.8);
            }
        }
    }
}

// ============================================================================
//  TAXIWAYS
// ============================================================================
static void DrawTaxiways() {
    glColor3f(0.20f, 0.20f, 0.22f);
    FilledRect(0, TAXIWAY_BOT, WORLD_W, TAXIWAY_TOP - TAXIWAY_BOT);

    glColor3f(1.0f, 0.85f, 0.15f);
    double midY = (TAXIWAY_TOP + TAXIWAY_BOT) / 2.0;
    for (double x = 20; x < WORLD_W - 20; x += 36) {
        DDA_Line(x, midY, x + 20, midY);
    }

    DrawText(30, TAXIWAY_TOP - 18, "TAXIWAY ALPHA", GLUT_BITMAP_HELVETICA_12);
    DrawText(WORLD_W - 170, TAXIWAY_TOP - 18, "TAXIWAY BRAVO", GLUT_BITMAP_HELVETICA_12);

    // Taxiway connector links up to runway (Bresenham guidance path demo)
    glColor3f(1.0f, 0.85f, 0.15f);
    glPointSize(2.0f);
    Bresenham_Line(260, (int)TAXIWAY_TOP, 260, (int)RUNWAY_BOT);
    Bresenham_Line(WORLD_W - 260, (int)TAXIWAY_TOP, WORLD_W - 260, (int)RUNWAY_BOT);
}

// ============================================================================
//  CONTROL TOWER
// ============================================================================
static void DrawControlTower() {
    double baseX = WORLD_W / 2.0 - 30;
    double baseY = TOWER_BAND_BOT;
    // Shaft
    glColor3f(0.55f, 0.56f, 0.58f);
    FilledRect(baseX, baseY, 26, 70);
    glColor3f(0.45f, 0.46f, 0.50f);
    FilledRect(baseX + 26, baseY, 8, 70); // shading strip

    // Cab (radar room) — octagon-ish via polygon
    double cabY = baseY + 70;
    glColor3f(0.25f, 0.55f, 0.75f);
    glBegin(GL_POLYGON);
    glVertex2d(baseX - 18, cabY);
    glVertex2d(baseX - 18, cabY + 24);
    glVertex2d(baseX - 8, cabY + 34);
    glVertex2d(baseX + 42, cabY + 34);
    glVertex2d(baseX + 52, cabY + 24);
    glVertex2d(baseX + 52, cabY);
    glEnd();
    glColor3f(0.10f, 0.10f, 0.12f);
    LineRect(baseX - 18, cabY, 70, 24);

    // Roof + rotating radar dish
    glColor3f(0.30f, 0.30f, 0.33f);
    FilledRect(baseX - 6, cabY + 34, 58, 4);
    double dishX = baseX + 17, dishY = cabY + 46;
    glColor3f(0.85f, 0.15f, 0.15f);
    FilledRect(dishX - 1.5, cabY + 34, 3, 12);
    glPushMatrix();
    glTranslatef((float)dishX, (float)dishY, 0);
    glRotatef((float)fmod(g_simTime * 140.0, 360.0), 0, 0, 1);
    glColor3f(0.9f, 0.9f, 0.9f);
    FilledRect(-14, -1.5, 28, 3);
    FilledRect(-1.5, -14, 3, 28);
    glPopMatrix();

    // Beacon light on top (rotating red/white feel via pulsing)
    float pulse = 0.5f + 0.5f * (float)sin(g_simTime * 5.0);
    glColor3f(1.0f, pulse * 0.2f, pulse * 0.2f);
    FilledCircle(baseX + 17, cabY + 40, 3);

    // Label
    glColor3f(1,1,1);
    DrawText(baseX - 45, baseY + 30, "ATC TOWER", GLUT_BITMAP_HELVETICA_12);
}

// ============================================================================
//  TERMINAL BUILDING + GATES + FIDS BOARD (structure only; dynamic content
//  such as boarding gate assignment drawn separately once aircraft exist)
// ============================================================================
static void DrawTerminalStructure() {
    // Main terminal block
    glColor3f(0.80f, 0.82f, 0.86f);
    FilledRect(140, TERMINAL_BOT, WORLD_W - 280, TERMINAL_TOP - TERMINAL_BOT);
    glColor3f(0.55f, 0.58f, 0.65f);
    FilledRect(140, TERMINAL_TOP - 10, WORLD_W - 280, 10); // roof band

    // Glass facade strips
    glColor3f(0.45f, 0.65f, 0.80f);
    for (double x = 160; x < WORLD_W - 160; x += 30) {
        FilledRect(x, TERMINAL_BOT + 10, 18, TERMINAL_TOP - TERMINAL_BOT - 30);
    }

    // Title on building
    glColor3f(0.15f, 0.15f, 0.20f);
    std::string title = "HAZRAT SHAHJALAL INTERNATIONAL AIRPORT";
    DrawText(WORLD_W / 2.0 - TextWidth(title, GLUT_BITMAP_HELVETICA_18) / 2.0, TERMINAL_TOP - 24, title);

    // Jet bridges + gate markers
    for (size_t i = 0; i < g_gates.size(); i++) {
        Gate& gt = g_gates[i];
        glColor3f(0.70f, 0.72f, 0.75f);
        FilledRect(gt.x - 10, TERMINAL_BOT - 22, 20, 22);
        glColor3f(0.15f, 0.15f, 0.15f);
        LineRect(gt.x - 10, TERMINAL_BOT - 22, 20, 22);
        glColor3f(1,1,0.4f);
        std::string lbl = gt.label;
        DrawText(gt.x - TextWidth(lbl, GLUT_BITMAP_HELVETICA_12) / 2.0, TERMINAL_BOT - 36, lbl, GLUT_BITMAP_HELVETICA_12);
    }

    // Windows lit at night
    if (g_timeOfDay == TOD_NIGHT) {
        for (double x = 165; x < WORLD_W - 165; x += 30) {
            float on = (sin(x * 12.9) > -0.3) ? 1.0f : 0.15f;
            glColor3f(1.0f * on, 0.92f * on, 0.55f * on);
            FilledRect(x, TERMINAL_BOT + 12, 14, 8);
        }
    }
}

// ============================================================================
//  APRON — GROUND VEHICLES PARKING AREA + FIRE STATION
// ============================================================================
static void DrawApronStructure() {
    glColor3f(0.30f, 0.31f, 0.33f);
    FilledRect(0, APRON_BOT, WORLD_W, APRON_TOP - APRON_BOT);
    glColor3f(0.95f, 0.85f, 0.2f);
    for (double x = 10; x < WORLD_W - 10; x += 26) {
        DDA_Line(x, APRON_BOT + 6, x + 14, APRON_BOT + 6);
    }

    // Fire station block
    glColor3f(0.65f, 0.12f, 0.12f);
    FilledRect(20, APRON_BOT + 2, 90, 34);
    glColor3f(0.95f, 0.95f, 0.95f);
    LineRect(20, APRON_BOT + 2, 90, 34);
    FilledRect(50, APRON_BOT + 2, 26, 20); // garage door
    glColor3f(0.2f,0.2f,0.2f);
    LineRect(50, APRON_BOT + 2, 26, 20);
    glColor3f(1,1,1);
    DrawText(24, APRON_BOT + 40, "FIRE & RESCUE", GLUT_BITMAP_HELVETICA_12);
}

// ============================================================================
//  ROAD NETWORK
// ============================================================================
static void InitRoadVehicles() {
    g_roadVehicles.clear();
    for (int i = 0; i < 10; i++) {
        RoadVehicle v;
        v.x = RandRange(0, WORLD_W);
        v.lane = i % 4;
        v.speed = RandRange(60, 130) * ((v.lane % 2 == 0) ? 1 : -1);
        v.isBus = (i % 4 == 0);
        if (v.isBus) { v.r = 0.85f; v.g = 0.15f; v.b = 0.15f; }
        else {
            float palette[4][3] = {{0.2f,0.3f,0.8f},{0.9f,0.9f,0.9f},{0.1f,0.6f,0.3f},{0.9f,0.6f,0.1f}};
            int c = rand() % 4;
            v.r = palette[c][0]; v.g = palette[c][1]; v.b = palette[c][2];
        }
        g_roadVehicles.push_back(v);
    }
}

static void DrawRoad() {
    glColor3f(0.12f, 0.12f, 0.13f);
    FilledRect(0, ROAD_BOT, WORLD_W, ROAD_TOP - ROAD_BOT);
    glColor3f(0.9f, 0.9f, 0.3f);
    double laneH = (ROAD_TOP - ROAD_BOT) / 4.0;
    for (int lane = 1; lane < 4; lane++) {
        double y = ROAD_BOT + lane * laneH;
        for (double x = 0; x < WORLD_W; x += 30) {
            DDA_Line(x, y, x + 16, y);
        }
    }
    glColor3f(1,1,1);
    DrawText(10, ROAD_TOP - 16, "AIRPORT ROAD NETWORK", GLUT_BITMAP_HELVETICA_12);

    // Parking / drop-off / pickup zone labels
    glColor3f(0.75f, 0.9f, 1.0f);
    DrawText(WORLD_W - 260, ROAD_BOT + 6, "DROP-OFF | PICKUP | PARKING", GLUT_BITMAP_HELVETICA_12);
}

static void UpdateRoadVehicles(double dt) {
    for (size_t i = 0; i < g_roadVehicles.size(); i++) {
        g_roadVehicles[i].x += g_roadVehicles[i].speed * dt;
        if (g_roadVehicles[i].x > WORLD_W + 30) g_roadVehicles[i].x = -30;
        if (g_roadVehicles[i].x < -30) g_roadVehicles[i].x = WORLD_W + 30;
    }
}

static void DrawRoadVehicle(const RoadVehicle& v) {
    double laneH = (ROAD_TOP - ROAD_BOT) / 4.0;
    double y = ROAD_BOT + v.lane * laneH + laneH / 2.0 - 5;
    glColor3f(v.r, v.g, v.b);
    double w = v.isBus ? 34 : 20, h = v.isBus ? 12 : 9;
    FilledRect(v.x - w/2, y, w, h);
    glColor3f(0.7f, 0.85f, 0.95f);
    FilledRect(v.x - w/2 + 3, y + h - 5, w - 6, 4);
    glColor3f(0.05f,0.05f,0.05f);
    FilledCircle(v.x - w/2 + 4, y - 1, 2.2);
    FilledCircle(v.x + w/2 - 4, y - 1, 2.2);
}

static void DrawRoadVehicles() {
    for (size_t i = 0; i < g_roadVehicles.size(); i++) DrawRoadVehicle(g_roadVehicles[i]);
}


// ============================================================================
//  AIRCRAFT RENDERING
// ============================================================================
static void DrawAircraft(const Aircraft& ac) {
    const AircraftType& t = g_aircraftTypes[ac.typeIndex];

    glPushMatrix();
    glTranslatef((float)ac.pos.x, (float)ac.pos.y, 0);
    glRotatef((float)ac.heading, 0, 0, 1);

    double scale = 1.0 + 0.15 * ac.altitude; // slight visual growth when airborne (perspective flavor)
    glScalef((float)scale, (float)scale, 1.0f);

    double len = t.bodyLength * 0.42;   // half-length for centering
    double wid = t.bodyWidth * 0.5;
    double span = t.wingSpan * 0.5;

    // --- Wings (drawn first, sit behind fuselage) ---
    glColor3f(t.r * 0.92f, t.g * 0.92f, t.b * 0.92f);
    FilledTriangle(-len * 0.05, 0, -len * 0.55, span, -len * 0.30, 0);
    FilledTriangle(-len * 0.05, 0, -len * 0.55, -span, -len * 0.30, 0);

    // --- Horizontal stabilizer (tail wings) ---
    glColor3f(t.tailR, t.tailG, t.tailB);
    FilledTriangle(len * 0.75, 0, len, wid * 1.4, len, -wid * 1.4);

    // --- Fuselage (cylinder-ish via rounded rect + nose cone) ---
    glColor3f(t.r, t.g, t.b);
    FilledRect(-len, -wid * 0.5, len * 2, wid);
    FilledCircle(len, 0, wid * 0.5, 16);      // nose
    FilledCircle(-len, 0, wid * 0.5, 16);     // tail cap

    // Cockpit windows
    glColor3f(0.15f, 0.25f, 0.35f);
    FilledRect(len * 0.72, -wid * 0.28, len * 0.18, wid * 0.56);

    // Passenger windows strip
    glColor3f(0.20f, 0.30f, 0.42f);
    for (double wx = -len * 0.85; wx < len * 0.65; wx += len * 0.14) {
        FilledRect(wx, wid * 0.08, len * 0.06, wid * 0.16);
    }

    // Vertical tail fin
    glColor3f(t.tailR, t.tailG, t.tailB);
    FilledTriangle(len * 0.72, wid * 0.5, len * 1.02, wid * 0.5, len * 0.85, wid * 2.4);

    // Livery cheatline stripe
    glColor3f(t.tailR, t.tailG, t.tailB);
    FilledRect(-len * 0.9, -wid * 0.08, len * 1.8, wid * 0.16);

    // Engines — jets get pods; ATR turboprops get spinning props
    bool isTurboprop = (t.name.find("ATR") != std::string::npos) || (t.name == "Novoair");
    if (isTurboprop) {
        for (int s = -1; s <= 1; s += 2) {
            double ey = s * span * 0.5;
            glColor3f(0.25f, 0.25f, 0.28f);
            FilledRect(len * 0.05, ey - wid * 0.18, len * 0.35, wid * 0.36);
            glPushMatrix();
            glTranslatef((float)(len * 0.42), (float)ey, 0);
            glRotatef((float)ac.propSpin, 0, 0, 1);
            glColor3f(0.08f, 0.08f, 0.08f);
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glVertex2d(0, -wid * 0.9); glVertex2d(0, wid * 0.9);
            glVertex2d(-wid * 0.9, 0); glVertex2d(wid * 0.9, 0);
            glEnd();
            glPopMatrix();
        }
    } else {
        for (int s = -1; s <= 1; s += 2) {
            double ey = s * span * 0.55;
            glColor3f(0.30f, 0.30f, 0.34f);
            FilledRect(-len * 0.30, ey - wid * 0.22, len * 0.4, wid * 0.44);
            glColor3f(0.10f, 0.10f, 0.12f);
            FilledCircle(-len * 0.30, ey, wid * 0.22, 12);
            // engine glow when at high phases
            if (ac.phase == PHASE_TAKEOFF_ROLL || ac.phase == PHASE_CLIMB) {
                glColor3f(1.0f, 0.6f, 0.2f);
                FilledCircle(-len * 0.34, ey, wid * 0.12, 10);
            }
        }
    }

    // Flaps (visual only, angle down during approach/landing)
    if (ac.flapAngle > 0.5) {
        glColor3f(0.6f, 0.6f, 0.62f);
        FilledRect(-len * 0.5, -span * 0.35, len * 0.15, 3);
        FilledRect(-len * 0.5, span * 0.35, len * 0.15, 3);
    }

    // Landing gear / wheels (visible when on ground or low altitude)
    if (ac.altitude < 0.3) {
        glColor3f(0.05f, 0.05f, 0.05f);
        MidpointCircle(len * 0.55, -wid * 0.55, 3.0);
        MidpointCircle(-len * 0.35, -wid * 0.7, 3.5);
        MidpointCircle(-len * 0.35, wid * 0.7, 3.5);
        FilledCircle(len * 0.55, -wid * 0.55, 2.6, 10);
        FilledCircle(-len * 0.35, -wid * 0.7, 3.0, 10);
        FilledCircle(-len * 0.35, wid * 0.7, 3.0, 10);
    }

    // Navigation / anti-collision / landing lights
    if (ac.lightsOn) {
        glColor3f(1.0f, 0.15f, 0.15f);
        FilledCircle(0, wid * 2.4, 1.6);            // red on left wingtip area (visual simplification)
        glColor3f(0.15f, 1.0f, 0.15f);
        FilledCircle(0, -wid * 2.4, 1.6);           // green
        float blink = (sin(g_simTime * 10.0) > 0) ? 1.0f : 0.15f;
        glColor3f(1.0f * blink, 1.0f * blink, 1.0f * blink);
        FilledCircle(-len * 0.2, 0, 1.4);            // anti-collision strobe (top fuselage)
        if (ac.phase == PHASE_APPROACH || ac.phase == PHASE_TOUCHDOWN || ac.phase == PHASE_TAKEOFF_ROLL) {
            glColor3f(1.0f, 1.0f, 0.85f);
            FilledCircle(len * 0.95, 0, 1.6);        // landing light nose
        }
    }

    glPopMatrix();

    // Flight label above aircraft (world space, not rotated)
    glColor3f(1,1,1);
    std::string lbl = ac.flightNumber;
    DrawText(ac.pos.x - TextWidth(lbl, GLUT_BITMAP_HELVETICA_12) / 2.0, ac.pos.y + 26 + ac.altitude * 14, lbl, GLUT_BITMAP_HELVETICA_12);
}

static void DrawAllAircraft() {
    for (size_t i = 0; i < g_aircraft.size(); i++) DrawAircraft(g_aircraft[i]);
}

// ============================================================================
//  GROUND VEHICLE RENDERING
// ============================================================================
static void DrawGroundVehicle(const GroundVehicle& v) {
    glPushMatrix();
    glTranslatef((float)v.pos.x, (float)v.pos.y, 0);

    switch (v.type) {
        case VEH_FUEL:
            glColor3f(0.85f, 0.75f, 0.15f);
            FilledRect(-14, 0, 28, 12);
            glColor3f(0.3f,0.3f,0.3f);
            FilledCircle(0, 12, 8, 14); // tank
            glColor3f(0.1f,0.1f,0.1f);
            FilledCircle(-8, -2, 3, 10); FilledCircle(8, -2, 3, 10);
            DrawText(-10, 18, "FUEL", GLUT_BITMAP_HELVETICA_12);
            break;
        case VEH_BAGGAGE:
            glColor3f(0.2f, 0.45f, 0.85f);
            FilledRect(-16, 0, 14, 10);
            glColor3f(0.7f,0.55f,0.3f);
            FilledRect(0, 2, 20, 7);
            FilledRect(6, 6, 18, 6);
            glColor3f(0.1f,0.1f,0.1f);
            FilledCircle(-10, -2, 3, 10); FilledCircle(10, -2, 3, 10); FilledCircle(18,-2,3,10);
            break;
        case VEH_CATERING:
            glColor3f(0.85f, 0.85f, 0.9f);
            FilledRect(-15, 0, 30, 14);
            glColor3f(0.5f,0.5f,0.55f);
            FilledRect(-8, 14, 16, 8); // lift box
            glColor3f(0.1f,0.1f,0.1f);
            FilledCircle(-9, -2, 3, 10); FilledCircle(9, -2, 3, 10);
            break;
        case VEH_PUSHBACK:
            glColor3f(0.9f, 0.5f, 0.1f);
            FilledRect(-14, 0, 28, 10);
            glColor3f(0.2f,0.2f,0.2f);
            FilledRect(-18, 2, 6, 6);
            glColor3f(0.1f,0.1f,0.1f);
            FilledCircle(-9, -2, 3.4, 10); FilledCircle(9, -2, 3.4, 10);
            break;
        case VEH_FOLLOWME:
            glColor3f(0.95f, 0.95f, 0.15f);
            FilledRect(-13, 0, 26, 10);
            glColor3f(0.1f,0.1f,0.1f);
            DrawText(-18, 14, "FOLLOW ME", GLUT_BITMAP_HELVETICA_12);
            FilledCircle(-8, -2, 3, 10); FilledCircle(8, -2, 3, 10);
            break;
        case VEH_SHUTTLE:
            glColor3f(0.15f, 0.55f, 0.25f);
            FilledRect(-20, 0, 40, 16);
            glColor3f(0.7f,0.85f,0.95f);
            FilledRect(-16, 10, 32, 6);
            glColor3f(0.1f,0.1f,0.1f);
            FilledCircle(-12, -2, 3.5, 10); FilledCircle(12, -2, 3.5, 10);
            break;
        case VEH_FIRE:
            glColor3f(0.85f, 0.1f, 0.1f);
            FilledRect(-18, 0, 36, 16);
            glColor3f(1.0f,1.0f,1.0f);
            FilledRect(-18, 6, 36, 4);
            glColor3f(0.1f,0.1f,0.1f);
            FilledCircle(-12, -2, 4, 10); FilledCircle(12, -2, 4, 10);
            {
                float blink = (sin(g_simTime * 12.0) > 0) ? 1.0f : 0.2f;
                glColor3f(1.0f * blink, 0.1f, 0.1f);
                FilledCircle(0, 18, 2.2);
            }
            break;
    }
    glPopMatrix();
}

static void DrawAllVehicles() {
    for (size_t i = 0; i < g_vehicles.size(); i++) {
        if (g_vehicles[i].active) DrawGroundVehicle(g_vehicles[i]);
    }
}


// ============================================================================
//  FLIGHT NUMBER / DESTINATION GENERATION
// ============================================================================
static std::string GenerateFlightNumber(AircraftCategory cat) {
    const char* prefixDom[]  = { "BG", "BS", "VQ" };
    const char* prefixIntl[] = { "EK", "QR", "SQ", "TG", "TK", "CX" };
    const char* prefixCargo[]= { "D0", "FX" };
    std::ostringstream oss;
    if (cat == CAT_DOMESTIC)      oss << prefixDom[rand() % 3];
    else if (cat == CAT_CARGO)    oss << prefixCargo[rand() % 2];
    else                            oss << prefixIntl[rand() % 6];
    oss << "-" << (100 + rand() % 800);
    return oss.str();
}

static std::string GenerateDestination(AircraftCategory cat) {
    const char* dom[]  = { "Chittagong", "Sylhet", "Cox's Bazar", "Rajshahi", "Saidpur" };
    const char* intl[] = { "Dubai", "Doha", "Singapore", "Bangkok", "Istanbul", "London", "Kuala Lumpur", "Kolkata" };
    const char* cargo[]= { "Leipzig Hub", "Memphis Hub", "Dubai Cargo", "Singapore Cargo" };
    if (cat == CAT_DOMESTIC) return dom[rand() % 5];
    if (cat == CAT_CARGO)    return cargo[rand() % 4];
    return intl[rand() % 8];
}

// ============================================================================
//  FIDS HELPERS
// ============================================================================
static std::string FormatSimClock() {
    double totalMinutes = fmod(g_dayClock * 24.0 * 60.0, 24.0 * 60.0);
    int hh = (int)(totalMinutes / 60.0);
    int mm = (int)totalMinutes % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hh << ":" << std::setw(2) << std::setfill('0') << mm;
    return oss.str();
}

static void PushFIDS(const Aircraft& ac, const std::string& status) {
    FIDSEntry e;
    e.flightNumber = ac.flightNumber;
    e.destinationOrOrigin = ac.destination;
    e.status = status;
    e.gate = (ac.gateIndex >= 0) ? g_gates[ac.gateIndex].label : "--";
    e.time = FormatSimClock();
    e.isArrival = ac.isArrival;
    g_fids.push_front(e);
    if (g_fids.size() > 6) g_fids.pop_back();
}

static void UpdateFIDSStatusFor(int aircraftId, const std::string& status) {
    for (size_t i = 0; i < g_fids.size(); i++) {
        // FIDS entries don't store id directly; match by flight number against current aircraft
    }
    for (size_t a = 0; a < g_aircraft.size(); a++) {
        if (g_aircraft[a].id == aircraftId) {
            for (size_t i = 0; i < g_fids.size(); i++) {
                if (g_fids[i].flightNumber == g_aircraft[a].flightNumber) {
                    g_fids[i].status = status;
                    break;
                }
            }
            break;
        }
    }
}

// ============================================================================
//  GATE ALLOCATION
// ============================================================================
static int FindFreeGate() {
    for (size_t i = 0; i < g_gates.size(); i++) {
        if (!g_gates[i].occupied) return (int)i;
    }
    return -1;
}

// ============================================================================
//  VEHICLE DISPATCH — spawns servicing vehicles once an aircraft docks
// ============================================================================
static void DispatchServiceVehicles(int gateIndex) {
    if (gateIndex < 0 || gateIndex >= (int)g_gates.size()) return;
    Gate& g = g_gates[gateIndex];
    double apronY = APRON_BOT + 10 + (gateIndex % 2) * 18;

    GroundVehicle fuel;
    fuel.type = VEH_FUEL; fuel.pos = Vec2(g.x - 60, apronY); fuel.target = Vec2(g.x - 20, apronY);
    fuel.speed = 40; fuel.active = true; fuel.assignedGate = gateIndex; fuel.taskTimer = 6.0; fuel.label = "Fuel";
    g_vehicles.push_back(fuel);

    GroundVehicle bag;
    bag.type = VEH_BAGGAGE; bag.pos = Vec2(g.x + 60, apronY - 12); bag.target = Vec2(g.x + 15, apronY - 12);
    bag.speed = 46; bag.active = true; bag.assignedGate = gateIndex; bag.taskTimer = 5.0; bag.label = "Baggage";
    g_vehicles.push_back(bag);

    GroundVehicle cat;
    cat.type = VEH_CATERING; cat.pos = Vec2(g.x + 40, apronY + 20); cat.target = Vec2(g.x + 10, apronY + 15);
    cat.speed = 38; cat.active = true; cat.assignedGate = gateIndex; cat.taskTimer = 5.5; cat.label = "Catering";
    g_vehicles.push_back(cat);
}

static void DispatchPushback(int gateIndex) {
    if (gateIndex < 0) return;
    Gate& g = g_gates[gateIndex];
    GroundVehicle pb;
    pb.type = VEH_PUSHBACK; pb.pos = Vec2(g.x, APRON_TOP - 6); pb.target = Vec2(g.x, TAXIWAY_TOP + 10);
    pb.speed = 30; pb.active = true; pb.assignedGate = gateIndex; pb.taskTimer = 4.0; pb.label = "Pushback";
    g_vehicles.push_back(pb);
}

static void DispatchFireTrucks() {
    GroundVehicle f1;
    f1.type = VEH_FIRE; f1.pos = Vec2(65, APRON_BOT + 20); f1.target = Vec2(g_emergency.location.x, RUNWAY_BOT - 15);
    f1.speed = 220; f1.active = true; f1.assignedGate = -1; f1.taskTimer = 8.0; f1.label = "Fire-1";
    g_vehicles.push_back(f1);
    PushLog("Fire & Rescue dispatched to incident location.", 2);
}

static void UpdateVehicles(double dt) {
    for (size_t i = 0; i < g_vehicles.size(); i++) {
        GroundVehicle& v = g_vehicles[i];
        if (!v.active) continue;
        Vec2 delta = v.target - v.pos;
        double dist = sqrt(delta.x * delta.x + delta.y * delta.y);
        if (dist > 2.0) {
            Vec2 dir = delta * (1.0 / dist);
            v.pos = v.pos + dir * (v.speed * dt);
        } else {
            v.taskTimer -= dt;
            if (v.taskTimer <= 0) v.active = false;
        }
    }
    // Cleanup finished vehicles occasionally
    for (size_t i = 0; i < g_vehicles.size();) {
        if (!g_vehicles[i].active) g_vehicles.erase(g_vehicles.begin() + i);
        else i++;
    }
}


// ============================================================================
//  AIRCRAFT SPAWNING
// ============================================================================
static void SpawnDeparture() {
    int gateIdx = FindFreeGate();
    if (gateIdx < 0) return; // no free gate, skip this cycle

    Aircraft ac;
    ac.id = g_nextAircraftId++;
    ac.typeIndex = rand() % g_aircraftTypes.size();
    AircraftCategory cat = g_aircraftTypes[ac.typeIndex].category;
    ac.flightNumber = GenerateFlightNumber(cat);
    ac.destination = GenerateDestination(cat);
    ac.isArrival = false;
    ac.phase = PHASE_HOLDING;
    ac.pos = Vec2(g_gates[gateIdx].x, g_gates[gateIdx].y);
    ac.heading = 90; // facing north into apron initially
    ac.speed = 0;
    ac.targetSpeed = 0;
    ac.altitude = 0;
    ac.gateIndex = gateIdx;
    ac.phaseTimer = 0;
    ac.bezierT = 0;
    ac.lightsOn = true;
    ac.flapAngle = 0;
    ac.propSpin = 0;
    ac.statusText = "Boarding";

    g_gates[gateIdx].occupied = true;
    g_gates[gateIdx].aircraftId = ac.id;

    g_aircraft.push_back(ac);
    PushFIDS(ac, "Boarding");
    PushLog("Flight " + ac.flightNumber + " to " + ac.destination + " boarding at " + g_gates[gateIdx].label + ".");
}

static void SpawnArrival() {
    Aircraft ac;
    ac.id = g_nextAircraftId++;
    ac.typeIndex = rand() % g_aircraftTypes.size();
    AircraftCategory cat = g_aircraftTypes[ac.typeIndex].category;
    ac.flightNumber = GenerateFlightNumber(cat);
    ac.destination = GenerateDestination(cat); // used as "origin" for arrivals
    ac.isArrival = true;
    ac.phase = PHASE_APPROACH;
    ac.pos = Vec2(-80, SKY_BOTTOM + 40);
    ac.heading = 180; // approaching from the east, facing west (toward runway)
    ac.speed = 140;
    ac.targetSpeed = 140;
    ac.altitude = 1.0;
    ac.gateIndex = -1;
    ac.phaseTimer = 0;
    ac.bezierT = 0;
    ac.lightsOn = true;
    ac.flapAngle = 1.0;
    ac.propSpin = 0;
    ac.statusText = "Approaching";

    g_runway.landingQueue.push_back(ac.id);
    g_aircraft.push_back(ac);
    PushFIDS(ac, "Approaching");
    PushLog("Flight " + ac.flightNumber + " from " + ac.destination + " requesting approach clearance.");
}

// ============================================================================
//  AIRCRAFT STATE MACHINE UPDATE
// ============================================================================
static Aircraft* FindAircraftById(int id) {
    for (size_t i = 0; i < g_aircraft.size(); i++) if (g_aircraft[i].id == id) return &g_aircraft[i];
    return NULL;
}

static double WeatherSpeedMultiplier() {
    switch (g_weather) {
        case WEATHER_RAIN: return 0.88;
        case WEATHER_HEAVY_RAIN: return 0.70;
        case WEATHER_FOG: return 0.75;
        case WEATHER_STORM: return 0.55;
        default: return 1.0;
    }
}

static void UpdateAircraft(Aircraft& ac, double dt) {
    ac.phaseTimer += dt;
    ac.propSpin = fmod(ac.propSpin + dt * 900.0, 360.0);
    double wMul = WeatherSpeedMultiplier();

    switch (ac.phase) {
        case PHASE_HOLDING: {
            // Waiting at gate for boarding to complete
            if (ac.phaseTimer > 6.0) {
                ac.phase = PHASE_PUSHBACK;
                ac.phaseTimer = 0;
                DispatchPushback(ac.gateIndex);
                ac.statusText = "Pushback";
                UpdateFIDSStatusFor(ac.id, "Pushback");
                PushLog("Flight " + ac.flightNumber + " pushback initiated.");
            }
            break;
        }
        case PHASE_PUSHBACK: {
            double targetY = TAXIWAY_TOP + 10;
            ac.pos.y = Lerp(ac.pos.y, targetY, dt * 0.8);
            ac.heading = 270;
            if (ac.phaseTimer > 4.0) {
                ac.phase = PHASE_TAXI_OUT;
                ac.phaseTimer = 0;
                ac.heading = (ac.pos.x < WORLD_W / 2.0) ? 0 : 180;
                ac.statusText = "Taxiing";
                UpdateFIDSStatusFor(ac.id, "Taxiing");
                if (ac.gateIndex >= 0) {
                    g_gates[ac.gateIndex].occupied = false;
                    g_gates[ac.gateIndex].aircraftId = -1;
                }
            }
            break;
        }
        case PHASE_TAXI_OUT: {
            double taxiY = (TAXIWAY_TOP + TAXIWAY_BOT) / 2.0;
            ac.pos.y = Lerp(ac.pos.y, taxiY, dt * 1.2);
            double targetX = WORLD_W / 2.0;
            double dirX = (targetX > ac.pos.x) ? 1 : -1;
            ac.pos.x += dirX * 55 * wMul * dt;
            ac.heading = (dirX > 0) ? 0 : 180;
            if (fabs(ac.pos.x - targetX) < 30) {
                ac.phase = PHASE_HOLDING_SHORT;
                ac.phaseTimer = 0;
                g_runway.takeoffQueue.push_back(ac.id);
                ac.statusText = "Holding Short";
                UpdateFIDSStatusFor(ac.id, "Holding Short");
                PushLog("Flight " + ac.flightNumber + " holding short of runway.");
            }
            break;
        }
        case PHASE_HOLDING_SHORT: {
            // Wait until ATC clears via runway queue logic (handled globally)
            break;
        }
        case PHASE_TAKEOFF_ROLL: {
            ac.pos.y = Lerp(ac.pos.y, (RUNWAY_TOP + RUNWAY_BOT) / 2.0, dt * 2.0);
            ac.heading = 0;
            ac.speed = std::min(ac.speed + 60 * wMul * dt, 220.0);
            ac.pos.x += ac.speed * dt;
            ac.flapAngle = 1.0;
            if (ac.speed > 190) {
                ac.phase = PHASE_CLIMB;
                ac.phaseTimer = 0;
                ac.statusText = "Departed";
                UpdateFIDSStatusFor(ac.id, "Departed");
                PushLog("Flight " + ac.flightNumber + " airborne, climbing out.");
                g_stats.flightsHandled++;
            }
            if (ac.pos.x > WORLD_W + 100) {
                ac.phase = PHASE_DEPARTED;
            }
            break;
        }
        case PHASE_CLIMB: {
            ac.pos.x += ac.speed * dt;
            ac.altitude = std::min(ac.altitude + dt * 0.4, 1.0);
            ac.pos.y += 30 * dt;
            ac.flapAngle = std::max(0.0, ac.flapAngle - dt * 0.3);
            if (ac.pos.x > WORLD_W + 120 || ac.pos.y > SKY_TOP + 40) ac.phase = PHASE_DEPARTED;
            break;
        }
        case PHASE_APPROACH: {
            // Bezier curve sweeping from off-screen sky down onto the runway threshold
            ac.bezierT += dt * 0.09 * wMul;
            Vec2 p0(-80, SKY_BOTTOM + 40);
            Vec2 p1(WORLD_W * 0.35, SKY_BOTTOM - 10);
            Vec2 p2(WORLD_W * 0.65, RUNWAY_TOP + 40);
            Vec2 p3(WORLD_W - 120, (RUNWAY_TOP + RUNWAY_BOT) / 2.0);
            Vec2 pos = CubicBezier(p0, p1, p2, p3, Clamp(ac.bezierT, 0, 1));
            Vec2 prev = CubicBezier(p0, p1, p2, p3, Clamp(ac.bezierT - 0.01, 0, 1));
            Vec2 d = pos - prev;
            if (fabs(d.x) > 0.001 || fabs(d.y) > 0.001) {
                ac.heading = atan2(d.y, d.x) * 180.0 / PI;
            }
            ac.pos = pos;
            ac.altitude = Clamp(1.0 - ac.bezierT, 0, 1);
            if (ac.bezierT >= 1.0) {
                ac.phase = PHASE_TOUCHDOWN;
                ac.phaseTimer = 0;
                ac.altitude = 0;
                ac.heading = 180;
                ac.statusText = "Landed";
                UpdateFIDSStatusFor(ac.id, "Landed");
                PushLog("Flight " + ac.flightNumber + " touched down on Runway 14/32.");
                // Remove from landing queue & free runway shortly
                for (size_t i = 0; i < g_runway.landingQueue.size(); i++) {
                    if (g_runway.landingQueue[i] == ac.id) { g_runway.landingQueue.erase(g_runway.landingQueue.begin() + i); break; }
                }
            }
            break;
        }
        case PHASE_TOUCHDOWN: {
            ac.speed = std::max(ac.speed - 90 * dt, 30.0);
            ac.pos.x -= ac.speed * dt;
            ac.flapAngle = std::max(0.0, ac.flapAngle - dt * 0.5);
            if (ac.speed <= 30.0 && ac.phaseTimer > 2.0) {
                ac.phase = PHASE_TAXI_IN;
                ac.phaseTimer = 0;
                ac.statusText = "Taxiing to Gate";
                UpdateFIDSStatusFor(ac.id, "Taxiing to Gate");
                g_stats.flightsHandled++;
            }
            break;
        }
        case PHASE_TAXI_IN: {
            double taxiY = (TAXIWAY_TOP + TAXIWAY_BOT) / 2.0;
            ac.pos.y = Lerp(ac.pos.y, taxiY, dt * 1.0);
            int gateIdx = ac.gateIndex;
            if (gateIdx < 0) {
                gateIdx = FindFreeGate();
                if (gateIdx >= 0) {
                    ac.gateIndex = gateIdx;
                    g_gates[gateIdx].occupied = true;
                    g_gates[gateIdx].aircraftId = ac.id;
                }
            }
            if (gateIdx >= 0) {
                double targetX = g_gates[gateIdx].x;
                double dirX = (targetX > ac.pos.x) ? 1 : -1;
                ac.pos.x += dirX * 45 * wMul * dt;
                ac.heading = (dirX > 0) ? 0 : 180;
                if (fabs(ac.pos.x - targetX) < 10) {
                    ac.phase = PHASE_AT_GATE;
                    ac.phaseTimer = 0;
                    ac.pos.y = g_gates[gateIdx].y;
                    ac.statusText = "At Gate";
                    UpdateFIDSStatusFor(ac.id, "Arrived");
                    DispatchServiceVehicles(gateIdx);
                    PushLog("Flight " + ac.flightNumber + " docked at " + g_gates[gateIdx].label + ".");
                }
            } else {
                // No free gate yet — hold position near taxiway
                ac.pos.x -= 10 * dt;
            }
            break;
        }
        case PHASE_AT_GATE: {
            ac.speed = 0;
            if (ac.phaseTimer > 10.0) {
                ac.phase = PHASE_DEPARTED; // deboarded, remove from sim
            }
            break;
        }
        case PHASE_DEPARTED:
        default:
            break;
    }
}

static void UpdateAllAircraft(double dt) {
    for (size_t i = 0; i < g_aircraft.size(); i++) UpdateAircraft(g_aircraft[i], dt);

    // Remove departed aircraft & free gates if needed
    for (size_t i = 0; i < g_aircraft.size();) {
        if (g_aircraft[i].phase == PHASE_DEPARTED) {
            if (g_aircraft[i].gateIndex >= 0 && g_aircraft[i].gateIndex < (int)g_gates.size()) {
                Gate& g = g_gates[g_aircraft[i].gateIndex];
                if (g.aircraftId == g_aircraft[i].id) { g.occupied = false; g.aircraftId = -1; }
            }
            g_aircraft.erase(g_aircraft.begin() + i);
        } else {
            i++;
        }
    }
}

// ============================================================================
//  RUNWAY / ATC SEQUENCING LOGIC
//  Only one aircraft may occupy the runway. Landings take priority over
//  takeoffs (standard real-world ATC practice) unless emergency overrides.
// ============================================================================
static void UpdateRunwayLogic(double dt) {
    // Determine if runway is currently physically occupied by scanning aircraft phases
    bool physicallyOccupied = false;
    int occupant = -1;
    for (size_t i = 0; i < g_aircraft.size(); i++) {
        AircraftPhase ph = g_aircraft[i].phase;
        if (ph == PHASE_TAKEOFF_ROLL || ph == PHASE_TOUCHDOWN) {
            physicallyOccupied = true;
            occupant = g_aircraft[i].id;
            break;
        }
    }
    g_runway.occupied = physicallyOccupied;
    g_runway.occupantAircraftId = occupant;

    if (g_emergency.active && g_emergency.type == EMG_RUNWAY_OBSTRUCTION) {
        return; // no clearances issued during a runway obstruction
    }

    if (!physicallyOccupied) {
        // Landing queue has priority
        if (!g_runway.landingQueue.empty()) {
            int id = g_runway.landingQueue.front();
            Aircraft* ac = FindAircraftById(id);
            if (ac && ac->phase == PHASE_APPROACH && ac->bezierT > 0.85) {
                // allow it to continue — it's already committed to final approach
            }
        } else if (!g_runway.takeoffQueue.empty()) {
            int id = g_runway.takeoffQueue.front();
            Aircraft* ac = FindAircraftById(id);
            if (ac && ac->phase == PHASE_HOLDING_SHORT) {
                ac->phase = PHASE_TAKEOFF_ROLL;
                ac->phaseTimer = 0;
                ac->speed = 0;
                ac->statusText = "Takeoff Roll";
                UpdateFIDSStatusFor(ac->id, "Departing");
                g_runway.takeoffQueue.pop_front();
                PushLog("ATC: Flight " + ac->flightNumber + " cleared for takeoff, Runway 14/32.");
            }
        }
    }
    if (physicallyOccupied) g_stats.runwayBusySeconds += dt;
}


// ============================================================================
//  EMERGENCY MANAGEMENT SYSTEM
// ============================================================================
static const char* EmergencyName(EmergencyType t) {
    switch (t) {
        case EMG_BIRD_STRIKE:         return "Bird Strike";
        case EMG_ENGINE_FAILURE:      return "Engine Failure";
        case EMG_FUEL_LEAK:           return "Fuel Leakage";
        case EMG_MEDICAL:             return "Medical Emergency";
        case EMG_FIRE:                return "Fire Incident";
        case EMG_RUNWAY_OBSTRUCTION:  return "Runway Obstruction";
        case EMG_EMERGENCY_LANDING:   return "Emergency Landing";
        default:                        return "None";
    }
}

static void TriggerRandomEmergency() {
    if (g_emergency.active) return;
    EmergencyType types[] = { EMG_BIRD_STRIKE, EMG_ENGINE_FAILURE, EMG_FUEL_LEAK, EMG_MEDICAL, EMG_FIRE, EMG_RUNWAY_OBSTRUCTION, EMG_EMERGENCY_LANDING };
    EmergencyType t = types[rand() % 7];
    g_emergency.active = true;
    g_emergency.type = t;
    g_emergency.timer = 0;
    g_emergency.duration = RandRange(9.0, 15.0);
    g_emergency.location = Vec2(RandRange(200, WORLD_W - 200), (RUNWAY_TOP + RUNWAY_BOT) / 2.0);
    g_emergency.description = std::string("EMERGENCY: ") + EmergencyName(t) + " reported!";

    PushLog(g_emergency.description, 2);
    PushLog("ATC notified. Coordinating emergency response.", 1);
    DispatchFireTrucks();
    g_stats.emergenciesHandled++;
}

static void UpdateEmergency(double dt) {
    if (!g_emergency.active) return;
    g_emergency.timer += dt;
    if (g_emergency.timer >= g_emergency.duration) {
        g_emergency.active = false;
        PushLog("Emergency resolved. Normal operations resumed.", 0);
    }
}

// ============================================================================
//  DAY / NIGHT CYCLE LOGIC
// ============================================================================
static void UpdateDayNightCycle(double dt) {
    if (g_autoDayNight) {
        g_dayClock += dt / 180.0; // full cycle every 180 seconds
        if (g_dayClock > 1.0) g_dayClock -= 1.0;
    }
    if (g_dayClock < 0.22)      g_timeOfDay = TOD_NIGHT;
    else if (g_dayClock < 0.30) g_timeOfDay = TOD_MORNING;
    else if (g_dayClock < 0.62) g_timeOfDay = TOD_AFTERNOON;
    else if (g_dayClock < 0.75) g_timeOfDay = TOD_EVENING;
    else                          g_timeOfDay = TOD_NIGHT;
}

// ============================================================================
//  FLIGHT SCHEDULER — periodically spawns arrivals/departures
// ============================================================================
static double g_nextSpawnTimer = 3.0;

static void UpdateScheduler(double dt) {
    g_nextSpawnTimer -= dt;
    if (g_nextSpawnTimer <= 0) {
        g_nextSpawnTimer = RandRange(7.0, 13.0);
        if (g_aircraft.size() < 9) {
            if (rand() % 2 == 0) SpawnArrival();
            else SpawnDeparture();
        }
    }
}


// ============================================================================
//  FLIGHT INFORMATION DISPLAY BOARD (rendered as a panel over the terminal)
// ============================================================================
static void DrawFIDSBoard() {
    double bx = WORLD_W / 2.0 - 190, by = TERMINAL_TOP + 6, bw = 380, bh = 118;
    glColor3f(0.05f, 0.08f, 0.10f);
    FilledRect(bx, by, bw, bh);
    glColor3f(0.15f, 0.85f, 0.95f);
    LineRect(bx, by, bw, bh);

    glColor3f(0.2f, 0.95f, 1.0f);
    DrawText(bx + 8, by + bh - 16, "FLIGHT INFORMATION DISPLAY", GLUT_BITMAP_HELVETICA_12);
    DrawText(bx + 8, by + bh - 32, "FLT      DEST/ORIG      STATUS         GATE   TIME", GLUT_BITMAP_HELVETICA_12);

    double rowY = by + bh - 48;
    for (size_t i = 0; i < g_fids.size() && i < 5; i++) {
        const FIDSEntry& e = g_fids[i];
        float r = 0.85f, g = 0.95f, b = 0.95f;
        if (e.status.find("Delay") != std::string::npos) { r = 1.0f; g = 0.6f; b = 0.2f; }
        glColor3f(r, g, b);
        std::ostringstream line;
        line << std::left << std::setw(8) << e.flightNumber
             << std::setw(15) << e.destinationOrOrigin.substr(0, 13)
             << std::setw(15) << e.status.substr(0, 13)
             << std::setw(7) << e.gate
             << e.time;
        DrawText(bx + 8, rowY, line.str(), GLUT_BITMAP_HELVETICA_12);
        rowY -= 15;
    }
    if (g_fids.empty()) {
        glColor3f(0.6f, 0.6f, 0.6f);
        DrawText(bx + 8, rowY, "No active flights scheduled.", GLUT_BITMAP_HELVETICA_12);
    }
}

// ============================================================================
//  OPERATIONS DASHBOARD (HUD overlay — top-left, top-right, bottom panels)
// ============================================================================
static std::string WeatherName(WeatherType w) {
    switch (w) {
        case WEATHER_CLEAR: return "Clear Sky";
        case WEATHER_RAIN: return "Rain";
        case WEATHER_HEAVY_RAIN: return "Heavy Rain";
        case WEATHER_FOG: return "Fog";
        case WEATHER_STORM: return "Thunderstorm";
        default: return "?";
    }
}
static std::string TimeOfDayName(TimeOfDay t) {
    switch (t) {
        case TOD_MORNING: return "Morning";
        case TOD_AFTERNOON: return "Afternoon";
        case TOD_EVENING: return "Evening";
        case TOD_NIGHT: return "Night";
        default: return "?";
    }
}
static std::string VisibilityText() {
    if (g_weather == WEATHER_FOG) return "Low (Radar Dependent)";
    if (g_weather == WEATHER_STORM) return "Poor";
    if (g_weather == WEATHER_HEAVY_RAIN) return "Reduced";
    if (g_weather == WEATHER_RAIN) return "Moderate";
    return "Excellent";
}

static void DrawPanelBackground(double x, double y, double w, double h) {
    glColor4f(0.0f, 0.0f, 0.05f, 0.55f);
    FilledRect(x, y, w, h);
    glColor3f(0.2f, 0.7f, 0.9f);
    LineRect(x, y, w, h);
}

static void DrawDashboard() {
    if (!g_showDashboard) return;

    // ---------- TOP LEFT: Weather / Time / Visibility ----------
    {
        double x = 12, y = WORLD_H - 92, w = 250, h = 80;
        DrawPanelBackground(x, y, w, h);
        glColor3f(1,1,1);
        DrawText(x + 8, y + h - 16, "WEATHER & CONDITIONS", GLUT_BITMAP_HELVETICA_12);
        DrawText(x + 8, y + h - 34, "Weather: " + WeatherName(g_weather), GLUT_BITMAP_HELVETICA_12);
        DrawText(x + 8, y + h - 50, "Time: " + FormatSimClock() + " (" + TimeOfDayName(g_timeOfDay) + ")", GLUT_BITMAP_HELVETICA_12);
        DrawText(x + 8, y + h - 66, "Visibility: " + VisibilityText(), GLUT_BITMAP_HELVETICA_12);
    }

    // ---------- TOP RIGHT: Aircraft Count / Runway Status / Active Flights ----------
    {
        double w = 250, h = 80, x = WORLD_W - w - 12, y = WORLD_H - 92;
        DrawPanelBackground(x, y, w, h);
        glColor3f(1,1,1);
        DrawText(x + 8, y + h - 16, "AIRPORT STATUS", GLUT_BITMAP_HELVETICA_12);
        std::ostringstream c1; c1 << "Aircraft in Airspace: " << g_aircraft.size();
        DrawText(x + 8, y + h - 34, c1.str(), GLUT_BITMAP_HELVETICA_12);
        std::string rs = g_runway.occupied ? "OCCUPIED" : "CLEAR";
        glColor3f(g_runway.occupied ? 1.0f : 0.3f, g_runway.occupied ? 0.3f : 1.0f, 0.3f);
        DrawText(x + 8, y + h - 50, "Runway 14/32: " + rs, GLUT_BITMAP_HELVETICA_12);
        glColor3f(1,1,1);
        std::ostringstream c2; c2 << "Flights Handled Today: " << g_stats.flightsHandled;
        DrawText(x + 8, y + h - 66, c2.str(), GLUT_BITMAP_HELVETICA_12);
    }

    // ---------- BOTTOM: Flight Queue / Emergency Alerts / ATC Messages ----------
    {
        double w = WORLD_W - 24, h = 92, x = 12, y = 4;
        DrawPanelBackground(x, y, w, h);

        // Column 1: Queues
        glColor3f(1,1,1);
        DrawText(x + 10, y + h - 16, "RUNWAY QUEUE", GLUT_BITMAP_HELVETICA_12);
        std::ostringstream q;
        q << "Takeoff: " << g_runway.takeoffQueue.size() << "   Landing: " << g_runway.landingQueue.size();
        DrawText(x + 10, y + h - 34, q.str(), GLUT_BITMAP_HELVETICA_12);
        std::ostringstream gt;
        int freeGates = 0;
        for (size_t i = 0; i < g_gates.size(); i++) if (!g_gates[i].occupied) freeGates++;
        gt << "Free Gates: " << freeGates << " / " << g_gates.size();
        DrawText(x + 10, y + h - 50, gt.str(), GLUT_BITMAP_HELVETICA_12);
        std::ostringstream ut;
        double util = (g_simTime > 0) ? (g_stats.runwayBusySeconds / g_simTime * 100.0) : 0.0;
        ut << "Runway Utilization: " << std::fixed << std::setprecision(1) << util << "%";
        DrawText(x + 10, y + h - 66, ut.str(), GLUT_BITMAP_HELVETICA_12);

        // Column 2: Emergency Alerts
        double col2x = x + 260;
        glColor3f(1.0f, 0.4f, 0.3f);
        DrawText(col2x, y + h - 16, "EMERGENCY ALERTS", GLUT_BITMAP_HELVETICA_12);
        if (g_emergency.active) {
            glColor3f(1.0f, 0.2f, 0.2f);
            DrawText(col2x, y + h - 34, g_emergency.description, GLUT_BITMAP_HELVETICA_12);
            std::ostringstream et;
            et << "Response time remaining: " << std::fixed << std::setprecision(1) << (g_emergency.duration - g_emergency.timer) << "s";
            DrawText(col2x, y + h - 50, et.str(), GLUT_BITMAP_HELVETICA_12);
        } else {
            glColor3f(0.4f, 1.0f, 0.4f);
            DrawText(col2x, y + h - 34, "All systems normal.", GLUT_BITMAP_HELVETICA_12);
        }
        std::ostringstream em;
        em << "Emergencies Handled: " << g_stats.emergenciesHandled;
        glColor3f(1,1,1);
        DrawText(col2x, y + h - 66, em.str(), GLUT_BITMAP_HELVETICA_12);

        // Column 3: ATC Messages log
        double col3x = x + 560;
        glColor3f(0.6f, 0.9f, 1.0f);
        DrawText(col3x, y + h - 16, "ATC MESSAGES", GLUT_BITMAP_HELVETICA_12);
        double ly = y + h - 32;
        for (size_t i = 0; i < g_atcLog.size() && i < 4; i++) {
            const LogMessage& m = g_atcLog[i];
            if (m.severity == 2) glColor3f(1.0f, 0.3f, 0.3f);
            else if (m.severity == 1) glColor3f(1.0f, 0.8f, 0.3f);
            else glColor3f(0.85f, 0.85f, 0.9f);
            std::string trimmed = m.text.substr(0, 60);
            DrawText(col3x, ly, trimmed, GLUT_BITMAP_HELVETICA_12);
            ly -= 15;
        }
    }
}

// ============================================================================
//  EMERGENCY BANNER (large flashing top banner during active emergency)
// ============================================================================
static void DrawEmergencyBanner() {
    if (!g_emergency.active) return;
    float flash = (sin(g_simTime * 8.0) > 0) ? 1.0f : 0.4f;
    glColor4f(0.8f * flash, 0.05f, 0.05f, 0.85f);
    FilledRect(WORLD_W / 2.0 - 260, WORLD_H - 30, 520, 26);
    glColor3f(1,1,1);
    std::string msg = g_emergency.description;
    DrawText(WORLD_W / 2.0 - TextWidth(msg, GLUT_BITMAP_HELVETICA_18) / 2.0, WORLD_H - 22, msg);
}


// ============================================================================
//  RADAR VIEW (used by CAM_RADAR mode) — schematic top-down-style scope
// ============================================================================
static void DrawRadarView() {
    double cx = WORLD_W / 2.0, cy = WORLD_H / 2.0;
    double R = std::min(WORLD_W, WORLD_H) / 2.0 - 40;

    glColor3f(0.0f, 0.05f, 0.0f);
    FilledCircle(cx, cy, R + 10, 60);

    glColor3f(0.1f, 0.9f, 0.2f);
    for (int i = 1; i <= 4; i++) RingCircle(cx, cy, R * i / 4.0 - 1, 1.0);

    glBegin(GL_LINES);
    glVertex2d(cx - R, cy); glVertex2d(cx + R, cy);
    glVertex2d(cx, cy - R); glVertex2d(cx, cy + R);
    glEnd();

    // Sweeping radar line
    double sweepAngle = fmod(g_simTime * 90.0, 360.0);
    glColor4f(0.1f, 1.0f, 0.3f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2d(cx, cy);
    glVertex2d(cx + R * cos(DegToRad(sweepAngle)), cy + R * sin(DegToRad(sweepAngle)));
    glEnd();

    // Plot aircraft as blips mapped from world space into radar space
    for (size_t i = 0; i < g_aircraft.size(); i++) {
        const Aircraft& ac = g_aircraft[i];
        double nx = (ac.pos.x / WORLD_W - 0.5) * 2.0;
        double ny = (ac.pos.y / WORLD_H - 0.5) * 2.0;
        double bx = cx + nx * R * 0.9;
        double by = cy + ny * R * 0.9;
        glColor3f(0.2f, 1.0f, 0.3f);
        FilledCircle(bx, by, 4, 10);
        glColor3f(0.7f, 1.0f, 0.75f);
        DrawText(bx + 6, by + 4, ac.flightNumber, GLUT_BITMAP_HELVETICA_12);
        std::ostringstream alt;
        alt << "ALT " << (int)(ac.altitude * 350) << "ft  SPD " << (int)ac.speed;
        DrawText(bx + 6, by - 10, alt.str(), GLUT_BITMAP_HELVETICA_12);
    }

    glColor3f(0.2f, 1.0f, 0.3f);
    DrawText(cx - 60, cy + R + 20, "ATC RADAR — HSIA APPROACH CONTROL", GLUT_BITMAP_HELVETICA_18);
}

// ============================================================================
//  HELP / CONTROLS OVERLAY
// ============================================================================
static void DrawHelpOverlay() {
    if (!g_showHelp) return;
    double w = 480, h = 480;
    double x = WORLD_W / 2.0 - w / 2.0, y = WORLD_H / 2.0 - h / 2.0;
    glColor4f(0.02f, 0.02f, 0.05f, 0.92f);
    FilledRect(x, y, w, h);
    glColor3f(0.2f, 0.8f, 1.0f);
    LineRect(x, y, w, h);

    double ty = y + h - 30;
    glColor3f(1,1,1);
    DrawText(x + 20, ty, "SIMULATOR CONTROLS", GLUT_BITMAP_HELVETICA_18); ty -= 28;

    const char* lines[] = {
        "T - Request Takeoff (spawn departure)",
        "L - Request Landing (spawn arrival)",
        "R - Toggle Rain          F - Toggle Fog",
        "M - Heavy Rain           W - Thunderstorm",
        "N - Night Mode           D - Day Mode (resume auto cycle)",
        "E - Trigger Emergency Event",
        "G - Cycle Selected Gate",
        "S - Stop / Resume nearest taxiing aircraft",
        "P - Pause Simulation",
        "C - Change Camera View",
        "H - Toggle Aircraft Lights (all aircraft)",
        "V - Toggle Dashboard",
        "/ - Toggle this Help panel",
        "ESC - Exit"
    };
    int n = sizeof(lines) / sizeof(lines[0]);
    for (int i = 0; i < n; i++) {
        glColor3f(0.85f, 0.9f, 0.95f);
        DrawText(x + 24, ty, lines[i], GLUT_BITMAP_HELVETICA_12);
        ty -= 20;
    }
}

// ============================================================================
//  CAMERA APPLICATION
// ============================================================================
static void ApplyCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    switch (g_camMode) {
        case CAM_TOWER:
            glOrtho(0, WORLD_W, 0, WORLD_H, -1, 1);
            break;
        case CAM_RUNWAY: {
            double cx = WORLD_W / 2.0;
            double halfW = WORLD_W / 3.2;
            glOrtho(cx - halfW, cx + halfW, RUNWAY_BOT - 60, RUNWAY_TOP + 260, -1, 1);
            break;
        }
        case CAM_TERMINAL: {
            glOrtho(60, WORLD_W - 60, APRON_BOT - 20, TERMINAL_TOP + 60, -1, 1);
            break;
        }
        case CAM_RADAR:
            glOrtho(0, WORLD_W, 0, WORLD_H, -1, 1);
            break;
        case CAM_FOLLOW: {
            Aircraft* ac = FindAircraftById(g_followAircraftId);
            if (ac) {
                double halfW = 220, halfH = 140;
                glOrtho(ac->pos.x - halfW, ac->pos.x + halfW, ac->pos.y - halfH, ac->pos.y + halfH, -1, 1);
            } else {
                glOrtho(0, WORLD_W, 0, WORLD_H, -1, 1);
            }
            break;
        }
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static const char* CameraName(CameraMode m) {
    switch (m) {
        case CAM_TOWER: return "Tower View";
        case CAM_RUNWAY: return "Runway View";
        case CAM_TERMINAL: return "Terminal View";
        case CAM_RADAR: return "Radar View";
        case CAM_FOLLOW: return "Follow Aircraft View";
        default: return "?";
    }
}


// ============================================================================
//  SCENE COMPOSITION
// ============================================================================
static void DrawFullAirportScene() {
    DrawSkyGradient();
    DrawClouds();
    DrawSunMoon();

    // Air traffic zone divider (subtle)
    glColor4f(1,1,1,0.15f);
    DDA_Line(0, AIR_ZONE_TOP, WORLD_W, AIR_ZONE_TOP);

    DrawRunway();
    DrawTaxiways();
    DrawControlTower();
    DrawTerminalStructure();
    DrawApronStructure();
    DrawRoad();

    DrawRoadVehicles();
    DrawAllVehicles();
    DrawAllAircraft();

    DrawFIDSBoard();
    DrawRain();
    DrawFogOverlay();
    DrawLightning();
}

static void RenderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ApplyCamera();

    if (g_camMode == CAM_RADAR) {
        glClearColor(0,0,0,1);
        DrawRadarView();
    } else {
        DrawFullAirportScene();
    }

    // HUD is always drawn in fixed WORLD_W x WORLD_H orthographic space,
    // regardless of camera mode, so it stays legible.
    if (g_camMode != CAM_RADAR) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WORLD_W, 0, WORLD_H, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    DrawDashboard();
    DrawEmergencyBanner();

    // Camera mode label (top center)
    glColor3f(1,1,1);
    std::string camLbl = std::string("[ ") + CameraName(g_camMode) + " ]";
    DrawText(WORLD_W / 2.0 - TextWidth(camLbl) / 2.0, WORLD_H - 20, camLbl);

    if (g_paused) {
        glColor3f(1.0f, 0.9f, 0.2f);
        std::string p = "PAUSED";
        DrawText(WORLD_W / 2.0 - TextWidth(p, GLUT_BITMAP_TIMES_ROMAN_24) / 2.0, WORLD_H / 2.0, p, GLUT_BITMAP_TIMES_ROMAN_24);
    }

    DrawHelpOverlay();

    glutSwapBuffers();
}

// ============================================================================
//  MAIN SIMULATION UPDATE STEP
// ============================================================================
static double g_lastTimeMs = 0.0;

static void SimulationStep(double dt) {
    if (g_paused) return;

    g_simTime += dt;
    UpdateDayNightCycle(dt);
    UpdateClouds(dt);
    UpdateWeather(dt);
    UpdateRoadVehicles(dt);
    UpdateAllAircraft(dt);
    UpdateRunwayLogic(dt);
    UpdateVehicles(dt);
    UpdateEmergency(dt);
    UpdateScheduler(dt);

    for (size_t i = 0; i < g_aircraft.size(); i++) {
        if (!g_aircraft[i].lightsOn) continue;
    }
}

static void TimerCallback(int value) {
    double nowMs = (double)glutGet(GLUT_ELAPSED_TIME);
    double dt = (g_lastTimeMs <= 0) ? 0.016 : (nowMs - g_lastTimeMs) / 1000.0;
    dt = Clamp(dt, 0.0, 0.05); // clamp to avoid huge jumps
    g_lastTimeMs = nowMs;

    SimulationStep(dt);
    glutPostRedisplay();
    glutTimerFunc(16, TimerCallback, 0);
}


// ============================================================================
//  INPUT HANDLING
// ============================================================================
static void ToggleWeather(WeatherType w) {
    g_weather = (g_weather == w) ? WEATHER_CLEAR : w;
    PushLog(std::string("Weather changed to: ") + WeatherName(g_weather).c_str());
    if (g_weather != WEATHER_CLEAR && g_rainDrops.empty()) InitRain();
}

static void StopNearestTaxiingAircraft() {
    for (size_t i = 0; i < g_aircraft.size(); i++) {
        AircraftPhase ph = g_aircraft[i].phase;
        if (ph == PHASE_TAXI_OUT || ph == PHASE_TAXI_IN) {
            g_aircraft[i].speed = (g_aircraft[i].speed > 0) ? 0 : 45;
            PushLog("ATC: Flight " + g_aircraft[i].flightNumber + " hold position issued.");
            return;
        }
    }
}

static void KeyboardDown(unsigned char key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case 'T': case 't': SpawnDeparture(); break;
        case 'L': case 'l': SpawnArrival(); break;
        case 'R': case 'r': ToggleWeather(WEATHER_RAIN); break;
        case 'M': case 'm': ToggleWeather(WEATHER_HEAVY_RAIN); break;
        case 'F': case 'f': ToggleWeather(WEATHER_FOG); break;
        case 'W': case 'w': ToggleWeather(WEATHER_STORM); break;
        case 'N': case 'n':
            g_autoDayNight = false;
            g_timeOfDay = TOD_NIGHT;
            g_dayClock = 0.05;
            PushLog("Manual override: Night mode engaged.");
            break;
        case 'D': case 'd':
            g_autoDayNight = true;
            PushLog("Day/Night auto-cycle resumed.");
            break;
        case 'E': case 'e': TriggerRandomEmergency(); break;
        case 'G': case 'g':
            g_selectedGate = (g_selectedGate + 1) % (int)g_gates.size();
            PushLog("Selected " + g_gates[g_selectedGate].label + " for manual assignment.");
            break;
        case 'S': case 's': StopNearestTaxiingAircraft(); break;
        case 'P': case 'p':
            g_paused = !g_paused;
            break;
        case 'C': case 'c': {
            int m = (int)g_camMode;
            m = (m + 1) % 5;
            g_camMode = (CameraMode)m;
            if (g_camMode == CAM_FOLLOW && !g_aircraft.empty()) {
                g_followAircraftId = g_aircraft[0].id;
            }
            break;
        }
        case 'H': case 'h':
            for (size_t i = 0; i < g_aircraft.size(); i++) g_aircraft[i].lightsOn = !g_aircraft[i].lightsOn;
            break;
        case 'V': case 'v':
            g_showDashboard = !g_showDashboard;
            break;
        case '/':
            g_showHelp = !g_showHelp;
            break;
        case 27: // ESC
            exit(0);
            break;
        default:
            break;
    }
}

static void SpecialKeyDown(int key, int x, int y) {
    (void)x; (void)y;
    // Cycle follow-camera target with left/right arrows when in follow mode
    if (g_camMode == CAM_FOLLOW && !g_aircraft.empty()) {
        int idx = 0;
        for (size_t i = 0; i < g_aircraft.size(); i++) {
            if (g_aircraft[i].id == g_followAircraftId) { idx = (int)i; break; }
        }
        if (key == GLUT_KEY_RIGHT) idx = (idx + 1) % (int)g_aircraft.size();
        if (key == GLUT_KEY_LEFT)  idx = (idx - 1 + (int)g_aircraft.size()) % (int)g_aircraft.size();
        g_followAircraftId = g_aircraft[idx].id;
    }
}

static void ReshapeWindow(int w, int h) {
    WIN_W = w; WIN_H = h;
    glViewport(0, 0, w, h);
}

// ============================================================================
//  INITIALIZATION
// ============================================================================
static void InitGL() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glClearColor(0.4f, 0.7f, 0.95f, 1.0f);
}

static void InitSimulation() {
    srand((unsigned int)time(NULL));
    InitAircraftTypes();
    InitGates();
    InitClouds();
    InitRain();
    InitRoadVehicles();

    PushLog("HSIA Smart Operations Simulator initialized.");
    PushLog("ATC systems online. Runway 14/32 clear.");

    // Seed a couple of initial flights so the airport isn't empty at start
    SpawnDeparture();
    SpawnArrival();
}

static void PrintControlsToConsole() {
    printf("============================================================\n");
    printf(" HAZRAT SHAHJALAL INTERNATIONAL AIRPORT - SMART OPS SIMULATOR\n");
    printf("============================================================\n");
    printf(" T Takeoff | L Landing | R Rain | M Heavy Rain | F Fog | W Storm\n");
    printf(" N Night   | D Day     | E Emergency | G Gate | S Stop taxi\n");
    printf(" P Pause   | C Camera  | H Lights | V Dashboard | / Help | ESC Exit\n");
    printf("============================================================\n");
}

// ============================================================================
//  MAIN ENTRY POINT
// ============================================================================
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(60, 30);
    glutCreateWindow("Hazrat Shahjalal International Airport - Smart Operations Simulator");

    GLenum glewStatus = glewInit();
    (void)glewStatus;

    InitGL();
    InitSimulation();
    PrintControlsToConsole();

    glutDisplayFunc(RenderScene);
    glutReshapeFunc(ReshapeWindow);
    glutKeyboardFunc(KeyboardDown);
    glutSpecialFunc(SpecialKeyDown);
    glutTimerFunc(16, TimerCallback, 0);

    glutMainLoop();
    return 0;
}
