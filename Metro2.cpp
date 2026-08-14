#if defined(__APPLE__)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#if defined(__has_include)
    #if __has_include(<GLFW/glfw3.h>)
        #include <GLFW/glfw3.h>
    #endif
#endif

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

 // Global Declaration

const int WINDOW_WIDTH = 1300;
const int WINDOW_HEIGHT = 750;
const char* WINDOW_TITLE = "Smart Metro Rail Station Simulation | CG Lab Project";

const double PI = 3.14159265358979323846;

//  Vertical layout of the scene
const double SKY_TOP = (double)WINDOW_HEIGHT;
const double SKY_BOTTOM = 380.0;
const double BACKDROP_BOTTOM = 260.0;   // For the buildings
const double ROAD_Y = 230.0;
const double ROAD_HEIGHT = 30.0;
const double BRIDGE_DECK_Y = 560.0;
const double TRACK_Y = 150.0;
const double TRACK_THICKNESS  = 6.0;
const double PLATFORM_TOP = 200.0;
const double PLATFORM_HEIGHT = 42.0;
const double PLATFORM_LEFT = 40.0;
const double PLATFORM_RIGHT = 1260.0;
const double SAFETY_LINE_Y = 205.0;
const double GROUND_Y = 0.0;

//  Station infrastructure from left to right
const double TICKET_X = 90.0,  TICKET_W = 100.0;
const double WAITING_X = 240.0, WAITING_W = 230.0;
const double ELEVATOR_X = 560.0, ELEVATOR_W = 42.0;
const double ESCALATOR_X = 650.0f, ESCALATOR_STEPW = 16.0;
const int ESCALATOR_STEPS = 10;
const double STAIRCASE_X = 850.0, STAIRCASE_STEPW = 22.0;
const int STAIRCASE_STEPS = 8;
const double BRIDGE_X0 = 1080.0, BRIDGE_X1 = 1260.0;

// Clock and Floating Window
const double CLOCK_CX = 130.0, CLOCK_CY = 650.0, CLOCK_R = 28.0;
const double WATCH_PANEL_X = 190.0, WATCH_PANEL_Y = 612.0;
const double WATCH_PANEL_W = 250.0, WATCH_PANEL_H = 66.0;

// Train Design Variables
const double TRAIN_BASE_Y = TRACK_Y + TRACK_THICKNESS;
const double TRAIN_HEIGHT = 92.0;
const double COACH_WIDTH = 150.0;
const double CABIN_WIDTH = 116.0;
const double COACH_GAP  = 10.0;
const int NUM_COACHES = 4;
const double WHEEL_RADIUS = 14.0;

// Position where the platform the train should stop
const double PLATFORM_STOP_X = 300.0;

// Passenger Design
const int NUM_WALKING_PASSENGERS = 6;
const int NUM_WAITING_PASSENGERS = 3;
const int NUM_QUEUE_PASSENGERS = 3;

// Weather Condition
const int NUM_RAINDROPS_MAX = 200;

const int NUM_CLOUDS = 5;

// Camera views
const int NUM_CAMERA_VIEWS = 4;

// Timing
const int TICKS_PER_SECOND = 10;
const int EVENING_PREVIEW_TICKS = 100;
const double LIGHT_TRANSITION_STEP = 0.02;


// Enums for faster access

// For Train
enum class TrainState {
    OFFSCREEN_LEFT,     // train is not present
    APPROACHING,        // train coming
    BRAKING,            // train stopping
    STOPPED,            // train stopped
    DOORS_OPENING,      // doors opening
    DOORS_OPEN,         // doors open for Enter & Exit
    DOORS_CLOSING,      // doors close
    DEPARTING,          // train leaving
    EMERGENCY_STOPPED   // everything stopped for emergency
};

// For Signal
enum class SignalState
{
    RED,YELLOW,GREEN
};

// For PAssengers
enum class PassengerActivity
{
    WALKING,WAITING,QUEUING,EXITING
};

// For Scenarios
enum class LightingMode
{
    DAY,EVENING,NIGHT
};

// Rain condition
enum class RainIntensity
{
    NONE,LIGHT,HEAVY
};

// For Elevator
enum class ElevatorState
{
    MOVING,DOORS_OPENING,DOORS_OPEN,DOORS_CLOSING
};


// OOP approach for creating classes

struct Point2D {
    double x, y;
};

struct Passenger {
    double x, y;
    double baseY;
    double speed;
    double phase;
    int   direction;
    PassengerActivity activity;
    double r, g, b;
    bool  active;
};

struct RainDrop {
    double x, y;
    double length;
    double speed;
};

struct Cloud {
    double x, y;
    double scale;
    double speed;
};

struct Bird {
    double x, y;
    double speed;
    double wingPhase;
};


// For sky & lighting
struct ModePalette {
    double skyTopR,  skyTopG,  skyTopB;
    double skyBotR,  skyBotG,  skyBotB;
    double buildingShade;
    double windowLitChance;
    double lampOn;
    double sunAlpha;
    double moonAlpha;
    double sunR, sunG, sunB;
};

// For Day/Night/Evening

const ModePalette PALETTE_DAY = {
    /*sky top*/    0.35, 0.65, 0.95,
    /*sky bottom*/ 0.75, 0.88, 0.98,
    /*bldgShade*/  0.55, /*winLit*/ 0.06, /*lampOn*/ 0.0,
    /*sunAlpha*/   1.0,  /*moonAlpha*/ 0.0,
    /*sun colour*/ 1.0, 0.87, 0.25
};
const ModePalette PALETTE_EVENING = {
    /*sky top*/    0.20, 0.20, 0.42,
    /*sky bottom*/ 0.95, 0.55, 0.35,
    /*bldgShade*/  0.30, /*winLit*/ 0.45, /*lampOn*/ 0.55,
    /*sunAlpha*/   0.85, /*moonAlpha*/ 0.25,
    /*sun colour*/ 1.0, 0.55, 0.20
};
const ModePalette PALETTE_NIGHT = {
    /*sky top*/    0.02, 0.02, 0.12,
    /*sky bottom*/ 0.08, 0.10, 0.25,
    /*bldgShade*/  0.12, /*winLit*/ 0.85, /*lampOn*/ 1.0,
    /*sunAlpha*/   0.0,  /*moonAlpha*/ 1.0,
    /*sun colour*/ 1.0, 1.0, 1.0
};

LightingMode activeLightMode = LightingMode::DAY;
LightingMode nextLightMode = LightingMode::DAY;
double lightTransition  = 1.0;
ModePalette  currentPalette = PALETTE_DAY;
int eveningPreviewTicks = 0;

// Environment
double sunMoonAngle = 0.0;
vector<Cloud> clouds;
vector<Bird>  birds;
double windStrength = 1.0;
bool isFoggy = false;

// Raining
RainIntensity rainIntensity = RainIntensity::NONE;
vector<RainDrop> rainDrops;

// Pause/Resume
bool isPaused = false;
bool autoMode = true;

// Camera
int cameraView = 0;

// Train Property
TrainState trainState = TrainState::OFFSCREEN_LEFT;
double trainX = -400.0;
double trainSpeed = 3.0;
const double MIN_SPEED = 1.0;
const double MAX_SPEED = 8.0;
double doorOpenAmount = 0.0;
double wheelRotationAngle = 0.0;
bool emergencyStopFlag = false;
int boardingTicksLeft = 0;

// Signal
SignalState signalState = SignalState::GREEN;

// Station operations
int clockHour = 8;
int clockMinute = 30;
int clockSecond = 0;
int tickCounter = 0;
double escalatorOffset = 0.0;
double elevatorY = PLATFORM_TOP + PLATFORM_HEIGHT;
bool elevatorGoingUp = true;
ElevatorState elevatorState = ElevatorState::MOVING;
double elevatorDoorOpenAmount = 0.0;
int elevatorDwellTicks = 0;
const int ELEVATOR_DWELL_TICKS = 40;
int passengerCount = 0;
int exitedPassengerCount = 0;
string destinationText = "UTTARA CENTRAL STATION";

// Station Details
vector<string> stationTimetable = {
    "UTTARA CENTRAL STATION",
    "UTTARA CENTER",
    "MIRPUR 10",
    "AGARGAON",
    "FARMGATE",
    "KARWAN BAZAR",
    "SHAHBAG",
    "DHAKA UNIVERSITY",
    "MOTIJHEEL"
};
int currentStationIndex = 0;

// Gate
double gateAnimTime = 0.0;

// Star Scenario
bool shootingStarActive = false;
double starX = 0.0, starY = 0.0, starVX = 0.0, starVY = 0.0;
int starCooldown = 250;

// Passengers
vector<Passenger> walkingPassengers;
vector<Passenger> waitingPassengers;
vector<Passenger> queuePassengers;
vector<Passenger> exitingPassengers;

// Announcement banner
string announcementText = "WELCOME TO UTTARA CENTRAL METRO STATION";
int announcementTimer = 0;

//Function declaration

// Algorithms
void setPixel(int x, int y);
void ddaLine(double x0, double y0, double x1, double y1);
void bresenhamLine(int x0, int y0, int x1, int y1);
void midpointCircle(double xc, double yc, double radius, bool filled);
void bezierCurve(Point2D p0, Point2D p1, Point2D p2, Point2D p3, int segments);
void drawRectangle(double x, double y, double w, double h, bool filled);
void drawText(double x, double y, const string &text, void *font, double r, double g, double b);

// 2D transformation
Point2D translatePoint(const Point2D &p, double dx, double dy);
Point2D scalePoint(const Point2D &p, const Point2D &pivot, double sx, double sy);
Point2D rotatePoint(const Point2D &p, const Point2D &pivot, double angleDegrees);

// Lighting for Day/Evening/Night
ModePalette paletteForMode(LightingMode mode);
ModePalette lerpPalette(const ModePalette &a, const ModePalette &b, double t);
void updateLighting();

// Environment
int starHeightFor(int i);
void drawSky();
void drawSunMoon();
void initClouds();
void drawClouds();
void updateClouds();
void drawTrees();
void drawTreeAt(double x, double y, double size);
void drawBackgroundBuildings();
void drawRoad();
void drawFogOverlay();
void updateShootingStar();
void drawShootingStar();
void initBirds();
void drawBirds();
void updateBirds();
void advanceToNextStation();

// Metro Station
void drawElevatedBridge();
void drawRailwayTracks();
void drawPlatform();
void drawSafetyLine();
void drawPlatformScreenDoors();
bool platformCrossingPermitted();
void drawWaitingArea();
void drawBenches();
void drawTicketCounter();
void drawEscalator();
void drawElevatorShaft();
void drawStaircase();
void drawDigitalWatch();
void drawAnalogClock();
void drawPlatformSignboards();
void drawSignalSystem();
void drawSignalReflectionStrip();
void drawStationLighting();
void updateGates();
void drawTicketGates();
void drawWetReflections();

// Train
void drawTrain();
void drawCoach(double x, double y, double w, double h, bool isCabin, bool front, int coachIndex);
void drawBogie(double x, double w);
void drawConnector(double x, double y, double w, double h);
void drawHeadTailLights();

// Passengers
void initPassengers();
void drawPassengerFigure(const Passenger &p);
void drawWalkingPassengers();
void drawWaitingPassengers();
void drawQueue();
void drawStationStaff();
void updatePassengers();
void spawnExitingPassengers();
void drawExitingPassengers();
void updateExitingPassengers();

// Weather
void initRain();
void drawRain();
void updateRain();
int activeRainDropCount();

// Screen
void drawHUD();
void drawControlsPanel();
void drawSpeedGauge();
string trainStateLabel();
string signalStateLabel();
string cameraViewLabel();
string lightingModeLabel();

// Animation
void updateTrain();
void updateSignalFromTrainState();
void requestDoorsOpen();
void requestDoorsClose();
void triggerBoarding();
void triggerEmergencyStop();
void cycleSignalManual();
void requestTrainArrival();

void logEvent(const string &msg);

// Camera
void applySceneProjection();
void applyDefaultProjection();

// GLUT functions
void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void timerFunc(int value);
void initGL();
void verifyLayoutNonOverlapping();
void printStartupBanner();

// point drawer
void setPixel(int x, int y) {
    glBegin(GL_POINTS);
        glVertex2i(x, y);
    glEnd();
}

// DDA
// railway tracks, platform boundaries, bridge trusses, background
// buildings, road markings and signal poles.
void ddaLine(double x0, double y0, double x1, double y1) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    int steps = (int) (std::max(std::fabs(dx), std::fabs(dy)));
    if (steps == 0)
    {
        setPixel((int)x0, (int)y0); return;
    }

    double xInc = dx / (double) steps;
    double yInc = dy / (double) steps;

    double x = x0;
    double y = y0;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; ++i) {
        glVertex2i((int)round(x), (int)round(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// Bresenham
// window frames, door frames, platform edges,

void bresenhamLine(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err= dx - dy;

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x0, y0);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 <  dx)
        {
            err += dx;
            y0 += sy;
        }
    }
    glEnd();
}

// MidPoint Circle
void midpointCircle(double xcF, double ycF, double radius, bool filled) {
    int xc = (int) xcF;
    int yc = (int) ycF;
    int x = 0;
    int y = (int) radius;
    int d = 1 - (int) radius;

    auto plotOctants = [&](int px, int py) {
        if (!filled) {
            glBegin(GL_POINTS);
            glVertex2i(xc + px, yc + py);
            glVertex2i(xc - px, yc + py);
            glVertex2i(xc + px, yc - py);
            glVertex2i(xc - px, yc - py);
            glVertex2i(xc + py, yc + px);
            glVertex2i(xc - py, yc + px);
            glVertex2i(xc + py, yc - px);
            glVertex2i(xc - py, yc - px);
            glEnd();
        } else {
            glBegin(GL_LINES);
            glVertex2i(xc - px, yc + py);
            glVertex2i(xc + px, yc + py);
            glVertex2i(xc - px, yc - py);
            glVertex2i(xc + px, yc - py);
            glVertex2i(xc - py, yc + px);
            glVertex2i(xc + py, yc + px);
            glVertex2i(xc - py, yc - px);
            glVertex2i(xc + py, yc - px);
            glEnd();
        }
    };

    plotOctants(x, y);
    while (x < y) {
        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
        plotOctants(x, y);
    }
}

// Bezier Curve

void bezierCurve(Point2D p0, Point2D p1, Point2D p2, Point2D p3, int segments) {
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        double t = (double) i / (double) segments;
        double u = 1.0 - t;
        double bx = u*u*u*p0.x + 3*u*u*t*p1.x + 3*u*t*t*p2.x + t*t*t*p3.x;
        double by = u*u*u*p0.y + 3*u*u*t*p1.y + 3*u*t*t*p2.y + t*t*t*p3.y;
        glVertex2f(bx, by);
    }
    glEnd();
}

// Rectangle Draw
void drawRectangle(double x, double y, double w, double h, bool filled) {
    glBegin(filled ? GL_QUADS : GL_LINE_LOOP);
        glVertex2f(x,y);
        glVertex2f(x + w,y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();
}

// Text for showing
void drawText(double x, double y, const string &text, void *font, double r, double g, double b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

// 2D Transformations

// Translation
Point2D translatePoint(const Point2D &p, double dx, double dy) {
    return { p.x + dx, p.y + dy };
}

// Scaling
Point2D scalePoint(const Point2D &p, const Point2D &pivot, double sx, double sy) {
    Point2D rel = { p.x - pivot.x, p.y - pivot.y };
    rel.x *= sx;
    rel.y *= sy;
    return { rel.x + pivot.x, rel.y + pivot.y };
}

// Rotation
Point2D rotatePoint(const Point2D &p, const Point2D &pivot, double angleDegrees) {
    double rad = (double) (angleDegrees * PI / 180.0);
    double cosA = cos(rad);
    double sinA = sin(rad);
    Point2D rel = { p.x - pivot.x, p.y - pivot.y };
    Point2D rotated;
    rotated.x = rel.x * cosA - rel.y * sinA;
    rotated.y = rel.x * sinA + rel.y * cosA;
    return { rotated.x + pivot.x, rotated.y + pivot.y };
}

// Light for Day/Night/Evening
ModePalette paletteForMode(LightingMode mode) {
    switch (mode) {
        case LightingMode::DAY:
            return PALETTE_DAY;
        case LightingMode::EVENING:
            return PALETTE_EVENING;
        case LightingMode::NIGHT:
            return PALETTE_NIGHT;
    }
    return PALETTE_DAY;
}

// Scene chnage
ModePalette lerpPalette(const ModePalette &a, const ModePalette &b, double t) {
    t = max(0.0, min(1.0, t));
    ModePalette r;
    r.skyTopR = a.skyTopR + (b.skyTopR - a.skyTopR) * t;
    r.skyTopG = a.skyTopG + (b.skyTopG - a.skyTopG) * t;
    r.skyTopB = a.skyTopB + (b.skyTopB - a.skyTopB) * t;
    r.skyBotR = a.skyBotR + (b.skyBotR - a.skyBotR) * t;
    r.skyBotG = a.skyBotG + (b.skyBotG - a.skyBotG) * t;
    r.skyBotB = a.skyBotB + (b.skyBotB - a.skyBotB) * t;
    r.buildingShade = a.buildingShade + (b.buildingShade - a.buildingShade) * t;
    r.windowLitChance = a.windowLitChance + (b.windowLitChance - a.windowLitChance) * t;
    r.lampOn = a.lampOn + (b.lampOn - a.lampOn) * t;
    r.sunAlpha = a.sunAlpha + (b.sunAlpha - a.sunAlpha) * t;
    r.moonAlpha = a.moonAlpha + (b.moonAlpha - a.moonAlpha) * t;
    r.sunR = a.sunR + (b.sunR - a.sunR) * t;
    r.sunG = a.sunG + (b.sunG - a.sunG) * t;
    r.sunB = a.sunB + (b.sunB - a.sunB) * t;
    return r;
}

// Animation change for lighting
void updateLighting() {
    if (eveningPreviewTicks > 0) {
        eveningPreviewTicks--;
        currentPalette = PALETTE_EVENING;
        return;
    }

    if (lightTransition < 1.0) {
        lightTransition = min(1.0, lightTransition + LIGHT_TRANSITION_STEP);
        currentPalette = lerpPalette(paletteForMode(activeLightMode), paletteForMode(nextLightMode), lightTransition);
        if (lightTransition >= 1.0) {
            activeLightMode = nextLightMode;
        }
    } else {
        currentPalette = paletteForMode(activeLightMode);
    }
}

// Scene
int starHeightFor(int i) {
    return (int) SKY_BOTTOM + 40 + ((i * 71) % (int)(SKY_TOP - SKY_BOTTOM - 60));
}


// Sky
void drawSky() {
    glBegin(GL_QUADS);
        glColor3f(currentPalette.skyTopR, currentPalette.skyTopG, currentPalette.skyTopB); glVertex2f(0, SKY_TOP);
        glColor3f(currentPalette.skyTopR, currentPalette.skyTopG, currentPalette.skyTopB); glVertex2f(WINDOW_WIDTH, SKY_TOP);
        glColor3f(currentPalette.skyBotR, currentPalette.skyBotG, currentPalette.skyBotB); glVertex2f(WINDOW_WIDTH, SKY_BOTTOM);
        glColor3f(currentPalette.skyBotR, currentPalette.skyBotG, currentPalette.skyBotB); glVertex2f(0, SKY_BOTTOM);
    glEnd();

    // Stars and moon
    if (currentPalette.moonAlpha > 0.05) {
        glColor4f(1.0, 1.0, 1.0, currentPalette.moonAlpha);
        static const int starCount = 60;
        for (int i = 0; i < starCount; ++i) {
            int sx = (i * 137) % WINDOW_WIDTH;
            int sy = starHeightFor(i);
            setPixel(sx, sy);
        }
    }
}

// Sun + Moon

void drawSunMoon() {
    double cx = 1120.0 + 15.0 * std::sin(sunMoonAngle);
    double cy = 640.0 + 8.0  * std::cos(sunMoonAngle);

    if (currentPalette.sunAlpha > 0.02) {
        for (int i = 4; i >= 1; --i) {
            double alpha = 0.05 * i * currentPalette.sunAlpha;
            glColor4f(currentPalette.sunR, currentPalette.sunG, currentPalette.sunB, alpha);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx, cy);
            for (int a = 0; a <= 360; a += 10) {
                double rad = a * PI / 180.0;
                glVertex2f(cx + cos(rad) * (40 + i * 8), cy + sin(rad) * (40 + i * 8));
            }
            glEnd();
        }
        glColor4f(currentPalette.sunR, currentPalette.sunG, currentPalette.sunB, currentPalette.sunAlpha);
        midpointCircle(cx, cy, 40.0, true);
    }

    if (currentPalette.moonAlpha > 0.02) {
        glColor4f(0.9, 0.9, 0.85, currentPalette.moonAlpha);
        midpointCircle(cx, cy, 34.0, true);
        glColor4f(0.75, 0.75, 0.72, currentPalette.moonAlpha);
        midpointCircle(cx - 10,cy + 8,6.0,true);
        midpointCircle(cx + 8,cy - 6,4.0,true);
        midpointCircle(cx + 2,cy + 14,3.0,true);
    }
}

// Clouds load

void initClouds() {
    clouds.clear();
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        Cloud c;
        c.x = (double) (rand() % WINDOW_WIDTH);
        c.y = 550.0 + (double) (rand() % 120);
        c.scale = 0.7 + 0.1 * (rand() % 5);
        c.speed = 0.15 + 0.05 * (rand() % 4);
        clouds.push_back(c);
    }
}

// Clouds Draw

void drawSingleCloud(const Cloud &c) {
    double shade = 1.0 - 0.45 * currentPalette.lampOn;
    glColor3f(shade, shade, shade);
    midpointCircle(c.x,c.y,18.0 * c.scale,true);
    midpointCircle(c.x + 20 * c.scale,c.y + 8 * c.scale,22.0 * c.scale,true);
    midpointCircle(c.x + 45 * c.scale,c.y,16.0 * c.scale,true);
    midpointCircle(c.x - 18 * c.scale,c.y + 4 * c.scale,14.0 * c.scale,true);
}

void drawClouds() {
    for (const auto &c : clouds) drawSingleCloud(c);
}

// Clouds animation

void updateClouds() {
    for (auto &c : clouds) {
        Point2D moved = translatePoint({c.x, c.y}, c.speed * windStrength, 0.0);
        c.x = moved.x;
        if (c.x > WINDOW_WIDTH + 60) c.x = -60.0;
    }
}

// Tree ---> MidPoint & Bresenham
void drawTreeAt(double x, double y, double size) {
    glColor3f(0.36, 0.22, 0.10);
    bresenhamLine((int)(x - 3 * size), (int)y, (int)(x - 2 * size), (int)(y + 22 * size));
    bresenhamLine((int)(x + 3 * size), (int)y, (int)(x + 2 * size), (int)(y + 22 * size));
    drawRectangle(x - 3 * size, y, 6 * size, 22 * size, true);

    double dim = 1.0 - 0.5 * currentPalette.lampOn;
    glColor3f(0.18 * dim, 0.42 * dim, 0.18 * dim);
    midpointCircle(x, y + 34 * size, 16 * size, true);
    midpointCircle(x - 12 * size,y + 26 * size, 12 * size, true);
    midpointCircle(x + 12 * size,y + 26 * size, 12 * size, true);
    midpointCircle(x, y + 22 * size, 10 * size, true);
}

void drawTrees() {
    drawTreeAt(90,  BACKDROP_BOTTOM, 1.0);
    drawTreeAt(150, BACKDROP_BOTTOM, 0.8);
    drawTreeAt(1160, BACKDROP_BOTTOM, 1.0);
    drawTreeAt(1210, BACKDROP_BOTTOM, 0.85);
    drawTreeAt(1100, BACKDROP_BOTTOM, 0.7);
}

// Buildings
void drawBackgroundBuildings() {
    struct Bldg { double x, w, h; };
    static const Bldg buildings[] = {
        { 40,  70, 150}, {130, 55, 190}, {210, 90, 120},
        {950, 80, 170}, {1040, 60, 140}, {1105, 95, 200}
    };

    for (const auto &b : buildings) {
        double shade = currentPalette.buildingShade;
        glColor3f(shade, shade, shade + 0.05);
        drawRectangle(b.x, BACKDROP_BOTTOM, b.w, b.h, true);

        glColor3f(shade * 0.5, shade * 0.5, shade * 0.55);
        for (double wy = BACKDROP_BOTTOM + 10; wy < BACKDROP_BOTTOM + b.h - 10; wy += 18) {
            ddaLine(b.x + 6, wy, b.x + b.w - 6, wy);
        }
        for (double wx = b.x + 6; wx < b.x + b.w - 6; wx += 14) {
            ddaLine(wx, BACKDROP_BOTTOM + 6, wx, BACKDROP_BOTTOM + b.h - 6);
        }

        // Window light
        glColor3f(0.95, 0.85, 0.35);
        int cols = max(1, (int)((b.w - 12) / 14));
        int rows = max(1, (int)((b.h - 20) / 18));
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                double pseudo = fmod((r * 37 + c * 53 + b.x) * 0.61803, 1.0);
                if (pseudo < currentPalette.windowLitChance) {
                    double wx = b.x + 8 + c * 14;
                    double wy = BACKDROP_BOTTOM + 12 + r * 18;
                    drawRectangle(wx, wy, 6, 8, true);
                }
            }
        }
    }
}

// Road

void drawRoad() {
    double shade = currentPalette.buildingShade * 0.4;
    glColor3f(shade, shade, shade + 0.02);
    drawRectangle(0, ROAD_Y, WINDOW_WIDTH, ROAD_HEIGHT, true);

    glColor3f(0.9, 0.9, 0.2);
    for (double x = 10; x < WINDOW_WIDTH; x += 45) {
        ddaLine(x, ROAD_Y + ROAD_HEIGHT / 2.0, x + 22, ROAD_Y + ROAD_HEIGHT / 2.0);
    }
}

// Weather ---> Fog

void drawFogOverlay() {
    if (!isFoggy) return;
    glColor4f(0.75, 0.78, 0.8, 0.30);
    drawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, true);
    glColor4f(0.8, 0.82, 0.85, 0.22);
    drawRectangle(0, BACKDROP_BOTTOM, WINDOW_WIDTH, SKY_BOTTOM - BACKDROP_BOTTOM + 40, true);
}

// Star Shooting

void updateShootingStar() {
    if (!shootingStarActive) {
        if (starCooldown > 0)
        {
            starCooldown--; return;
        }
        // works on night when sky is clear
        bool clearNight = (currentPalette.moonAlpha > 0.6) && !isFoggy && (activeRainDropCount() == 0);
        if (clearNight && (rand() % 100) < 3) {
            shootingStarActive = true;
            starX = 700.0 + (double) (rand() % 500);
            starY = (double) WINDOW_HEIGHT - 40.0 - (float) (rand() % 120);
            starVX = -7.0 - (double) (rand() % 3);
            starVY = -3.5 - (double) (rand() % 2);
        } else {
            starCooldown = 20;
        }
        return;
    }

    Point2D moved = translatePoint({starX, starY}, starVX, starVY);
    starX = moved.x;
    starY = moved.y;

    if (starX < 0 || starY < SKY_BOTTOM) {
        shootingStarActive = false;
        starCooldown = 300 + (rand() % 400);
    }
}

void drawShootingStar() {
    if (!shootingStarActive)
        return;
    glColor4f(1.0, 1.0, 1.0, 0.9);
    ddaLine(starX, starY, starX - starVX * 2.2, starY - starVY * 2.2);
    glColor3f(1.0, 1.0, 1.0);
    midpointCircle(starX, starY, 1.6, true);
}

// Birds

void initBirds() {
    birds.clear();
    for (int i = 0; i < 6; ++i) {
        Bird b;
        b.x = (double) (rand() % WINDOW_WIDTH);
        b.y = 560.0 + (double) (rand() % 100);
        b.speed = 1.2 + 0.3 * (i % 3);
        b.wingPhase = (double) i;
        birds.push_back(b);
    }
}

void drawBirds() {
    if (currentPalette.lampOn > 0.7)
        return;
    glColor3f(0.15, 0.15, 0.15);
    for (const auto &b : birds) {
        double flap = 3.0 + 2.0 * sin(b.wingPhase);
        bresenhamLine((int)(b.x - 8), (int)(b.y - flap), (int)b.x, (int)b.y);
        bresenhamLine((int)(b.x + 8), (int)(b.y - flap), (int)b.x, (int)b.y);
    }
}

// Birds Flying
void updateBirds() {
    for (auto &b : birds) {
        Point2D moved = translatePoint({b.x, b.y}, b.speed * windStrength, 0.0f);
        b.x = moved.x;
        b.wingPhase += 0.3;
        if (b.x > WINDOW_WIDTH + 20) b.x = -20.0;
    }
}

// Train leave
void advanceToNextStation() {
    currentStationIndex = (currentStationIndex + 1) % (int) stationTimetable.size();
    destinationText = stationTimetable[currentStationIndex];
}

// Platform

// Bridge
void drawElevatedBridge() {
    double bx0 = BRIDGE_X0, bx1 = BRIDGE_X1;
    double shade = 0.45 - 0.15 * currentPalette.lampOn;
    glColor3f(shade, shade, shade + 0.05);
    drawRectangle(bx0, BRIDGE_DECK_Y, bx1 - bx0, 16.0, true);

    drawRectangle(bx0 + 10,  PLATFORM_TOP + PLATFORM_HEIGHT, 14, BRIDGE_DECK_Y - (PLATFORM_TOP + PLATFORM_HEIGHT), true);
    drawRectangle(bx1 - 24,  PLATFORM_TOP + PLATFORM_HEIGHT, 14, BRIDGE_DECK_Y - (PLATFORM_TOP + PLATFORM_HEIGHT), true);

    glColor3f(0.25, 0.25, 0.3);
    for (double x = bx0; x < bx1 - 20; x += 40) {
        ddaLine(x, BRIDGE_DECK_Y - 20,x + 40, BRIDGE_DECK_Y);
        ddaLine(x, BRIDGE_DECK_Y,x + 40, BRIDGE_DECK_Y - 20);
    }
    ddaLine(bx0, BRIDGE_DECK_Y + 16 + 24, bx1, BRIDGE_DECK_Y + 16 + 24);

    Point2D p0 = {bx0 - 40, PLATFORM_TOP + PLATFORM_HEIGHT};
    Point2D p1 = {bx0 - 40, BRIDGE_DECK_Y + 50};
    Point2D p2 = {bx0 + 10, BRIDGE_DECK_Y + 50};
    Point2D p3 = {bx0 + 10, PLATFORM_TOP + PLATFORM_HEIGHT};
    glColor3f(0.55, 0.55, 0.6);
    bezierCurve(p0, p1, p2, p3,40);

    // Bridge lamps
    if (currentPalette.lampOn > 0.05) {
        glColor4f(1.0, 0.95, 0.6, 0.5 * currentPalette.lampOn);
        midpointCircle(bx0 + 20, BRIDGE_DECK_Y + 40, 6.0f, true);
        midpointCircle(bx1 - 30, BRIDGE_DECK_Y + 40, 6.0f, true);
    }
}

// Railway Tracks

void drawRailwayTracks() {
    glColor3f(0.3, 0.3, 0.32);
    drawRectangle(0, TRACK_Y - 14, WINDOW_WIDTH, 14,true);

    glColor3f(0.35, 0.22, 0.10);
    for (double x = 0; x < WINDOW_WIDTH; x += 26) {
        drawRectangle(x, TRACK_Y - 4, 16, 6, true);
    }

    glColor3f(0.75, 0.75, 0.78);
    ddaLine(0, TRACK_Y,WINDOW_WIDTH, TRACK_Y);
    ddaLine(0, TRACK_Y + TRACK_THICKNESS, WINDOW_WIDTH, TRACK_Y + TRACK_THICKNESS);
}

// Platform Drawingd
void drawPlatform() {
    glColor3f(0.62, 0.60, 0.58);
    drawRectangle(PLATFORM_LEFT, PLATFORM_TOP, PLATFORM_RIGHT - PLATFORM_LEFT, PLATFORM_HEIGHT, true);

    glColor3f(0.2, 0.2, 0.2);
    bresenhamLine((int)PLATFORM_LEFT, (int)PLATFORM_TOP, (int)PLATFORM_RIGHT, (int)PLATFORM_TOP);

    glColor3f(0.85, 0.75, 0.15);
    drawRectangle(PLATFORM_LEFT, PLATFORM_TOP + 2, PLATFORM_RIGHT - PLATFORM_LEFT, 5, true);
}

// Safety Line
void drawSafetyLine() {
    glColor3f(0.95, 0.85, 0.1);
    for (double x = PLATFORM_LEFT + 10; x < PLATFORM_RIGHT - 10; x += 20) {
        drawRectangle(x, SAFETY_LINE_Y, 12, 4, true);
    }
}

// Safety Line Crossing

bool platformCrossingPermitted() {
    const double FULLY_OPEN_THRESHOLD = 0.98;
    return doorOpenAmount >= FULLY_OPEN_THRESHOLD;
}

// Doors on Safety Line

void drawPlatformScreenDoors() {
    double doorY = SAFETY_LINE_Y + 6.0;
    double doorH = 46.0;
    double unitW = 64.0;
    double gap   = 66.0;

    bool crossingPermitted = platformCrossingPermitted();

    for (double cx = PLATFORM_LEFT + 110.0; cx < PLATFORM_RIGHT - 90.0; cx += gap + unitW) {
        glColor3f(0.30, 0.32, 0.35);
        drawRectangle(cx - unitW * 0.5 - 3, doorY + doorH, unitW + 6, 3, true);
        drawRectangle(cx - unitW * 0.5 - 4, doorY, 4, doorH, true);
        drawRectangle(cx + unitW * 0.5,doorY, 4, doorH, true);

        double leafW    = unitW / 2.0;
        double maxSlide = leafW * 0.92;
        double slide    = maxSlide * doorOpenAmount;

        glColor3f(0.55, 0.75, 0.85);
        Point2D leftBase  = {cx - leafW, doorY};
        Point2D leftOpen  = translatePoint(leftBase, -slide, 0.0);
        drawRectangle(leftOpen.x, leftOpen.y, leafW, doorH, true);

        Point2D rightBase = {cx, doorY};
        Point2D rightOpen = translatePoint(rightBase, slide, 0.0);
        drawRectangle(rightOpen.x, rightOpen.y, leafW, doorH, true);

        glColor3f(0.10, 0.10, 0.12);
        drawRectangle(leftOpen.x,  leftOpen.y,  leafW, doorH, false);
        drawRectangle(rightOpen.x, rightOpen.y, leafW, doorH, false);

        glColor3f(crossingPermitted ? 0.15 : 0.75, crossingPermitted ? 0.75 : 0.15, 0.15);
        drawRectangle(cx - 3, doorY + doorH + 6, 6, 6, true);
    }
}

// Waiting Area
void drawWaitingArea() {
    double sx = WAITING_X, sw = WAITING_W, sy = PLATFORM_TOP + PLATFORM_HEIGHT;
    glColor3f(0.5, 0.15, 0.15);
    drawRectangle(sx, sy + 60, sw, 8, true);
    glColor3f(0.3, 0.3, 0.3);
    drawRectangle(sx + 4,sy, 6, 60, true);
    drawRectangle(sx + sw - 10,sy, 6, 60, true);

    if (currentPalette.lampOn > 0.05) {
        glColor4f(1.0, 0.95, 0.6, 0.35 * currentPalette.lampOn);
        midpointCircle(sx + sw / 2.0, sy + 55, 20.0, true);
    }
}

// Bench
void drawBenches() {
    double benchY = PLATFORM_TOP + PLATFORM_HEIGHT + 4;
    double positions[] = { WAITING_X + 20, WAITING_X + 90, WAITING_X + 160 };
    for (double bx : positions) {
        glColor3f(0.42, 0.28, 0.14);
        drawRectangle(bx, benchY + 14, 44, 6, true);
        drawRectangle(bx, benchY + 20, 44, 22, false);
        glColor3f(0.2, 0.2, 0.2);
        drawRectangle(bx + 3, benchY, 4, 14, true);
        drawRectangle(bx + 37, benchY, 4, 14, true);
        glColor3f(0.3, 0.19, 0.09);
        for (int i = 0; i < 3; ++i) {
            bresenhamLine((int)(bx + 5), (int)(benchY + 16 + i * 4), (int)(bx + 39), (int)(benchY + 16 + i * 4));
        }
    }
}

// Ticket COunter
void drawTicketCounter() {
    double cx = TICKET_X, cy = PLATFORM_TOP + PLATFORM_HEIGHT, cw = TICKET_W, ch = 70.0;
    glColor3f(0.55, 0.45, 0.35);
    drawRectangle(cx, cy, cw, ch, true);

    double glassLit = 0.15 + 0.25 * currentPalette.lampOn;
    glColor3f(glassLit * 0.6, 0.5, 0.6);
    drawRectangle(cx + 15, cy + 30, cw - 30, 22, true);
    glColor3f(0.1, 0.1, 0.1);
    bresenhamLine((int)(cx + 15), (int)(cy + 30), (int)(cx + cw - 15), (int)(cy + 30));
    bresenhamLine((int)(cx + 15), (int)(cy + 52), (int)(cx + cw - 15), (int)(cy + 52));
    bresenhamLine((int)(cx + 15), (int)(cy + 30), (int)(cx + 15), (int)(cy + 52));
    bresenhamLine((int)(cx + cw - 15), (int)(cy + 30), (int)(cx + cw - 15), (int)(cy + 52));

    glColor3f(0.75, 0.15, 0.15);
    drawRectangle(cx - 6, cy + ch, cw + 12, 8, true);
    drawText(cx + 8, cy + ch + 16, "TICKETS", GLUT_BITMAP_HELVETICA_12, 1, 1, 1);
}

// Escalator
void drawEscalator() {
    double ex = ESCALATOR_X, ey = PLATFORM_TOP + PLATFORM_HEIGHT;
    double stepW = ESCALATOR_STEPW, stepH = 10.0;

    glColor3f(0.35, 0.35, 0.38);
    drawRectangle(ex, ey, ESCALATOR_STEPS * stepW, 14, true);

    glColor3f(0.6, 0.6, 0.65);
    for (int i = 0; i < ESCALATOR_STEPS; ++i) {
        double offset = fmod(escalatorOffset + i * stepW, ESCALATOR_STEPS * stepW);
        drawRectangle(ex + offset, ey + 14, stepW - 3, stepH, true);
    }
    glColor3f(0.2, 0.2, 0.2);
    bresenhamLine((int)ex, (int)(ey + 26), (int)(ex + ESCALATOR_STEPS * stepW), (int)(ey + 60));
    drawText(ex + 8, ey - 12, "ESCALATOR", GLUT_BITMAP_HELVETICA_10, 0.9, 0.9, 0.9);
}

// Elevator Shaft
void drawElevatorShaft() {
    double shaftX = ELEVATOR_X, shaftW = ELEVATOR_W;
    double shaftBottom = GROUND_Y + 20.0;
    double shaftTop = PLATFORM_TOP + PLATFORM_HEIGHT + 50.0;

    glColor3f(0.25, 0.25, 0.27);
    drawRectangle(shaftX, shaftBottom, shaftW, shaftTop - shaftBottom, false);

    glColor3f(0.55, 0.55, 0.6);
    drawRectangle(shaftX + 3, elevatorY, shaftW - 6, 34, true);

    double innerX = shaftX + 8, innerY = elevatorY + 4;
    double innerW = shaftW - 16, innerH = 26;
    glColor3f(0.10, 0.10, 0.12);
    drawRectangle(innerX, innerY, innerW, innerH, true);

    double leafW    = innerW / 2.0;
    double maxSlide = leafW * 0.95;
    double slide    = maxSlide * elevatorDoorOpenAmount;

    glColor3f(0.15, 0.55, 0.75);
    Point2D leftBase  = {innerX, innerY};
    Point2D leftOpen  = translatePoint(leftBase, -slide, 0.0);
    drawRectangle(leftOpen.x, leftOpen.y, leafW, innerH, true);

    Point2D rightBase = {innerX + leafW, innerY};
    Point2D rightOpen = translatePoint(rightBase, slide, 0.0);
    drawRectangle(rightOpen.x, rightOpen.y, leafW, innerH, true);

    glColor3f(0.05, 0.05, 0.05);
    drawRectangle(leftOpen.x,  leftOpen.y,  leafW, innerH, false);
    drawRectangle(rightOpen.x, rightOpen.y, leafW, innerH, false);

    drawText(shaftX - 4, shaftTop + 6, "ELEVATOR", GLUT_BITMAP_HELVETICA_10, 0.9, 0.9, 0.9);
}

// Stair
void drawStaircase() {
    double sx = STAIRCASE_X, sy = GROUND_Y + 10.0;
    double stepW = STAIRCASE_STEPW, stepH = (PLATFORM_TOP - sy) / STAIRCASE_STEPS;

    glColor3f(0.5, 0.5, 0.52);
    for (int i = 0; i < STAIRCASE_STEPS; ++i) {
        double x = sx + i * stepW;
        double y = sy + i * stepH;
        drawRectangle(x, y, stepW, stepH, true);
        glColor3f(0.15, 0.15, 0.15);
        bresenhamLine((int)x, (int)(y + stepH), (int)(x + stepW), (int)(y + stepH));
        glColor3f(0.5, 0.5, 0.52);
    }
    drawText(sx - 4, PLATFORM_TOP + PLATFORM_HEIGHT + 4, "", GLUT_BITMAP_HELVETICA_10, 0.9, 0.9, 0.9);
}

// Digital Clock
void drawDigitalWatch() {
    double dx = WATCH_PANEL_X, dy = WATCH_PANEL_Y, dw = WATCH_PANEL_W, dh = WATCH_PANEL_H;

    glColor3f(0.04, 0.05, 0.06);
    drawRectangle(dx, dy, dw, dh, true);
    glColor3f(0.5, 0.55, 0.55);
    bresenhamLine((int)dx, (int)dy, (int)(dx + dw), (int)dy);
    bresenhamLine((int)dx, (int)(dy + dh), (int)(dx + dw), (int)(dy + dh));
    bresenhamLine((int)dx, (int)dy, (int)dx, (int)(dy + dh));
    bresenhamLine((int)(dx + dw), (int)dy, (int)(dx + dw), (int)(dy + dh));

    const double cyanR = 0.25, cyanG = 0.9, cyanB = 0.95;

    drawText(dx + 12, dy + dh - 20, "NEXT: " + destinationText, GLUT_BITMAP_HELVETICA_12, cyanR, cyanG, cyanB);

    ostringstream timeLine;
    timeLine << "TIME: " << setfill('0') << setw(2) << clockHour << ":"
             << setfill('0') << setw(2) << clockMinute << ":"
             << setfill('0') << setw(2) << clockSecond;
    drawText(dx + 12, dy + dh / 2.0f - 2, timeLine.str(), GLUT_BITMAP_HELVETICA_12, cyanR, cyanG, cyanB);

    bool occupied = (trainState == TrainState::STOPPED || trainState == TrainState::DOORS_OPENING ||
                      trainState == TrainState::DOORS_OPEN || trainState == TrainState::DOORS_CLOSING ||
                      trainState == TrainState::BRAKING);
    string platformLine = string("PLATFORM 1 | ") + (occupied ? "OCCUPIED" : "CLEAR");
    drawText(dx + 12, dy + 12, platformLine, GLUT_BITMAP_HELVETICA_12, cyanR, cyanG, cyanB);
}

// Analog clock
void drawAnalogClock() {
    double ccx = CLOCK_CX, ccy = CLOCK_CY, r = CLOCK_R;

    glColor3f(0.95, 0.95, 0.9);
    midpointCircle(ccx, ccy, r, true);
    glColor3f(0.1f, 0.1f, 0.1f);
    midpointCircle(ccx, ccy, r, false);

    for (int i = 0; i < 12; ++i) {
        double angle = i * 30.0;
        Point2D outer = rotatePoint({ccx, ccy + r - 3}, {ccx, ccy}, angle);
        Point2D inner = rotatePoint({ccx, ccy + r - 8}, {ccx, ccy}, angle);
        bresenhamLine((int)inner.x, (int)inner.y, (int)outer.x, (int)outer.y);
    }

    double hourAngle   = -((clockHour % 12) * 30.0 + clockMinute * 0.5);
    double minuteAngle = -(clockMinute * 6.0);

    Point2D hourTip   = rotatePoint({ccx, ccy + r * 0.5}, {ccx, ccy}, hourAngle);
    Point2D minuteTip = rotatePoint({ccx, ccy + r * 0.8}, {ccx, ccy}, minuteAngle);

    glColor3f(0.1, 0.1, 0.1);
    bresenhamLine((int)ccx, (int)ccy, (int)hourTip.x, (int)hourTip.y);
    bresenhamLine((int)ccx, (int)ccy, (int)minuteTip.x, (int)minuteTip.y);
    glColor3f(0.7, 0.1, 0.1);
    midpointCircle(ccx, ccy, 2.0, true);
}

// Station Signboard
void drawPlatformSignboards() {
    struct Sign {
        double x;
        const char *label;
    };
    static const Sign signs[] = { {200, "UTTARA CENTRAL"}, {950, "UTTARA CENTRAL"} };
    for (const auto &s : signs) {
        double y = PLATFORM_TOP + PLATFORM_HEIGHT + 100;
        glColor3f(0.15, 0.15, 0.15);
        drawRectangle(s.x, PLATFORM_TOP + PLATFORM_HEIGHT, 4, 100, true);
        glColor3f(0.05, 0.35, 0.15);
        drawRectangle(s.x - 60, y, 130, 24, true);
        drawText(s.x - 52, y + 8, s.label, GLUT_BITMAP_HELVETICA_12, 1, 1, 1);
    }
}

// Signal Indicator
void drawSignalSystem() {
    double px = 20.0, poleTop = TRACK_Y + 130.0;
    glColor3f(0.15, 0.15, 0.15);
    ddaLine(px, TRACK_Y, px, poleTop);

    double boxX = px - 12, boxY = poleTop - 6, boxW = 24, boxH = 72;
    glColor3f(0.08, 0.08, 0.08);
    drawRectangle(boxX, boxY, boxW, boxH, true);

    float redY = boxY + boxH - 16, yellowY = boxY + boxH / 2, greenY = boxY + 16;

    auto drawLamp = [&](double cy, double r, double g, double b, bool active) {
        if (active) {
            glColor4f(r, g, b, 0.25);
            midpointCircle(px, cy, 15.0, true);
        }
        glColor3f(active ? r : r * 0.25, active ? g : g * 0.25, active ? b : b * 0.25);
        midpointCircle(px, cy, 9.0, true);
    };

    drawLamp(redY,    1.0, 0.1, 0.1, signalState == SignalState::RED);
    drawLamp(yellowY, 1.0, 0.85, 0.1, signalState == SignalState::YELLOW);
    drawLamp(greenY,  0.1, 0.9, 0.2, signalState == SignalState::GREEN);

    drawText(boxX - 24, boxY + boxH + 14, autoMode ? "AUTO" : "MANUAL", GLUT_BITMAP_HELVETICA_10, 1, 1, 0.6);
}

void drawSignalReflectionStrip() {
    double r, g, b;
    switch (signalState) {
        case SignalState::RED:    r = 1.0; g = 0.10; b = 0.10;
        break;
        case SignalState::YELLOW: r = 1.0; g = 0.85; b = 0.10;
        break;
        case SignalState::GREEN:  r = 0.10; g = 0.90; b = 0.20;
        break;
    }
    glColor4f(r, g, b, 0.12);
    drawRectangle(PLATFORM_LEFT, SAFETY_LINE_Y - 3.0, PLATFORM_RIGHT - PLATFORM_LEFT, 3.0, true);
}

// Stating Lighting
void drawStationLighting() {
    double lampY = PLATFORM_TOP + PLATFORM_HEIGHT + 90.0;
    for (double lx = 250.0; lx < PLATFORM_RIGHT - 40; lx += 220.0) {
        glColor3f(0.2, 0.2, 0.2);
        drawRectangle(lx, PLATFORM_TOP + PLATFORM_HEIGHT, 5, 90, true);

        if (currentPalette.lampOn > 0.03) {
            glColor4f(1.0, 0.95, 0.6, 0.15 * currentPalette.lampOn);
            midpointCircle(lx + 2, lampY, 34.0f, true);
        }
        double on = currentPalette.lampOn;
        glColor3f(0.6 + 0.4 * on, 0.6 + 0.35 * on, 0.55 - 0.0 * on);
        midpointCircle(lx + 2, lampY, 8.0, true);
    }
}

// Ticket gate
void drawTicketGates() {
    double baseX = TICKET_X + TICKET_W + 8.0;
    double baseY = PLATFORM_TOP + PLATFORM_HEIGHT;

    for (int i = 0; i < 2; ++i) {
        double gx = baseX + i * 18.0;
        glColor3f(0.25, 0.25, 0.27);
        drawRectangle(gx, baseY, 5, 30, true);

        double angle = 45.0 + 45.0 * sin(gateAnimTime + i * 1.3);
        bool  open  = angle > 55.0;

        Point2D armBase = { gx + 2.5, baseY + 28.0 };
        Point2D armRest = { gx + 2.5, baseY + 28.0 + 24.0 };
        Point2D armTip  = rotatePoint(armRest, armBase, angle);
        glColor3f(0.85, 0.75, 0.15);
        bresenhamLine((int)armBase.x, (int)armBase.y, (int)armTip.x, (int)armTip.y);

        glColor3f(open ? 0.1 : 0.8, open ? 0.8 : 0.1, 0.1);
        midpointCircle(gx + 2.5, baseY + 34.0, 3.0, true);
    }
    drawText(baseX - 8, baseY + 50, "ENTRY GATES", GLUT_BITMAP_HELVETICA_10, 0.9, 0.9, 0.9);
}

// Lamp Light
void drawWetReflections() {
    if (activeRainDropCount() == 0)
        return;
    if (currentPalette.lampOn < 0.3)
        return;

    double lampY = PLATFORM_TOP + PLATFORM_HEIGHT + 90.0;
    for (double lx = 250.0; lx < PLATFORM_RIGHT - 40; lx += 220.0) {
        (void) lampY;
        glColor4f(1.0, 0.95, 0.6, 0.14 * currentPalette.lampOn);
        drawRectangle(lx - 6, PLATFORM_TOP - 26, 16, 26, true);
    }
}

void updateGates() {
    gateAnimTime += 0.05;
}

// Coach Draw
void drawCoach(double x, double y, double w, double h, bool isCabin, bool front, int coachIndex) {

    glColor3f(0.90, 0.91, 0.93);
    drawRectangle(x, y, w, h, true);

    glColor3f(0.78, 0.10, 0.14);
    drawRectangle(x, y, w, h * 0.18, true);
    glColor3f(0.10, 0.30, 0.62);
    drawRectangle(x, y + h * 0.45, w, h * 0.30, true);

    glColor3f(0.08, 0.08, 0.08);
    drawRectangle(x, y, w, h, false);

    glColor3f(0.55, 0.57, 0.6);
    drawRectangle(x, y + h - 8, w, 8, true);

    if (!isCabin && coachIndex == 1) {
        glColor3f(0.2, 0.2, 0.22);
        double pcx = x + w * 0.5;
        bresenhamLine((int)(pcx - 14), (int)(y + h), (int)pcx, (int)(y + h + 16));
        bresenhamLine((int)(pcx + 14), (int)(y + h), (int)pcx, (int)(y + h + 16));
        bresenhamLine((int)(pcx - 10), (int)(y + h + 16), (int)(pcx + 10), (int)(y + h + 16));
    }

    if (isCabin) {
        double nx = front ? x + w : x;
        double dir = front ? 1.0 : -1.0;
        Point2D p0 = {nx, y};
        Point2D p1 = {nx + dir * 30, y};
        Point2D p2 = {nx + dir * 30, y + h * 0.62};
        Point2D p3 = {nx, y + h};
        glColor3f(0.78, 0.10, 0.14);
        bezierCurve(p0, p1, p2, p3, 28);

        Point2D q1 = {nx + dir * 18, y + h * 0.12};
        Point2D q2 = {nx + dir * 18, y + h * 0.55};
        bezierCurve(p0, q1, q2, p3, 24);

        glColor3f(0.12, 0.32, 0.48);
        drawRectangle(x + w * 0.30, y + h * 0.55, w * 0.40, h * 0.30, true);
        glColor3f(0.05, 0.05, 0.05);
        drawRectangle(x + w * 0.30, y + h * 0.55, w * 0.40, h * 0.30, false);

        glColor3f(0.9, 0.85, 0.2);
        midpointCircle(x + w * 0.24, y + h * 0.68, 3.0, true);
        midpointCircle(x + w * 0.76, y + h * 0.68, 3.0, true);
    } else {
        int windowCount = 4;
        double wSpacing = w / (windowCount + 1);
        double wW = wSpacing * 0.6, wH = h * 0.30;
        double wY = y + h * 0.5;
        double lit = currentPalette.lampOn;
        for (int i = 0; i < windowCount; ++i) {
            double wx = x + wSpacing * (i + 1) - wW / 2.0;
            glColor3f(0.55 + 0.40 * lit, 0.70 + 0.20 * lit, 0.85 - 0.10 * lit);
            drawRectangle(wx, wY, wW, wH, true);
            glColor3f(0.05, 0.05, 0.05);
            bresenhamLine((int)wx, (int)wY, (int)(wx + wW), (int)wY);
            bresenhamLine((int)wx, (int)(wY + wH), (int)(wx + wW), (int)(wY + wH));
            bresenhamLine((int)wx, (int)wY, (int)wx, (int)(wY + wH));
            bresenhamLine((int)(wx + wW), (int)wY, (int)(wx + wW), (int)(wY + wH));
        }

        double doorW = w * 0.15, doorH = h * 0.60;
        double doorY = y + h * 0.20;
        double doorCX = x + w / 2.0;

        double maxSlide = doorW * 0.95;
        double slide = maxSlide * doorOpenAmount;

        glColor3f(0.65, 0.55, 0.15);
        Point2D leftBase = {doorCX - doorW, doorY};
        Point2D leftOpen = translatePoint(leftBase, -slide, 0.0);
        drawRectangle(leftOpen.x, leftOpen.y, doorW, doorH, true);

        Point2D rightBase = {doorCX, doorY};
        Point2D rightOpen = translatePoint(rightBase, slide, 0.0);
        drawRectangle(rightOpen.x, rightOpen.y, doorW, doorH, true);

        glColor3f(0.05, 0.05, 0.05);
        drawRectangle(leftOpen.x, leftOpen.y, doorW, doorH, false);
        drawRectangle(rightOpen.x, rightOpen.y, doorW, doorH, false);
    }
}

// Draws bogie
void drawBogie(double x, double w) {
    double bogieCX = x + w / 2.0;
    double frameY  = TRAIN_BASE_Y - WHEEL_RADIUS * 0.9;

    glColor3f(0.15, 0.15, 0.16);
    drawRectangle(bogieCX - 26, frameY, 52, 10, true);

    double wheelY = TRAIN_BASE_Y - WHEEL_RADIUS * 0.4;
    double positions[] = { bogieCX - 16.0, bogieCX + 16.0 };
    for (double wx : positions) {
        glColor3f(0.1, 0.1, 0.1);
        midpointCircle(wx, wheelY, WHEEL_RADIUS, true);
        glColor3f(0.6, 0.6, 0.6);
        midpointCircle(wx, wheelY, WHEEL_RADIUS * 0.35, true);

        Point2D spokeTip = rotatePoint({wx, wheelY + WHEEL_RADIUS - 2}, {wx, wheelY}, wheelRotationAngle);
        glColor3f(0.85, 0.85, 0.1);
        bresenhamLine((int)wx, (int)wheelY, (int)spokeTip.x, (int)spokeTip.y);
    }
}

// Connector for cart to cart
void drawConnector(double x, double y, double w, double h) {
    glColor3f(0.12, 0.12, 0.13);
    drawRectangle(x, y + h * 0.15, w, h * 0.6, true);
    glColor3f(0.05, 0.05, 0.05);
    for (double cy = y + h * 0.2; cy < y + h * 0.7; cy += 6.0) {
        bresenhamLine((int)x, (int)cy, (int)(x + w), (int)cy);
    }
}

// Lights
void drawHeadTailLights() {
    double frontX = trainX + NUM_COACHES * (COACH_WIDTH + COACH_GAP) + CABIN_WIDTH - 8;
    double rearX = trainX + 6;
    double lightY = TRAIN_BASE_Y + TRAIN_HEIGHT * 0.55;

    if (currentPalette.lampOn > 0.05) {
        glColor4f(1.0, 1.0, 0.7, 0.2 * currentPalette.lampOn);
        midpointCircle(frontX + 30, lightY, 22.0, true);
    }
    double warm = currentPalette.lampOn;
    glColor3f(1.0, 1.0, 0.85 - 0.25 * warm);
    midpointCircle(frontX, lightY + 14, 5.0, true);
    midpointCircle(frontX, lightY - 14, 5.0f, true);

    glColor3f(0.85, 0.1, 0.1);
    midpointCircle(rearX, lightY + 14, 4.5, true);
    midpointCircle(rearX, lightY - 14, 4.5, true);
}

void drawTrain() {
    if (trainState == TrainState::OFFSCREEN_LEFT) return;

    double x = trainX;
    for (int i = 0; i < NUM_COACHES; ++i) {
        drawCoach(x, TRAIN_BASE_Y, COACH_WIDTH, TRAIN_HEIGHT, false, false, i);
        drawBogie(x + COACH_WIDTH * 0.18 - 26, 52);
        drawBogie(x + COACH_WIDTH * 0.82 - 26, 52);
        if (i > 0) {
            drawConnector(x - COACH_GAP, TRAIN_BASE_Y + TRAIN_HEIGHT * 0.15, COACH_GAP, TRAIN_HEIGHT * 0.7);
        }
        x += COACH_WIDTH + COACH_GAP;
    }
    drawConnector(x - COACH_GAP, TRAIN_BASE_Y + TRAIN_HEIGHT * 0.15, COACH_GAP, TRAIN_HEIGHT * 0.7);
    drawCoach(x, TRAIN_BASE_Y, CABIN_WIDTH, TRAIN_HEIGHT, true, true, NUM_COACHES);
    drawBogie(x + CABIN_WIDTH * 0.20 - 26, 52);
    drawBogie(x + CABIN_WIDTH * 0.80 - 26, 52);

    drawHeadTailLights();
}

// passengers
void initPassengers() {
    walkingPassengers.clear();
    waitingPassengers.clear();
    queuePassengers.clear();
    exitingPassengers.clear();

    double  shirtPalette[][3] = {
        {0.8, 0.2, 0.2}, {0.2, 0.4, 0.8}, {0.2, 0.7, 0.3},
        {0.85, 0.65, 0.1}, {0.6f, 0.3f, 0.7}, {0.3, 0.3, 0.3}
    };

    for (int i = 0; i < NUM_WALKING_PASSENGERS; ++i) {
        Passenger p;
        p.x = PLATFORM_LEFT + 150 + (i * 160) % (int)(PLATFORM_RIGHT - PLATFORM_LEFT - 200);
        p.baseY = PLATFORM_TOP + PLATFORM_HEIGHT;
        p.y = p.baseY;
        p.speed = 0.6 + 0.15 * (i % 3);
        p.phase = (double) i * 0.8;
        p.direction = (i % 2 == 0) ? 1 : -1;
        p.activity = PassengerActivity::WALKING;
        p.r = shirtPalette[i % 6][0]; p.g = shirtPalette[i % 6][1]; p.b = shirtPalette[i % 6][2];
        p.active = true;
        walkingPassengers.push_back(p);
    }

    for (int i = 0; i < NUM_WAITING_PASSENGERS; ++i) {
        Passenger p;
        p.x = WAITING_X + 20.0 + i * 45.0;
        p.baseY = PLATFORM_TOP + PLATFORM_HEIGHT;
        p.y = p.baseY;
        p.speed = 0.0;
        p.phase = 0.0;
        p.direction = 1;
        p.activity = PassengerActivity::WAITING;
        p.r = shirtPalette[(i + 2) % 6][0]; p.g = shirtPalette[(i + 2) % 6][1]; p.b = shirtPalette[(i + 2) % 6][2];
        p.active = true;
        waitingPassengers.push_back(p);
    }

    for (int i = 0; i < NUM_QUEUE_PASSENGERS; ++i) {
        Passenger p;
        p.x = TICKET_X + 140.0 - i * 22.0;
        p.baseY = PLATFORM_TOP + PLATFORM_HEIGHT;
        p.y = p.baseY;
        p.speed = 0.0;
        p.phase = 0.0;
        p.direction = 1;
        p.activity = PassengerActivity::QUEUING;
        p.r = shirtPalette[(i + 4) % 6][0]; p.g = shirtPalette[(i + 4) % 6][1]; p.b = shirtPalette[(i + 4) % 6][2];
        p.active = true;
        queuePassengers.push_back(p);
    }
}

void drawPassengerFigure(const Passenger &p) {
    double  legSwing = (p.activity == PassengerActivity::WALKING ||
                       p.activity == PassengerActivity::EXITING) ? std::sin(p.phase) * 4.0 : 0.0;

    glColor3f(0.15, 0.15, 0.2);
    drawRectangle(p.x - 4 + legSwing * 0.2, p.y, 3, 14, true);
    drawRectangle(p.x + 1 - legSwing * 0.2, p.y, 3, 14, true);

    glColor3f(p.r, p.g, p.b);
    drawRectangle(p.x - 5, p.y + 14, 10, 16, true);

    glColor3f(p.r * 0.8, p.g * 0.8, p.b * 0.8);
    bresenhamLine((int)(p.x - 5), (int)(p.y + 26), (int)(p.x - 8 + legSwing), (int)(p.y + 16));
    bresenhamLine((int)(p.x + 5), (int)(p.y + 26), (int)(p.x + 8 - legSwing), (int)(p.y + 16));

    glColor3f(0.95, 0.8, 0.65);
    midpointCircle(p.x, p.y + 34, 5.0, true);
}

void drawWalkingPassengers() {
    for (const auto &p : walkingPassengers)
        if (p.active)
            drawPassengerFigure(p);
}

void drawWaitingPassengers() {
    for (const auto &p : waitingPassengers)
        if (p.active)
            drawPassengerFigure(p);
}

void drawQueue() {
    for (const auto &p : queuePassengers)
        if (p.active)
            drawPassengerFigure(p);
    drawText(TICKET_X + 10, PLATFORM_TOP + PLATFORM_HEIGHT + 60, "TICKET QUEUE", GLUT_BITMAP_HELVETICA_10, 0.9, 0.9, 0.9);
}

void drawStationStaff() {
    Passenger staff{};
    staff.r = 0.05; staff.g = 0.25; staff.b = 0.55;
    staff.x = WAITING_X - 60.0; staff.y = PLATFORM_TOP + PLATFORM_HEIGHT; staff.activity = PassengerActivity::WAITING;
    drawPassengerFigure(staff);
    drawText(staff.x - 22, staff.y + 42, "STAFF", GLUT_BITMAP_HELVETICA_10, 1, 1, 1);

    Passenger security{};
    security.r = 0.15; security.g = 0.15; security.b = 0.15;
    security.x = STAIRCASE_X + 60.0; security.y = PLATFORM_TOP + PLATFORM_HEIGHT; security.activity = PassengerActivity::WAITING;
    drawPassengerFigure(security);
    drawText(security.x - 26, security.y + 42, "SECURITY", GLUT_BITMAP_HELVETICA_10, 1, 1, 1);
}

// Animation for Walking
void updatePassengers() {
    for (auto &p : walkingPassengers) {
        Point2D moved = translatePoint({p.x, p.y}, p.speed * p.direction, 0.0);
        p.x = moved.x;
        p.phase += 0.15;
        if (p.x < PLATFORM_LEFT + 60)
            {
            p.x = PLATFORM_LEFT + 60;
            p.direction = 1;
            }
        if (p.x > PLATFORM_RIGHT - 60)
        {
            p.x = PLATFORM_RIGHT - 60; p.direction = -1;
        }
    }

    bool doorsAreOpen = platformCrossingPermitted();
    if (doorsAreOpen && boardingTicksLeft > 0) {
        double doorXNear = trainX + COACH_WIDTH * 1.5;
        for (auto &p : waitingPassengers) {
            if (!p.active) continue;
            float dir = (doorXNear > p.x) ? 1.0 : -1.0;
            p.x += dir * 1.4;
            p.phase += 0.2;
            if (fabs(p.x - doorXNear) < 6.0) {
                p.active = false;
                passengerCount++;
            }
        }
        boardingTicksLeft--;
    }

    updateExitingPassengers();
}

void spawnExitingPassengers() {
    double doorX = trainX + COACH_WIDTH * 1.5;
    double shirtPalette[][3] = {
        {0.9, 0.5, 0.1}, {0.1, 0.6, 0.6}, {0.7, 0.7, 0.2}
    };
    int spawnCount = 2 + (rand() % 2);
    for (int i = 0; i < spawnCount; ++i) {
        Passenger p;
        p.x = doorX + (i - 1) * 8.0;
        p.baseY = PLATFORM_TOP + PLATFORM_HEIGHT;
        p.y = p.baseY;
        p.speed = 0.8 + 0.2 * i;
        p.phase = (double) i;
        p.direction = (i % 2 == 0) ? 1 : -1;
        p.activity = PassengerActivity::EXITING;
        p.r = shirtPalette[i % 3][0]; p.g = shirtPalette[i % 3][1]; p.b = shirtPalette[i % 3][2];
        p.active = true;
        exitingPassengers.push_back(p);
    }
}

void drawExitingPassengers() {
    for (const auto &p : exitingPassengers)
        if (p.active)
            drawPassengerFigure(p);
}

void updateExitingPassengers() {
    for (auto &p : exitingPassengers) {
        if (!p.active)
            continue;
        Point2D moved = translatePoint({p.x, p.y}, p.speed * p.direction, 0.0);
        p.x = moved.x;
        p.phase += 0.18;
        if (p.x < PLATFORM_LEFT + 20 || p.x > PLATFORM_RIGHT - 20) {
            p.active = false;
            exitedPassengerCount++;
        }
    }
    exitingPassengers.erase(
        remove_if(exitingPassengers.begin(), exitingPassengers.end(),
                        [](const Passenger &p)
                        {
                            return !p.active;
                        }),
        exitingPassengers.end());
}

// Rain
void initRain() {
    rainDrops.clear();
    for (int i = 0; i < NUM_RAINDROPS_MAX; ++i) {
        RainDrop d;
        d.x = (double) (rand() % WINDOW_WIDTH);
        d.y = (double) (rand() % WINDOW_HEIGHT);
        d.length = 8.0 + (rand() % 10);
        d.speed  = 6.0 + (rand() % 6);
        rainDrops.push_back(d);
    }
}

// Heavy or Normal Rain
int activeRainDropCount() {
    switch (rainIntensity) {
        case RainIntensity::NONE:
            return 0;
        case RainIntensity::LIGHT:
            return NUM_RAINDROPS_MAX / 3;
        case RainIntensity::HEAVY:
            return NUM_RAINDROPS_MAX;
    }
    return 0;
}

// Rain
void drawRain() {
    int count = activeRainDropCount();
    if (count == 0)
        return;

    bool heavy = (rainIntensity == RainIntensity::HEAVY);
    glColor4f(0.6, 0.7, 0.9, heavy ? 0.75 : 0.55);
    for (int i = 0; i < count; ++i) {
        const RainDrop &d = rainDrops[i];
        double lengthMul = heavy ? 1.4 : 1.0;
        ddaLine(d.x, d.y, d.x - (heavy ? 6.0 : 4.0), d.y - d.length * lengthMul);
    }

    glColor4f(0.1, 0.15, 0.25, heavy ? 0.24 : 0.15);
    drawRectangle(0, GROUND_Y, WINDOW_WIDTH, PLATFORM_TOP + PLATFORM_HEIGHT, true);
}

// Rain animation
void updateRain() {
    int count = activeRainDropCount();
    if (count == 0)
        return;

    bool heavy = (rainIntensity == RainIntensity::HEAVY);
    double speedMul = heavy ? 1.6 : 1.0;
    for (int i = 0; i < count; ++i) {
        RainDrop &d = rainDrops[i];
        Point2D moved = translatePoint({d.x, d.y}, heavy ? -1.6 : -1.0, -d.speed * speedMul);
        d.x = moved.x;
        d.y = moved.y;
        if (d.y < 0) {
            d.y = (double) WINDOW_HEIGHT;
            d.x = (double) (rand() % WINDOW_WIDTH);
        }
        if (d.x < 0) d.x = (double) WINDOW_WIDTH;
    }
}

// Instructions
string trainStateLabel() {
    switch (trainState) {
        case TrainState::OFFSCREEN_LEFT:
            return "AWAITING ARRIVAL (press A)";
        case TrainState::APPROACHING:
            return "APPROACHING STATION";
        case TrainState::BRAKING:
            return "BRAKING";
        case TrainState::STOPPED:
            return "STOPPED AT PLATFORM";
        case TrainState::DOORS_OPENING:
            return "DOORS OPENING";
        case TrainState::DOORS_OPEN:
            return "DOORS OPEN - BOARDING";
        case TrainState::DOORS_CLOSING:
            return "DOORS CLOSING";
        case TrainState::DEPARTING:
            return "DEPARTING";
        case TrainState::EMERGENCY_STOPPED:
            return "EMERGENCY STOP ENGAGED";
    }
    return "";
}

string signalStateLabel() {
    switch (signalState) {
        case SignalState::RED:
            return "RED - STOP";
        case SignalState::YELLOW:
            return "YELLOW - PREPARE";
        case SignalState::GREEN:
            return "GREEN - PROCEED";
    }
    return "";
}

string cameraViewLabel() {
    switch (cameraView) {
        case 0:
            return "FULL STATION";
        case 1:
            return "PLATFORM & TRAIN";
        case 2:
            return "SIGNAL CLOSE-UP";
        case 3:
            return "SKY & STATION WIDE";
        default:
            return "FULL STATION";
    }
}

string lightingModeLabel() {
    if (eveningPreviewTicks > 0) return "EVENING (PREVIEW)";
    switch (activeLightMode) {
        case LightingMode::DAY:
            return "DAY";
        case LightingMode::EVENING:
            return "EVENING";
        case LightingMode::NIGHT:
            return "NIGHT";
    }
    return "DAY";
}

// Station Info
void drawHUD() {
    glColor4f(0.0, 0.0, 0.0, 0.4);
    drawRectangle(0, WINDOW_HEIGHT - 54, WINDOW_WIDTH, 54, true);

    ostringstream line1;
    line1 << "TRAIN: " << trainStateLabel()
          << "   |   SIGNAL: " << signalStateLabel() << " (" << (autoMode ? "AUTO" : "MANUAL") << ")"
          << "   |   LIGHTING: " << lightingModeLabel()
          << "   |   CAMERA: " << cameraViewLabel();
    drawText(14, WINDOW_HEIGHT - 20, line1.str(), GLUT_BITMAP_HELVETICA_12, 1, 1, 1);

    ostringstream line2;
    line2 << "BOARDED: " << passengerCount << "  EXITED: " << exitedPassengerCount
          << "   |   RAIN: " << (rainIntensity == RainIntensity::NONE ? "OFF" :
                                   rainIntensity == RainIntensity::LIGHT ? "LIGHT" : "HEAVY")
          << "   |   FOG: " << (isFoggy ? "ON" : "OFF");
    if (isPaused) line2 << "     ***  PAUSED  ***";
    drawText(14, WINDOW_HEIGHT - 40, line2.str(), GLUT_BITMAP_HELVETICA_12,
              isPaused ? 1.0 : 1.0, isPaused ? 0.3 : 1.0, isPaused ? 0.3 : 1.0);

    if (announcementTimer > 0) {
        drawText(14, WINDOW_HEIGHT - 70, "ANNOUNCEMENT: " + announcementText,
                  GLUT_BITMAP_HELVETICA_12, 1.0, 0.85, 0.2);
    }
}

void drawControlsPanel() {
    glColor4f(0.0, 0.0, 0.0, 0.45);
    drawRectangle(0, 0, 430, 168, true);

    drawText(10, 148, "CONTROLS", GLUT_BITMAP_HELVETICA_12, 1, 1, 1);

    const char *colA[] = {
        "A: Train Arrives",
        "D: Train Departs",
        "S: Emergency Stop",
        "G: Change Signal",
        "O: Open Doors",
        "C: Close Doors",
        "P: Pause / Resume"
    };
    const char *colB[] = {
        "L: Evening Preview",
        "N: Cycle Day/Eve/Night",
        "M: Auto / Manual Mode",
        "F: Toggle Fog",
        "R: Toggle Light Rain",
        "T: Toggle Heavy Rain",
        "V: Cycle Camera / Esc: Exit"
    };
    for (int i = 0; i < 7; ++i) {
        drawText(10,  126 - i * 17, colA[i], GLUT_BITMAP_HELVETICA_10, 0.95, 0.95, 0.95);
        drawText(225, 126 - i * 17, colB[i], GLUT_BITMAP_HELVETICA_10, 0.95, 0.95, 0.95);
    }
}

// Train speed
void drawSpeedGauge() {
    const double x = WINDOW_WIDTH - 180.0;
    const double y = WINDOW_HEIGHT - 65.0;
    const double w = 165.0;
    const double h = 55.0;

    // Shadow
    glColor4f(0.0, 0.0, 0.0, 0.35);
    drawRectangle(x + 4, y - 4, w, h, true);

    // Outer frame
    glColor3f(0.75, 0.78, 0.82);
    drawRectangle(x, y, w, h, true);

    // Inner LCD
    glColor3f(0.05, 0.08, 0.10);
    drawRectangle(x + 3, y + 3, w - 6, h - 6, true);

    // LCD glow
    glColor4f(0.0, 0.8, 1.0, 0.12);
    drawRectangle(x + 3, y + 3, w - 6, h - 6, true);

    // Label
    drawText(x + 12,
             y + h - 15,
             "TRAIN SPEED",
             GLUT_BITMAP_HELVETICA_10,
             1.0, 1.0, 1.0);

    // Speed value
    ostringstream ss;
    ss << setw(3) << setfill('0')
       << (int)round(trainSpeed * 20);

    drawText(x + 18,
             y + 18,
             ss.str() + " KM/H",
             GLUT_BITMAP_HELVETICA_18,
             0.20, 1.f, 0.35);
}

void logEvent(const string &msg) {
    cout << "[" << setfill('0') << setw(2) << clockHour << ":"
               << setfill('0') << setw(2) << clockMinute << ":"
               << setfill('0') << setw(2) << clockSecond << "] "
               << msg << endl;
}

// Train Animation
void updateTrain() {
    if (trainState == TrainState::EMERGENCY_STOPPED)
        return;

    switch (trainState) {
        case TrainState::OFFSCREEN_LEFT:
            break;

        case TrainState::APPROACHING: {
            Point2D moved = translatePoint({trainX, 0}, trainSpeed, 0.0);
            trainX = moved.x;
            double trainFrontX = trainX + NUM_COACHES * (COACH_WIDTH + COACH_GAP) + CABIN_WIDTH;
            if (trainFrontX >= PLATFORM_STOP_X + 250.0) {
                trainState = TrainState::BRAKING;
            }
            break;
        }

        case TrainState::BRAKING: {
            trainSpeed = max(MIN_SPEED * 0.3, trainSpeed * 0.92);
            Point2D moved = translatePoint({trainX, 0}, trainSpeed, 0.0);
            trainX = moved.x;
            if (trainSpeed <= MIN_SPEED * 0.35) {
                trainState = TrainState::STOPPED;
                trainSpeed = 0.0;
                announcementText = "TRAIN NOW ARRIVING - PLEASE STAND CLEAR OF THE DOORS";
                announcementTimer = 90;
                logEvent("Train stopped at platform.");
            }
            break;
        }

        case TrainState::STOPPED:
            break;

        case TrainState::DOORS_OPENING:
            doorOpenAmount = min(1.0, doorOpenAmount + 0.04);
            if (doorOpenAmount >= 1.0) {
                trainState = TrainState::DOORS_OPEN;
                boardingTicksLeft = 160;
                spawnExitingPassengers();
                announcementText = "DOORS OPEN - BOARDING NOW";
                announcementTimer = 90;
                logEvent("Doors open - boarding started.");
            }
            break;

        case TrainState::DOORS_OPEN:
            if (boardingTicksLeft <= 0) {
                trainState = TrainState::DOORS_CLOSING;
                announcementText = "DOORS CLOSING - PLEASE STAND CLEAR";
                announcementTimer = 90;
            }
            break;

        case TrainState::DOORS_CLOSING:
            doorOpenAmount = max(0.0, doorOpenAmount - 0.04);
            if (doorOpenAmount <= 0.0) {
                trainState = TrainState::STOPPED;
            }
            break;

        case TrainState::DEPARTING: {
            trainSpeed = min(MAX_SPEED, trainSpeed + 0.08);
            Point2D moved = translatePoint({trainX, 0}, trainSpeed, 0.0);
            trainX = moved.x;
            if (trainX > WINDOW_WIDTH + 50) {
                trainState = TrainState::OFFSCREEN_LEFT;
                trainX = -400.0;
                trainSpeed = 3.0;
                advanceToNextStation();
                announcementText = "WELCOME - NEXT ARRIVAL SHORTLY";
                announcementTimer = 90;
                logEvent("Train departed. Next destination: " + destinationText);
            }
            break;
        }

        case TrainState::EMERGENCY_STOPPED:
            break;
    }

    // Wheels Rotation
    wheelRotationAngle = fmod(wheelRotationAngle + trainSpeed * 6.0, 360.0);

    if (announcementTimer > 0)
        announcementTimer--;
}

// Signal
void updateSignalFromTrainState() {
    if (!autoMode)
        return;

    switch (trainState) {
        case TrainState::OFFSCREEN_LEFT:
        case TrainState::APPROACHING:
            signalState = SignalState::GREEN;
            break;
        case TrainState::BRAKING:
            signalState = SignalState::YELLOW;
            break;
        case TrainState::STOPPED:
        case TrainState::DOORS_OPENING:
        case TrainState::DOORS_OPEN:
        case TrainState::DOORS_CLOSING:
            signalState = SignalState::RED;
            break;
        case TrainState::DEPARTING:
            signalState = SignalState::YELLOW;
            break;
        case TrainState::EMERGENCY_STOPPED:
            signalState = SignalState::RED;
            break;
    }
}

void requestDoorsOpen() {
    if (trainState == TrainState::STOPPED) {
        trainState = TrainState::DOORS_OPENING;
    }
}
void requestDoorsClose() {
    if (trainState == TrainState::DOORS_OPEN) {
        trainState = TrainState::DOORS_CLOSING;
        boardingTicksLeft = 0;
    }
}

void triggerBoarding() {
    if (platformCrossingPermitted()) {
        boardingTicksLeft = max(boardingTicksLeft, 100);
        for (auto &p : queuePassengers) p.active = true;
    }
}


void triggerEmergencyStop() {
    if (!emergencyStopFlag) {
        emergencyStopFlag = true;
        trainState = TrainState::EMERGENCY_STOPPED;
        trainSpeed = 0.0;
        announcementText = "EMERGENCY STOP - ATTENTION PLEASE";
        announcementTimer = 150;
        logEvent("EMERGENCY STOP engaged.");
    } else {
        emergencyStopFlag = false;
        double trainFrontX = trainX + NUM_COACHES * (COACH_WIDTH + COACH_GAP) + CABIN_WIDTH;
        if (trainX < -50) trainState = TrainState::OFFSCREEN_LEFT;
        else if (trainFrontX < PLATFORM_STOP_X + 250.0) trainState = TrainState::APPROACHING;
        else trainState = TrainState::STOPPED;
        trainSpeed = 3.0;
        logEvent("Emergency stop released.");
    }
}


void cycleSignalManual() {
    if (autoMode) {
        announcementText = "SWITCH TO MANUAL MODE (M) TO CONTROL THE SIGNAL";
        announcementTimer = 80;
        return;
    }
    switch (signalState) {
        case SignalState::RED:
            signalState = SignalState::YELLOW;
            break;
        case SignalState::YELLOW:
            signalState = SignalState::GREEN;
            break;
        case SignalState::GREEN:
            signalState = SignalState::RED;
            break;
    }
    logEvent("Signal manually changed to " + signalStateLabel());
}

// Train Coming
void requestTrainArrival() {
    if (trainState != TrainState::OFFSCREEN_LEFT)
        return;
    if (signalState != SignalState::GREEN) {
        announcementText = "CANNOT ARRIVE - SIGNAL IS NOT GREEN";
        announcementTimer = 80;
        return;
    }
    trainState = TrainState::APPROACHING;
    trainSpeed = 3.0;
    announcementText = "TRAIN APPROACHING PLATFORM 1";
    announcementTimer = 90;
    logEvent("Train arrival requested - approaching.");
}

void updateClockTime() {
    tickCounter++;
    if (tickCounter >= TICKS_PER_SECOND) {
        tickCounter = 0;
        clockSecond++;
        if (clockSecond >= 60) {
            clockSecond = 0;
            clockMinute++;
            if (clockMinute >= 60) {
                clockMinute = 0;
                clockHour = (clockHour + 1) % 24;
            }
        }
    }
}

void updateEscalatorAnimation() {
    escalatorOffset += 0.6;
    if (escalatorOffset > 1000.0) escalatorOffset = 0.0;
}


void updateElevatorAnimation() {
    double floorY    = GROUND_Y + 20.0;
    double platformY = PLATFORM_TOP + PLATFORM_HEIGHT + 15.0;

    switch (elevatorState) {
        case ElevatorState::MOVING: {
            double dy = elevatorGoingUp ? 0.6 : -0.6;
            Point2D moved = translatePoint({0, elevatorY}, 0.0, dy);
            elevatorY = moved.y;

            bool arrivedAtPlatform = elevatorGoingUp  && elevatorY >= platformY;
            bool arrivedAtSurface  = !elevatorGoingUp && elevatorY <= floorY;

            if (arrivedAtPlatform) {
                elevatorY = platformY;
                elevatorState = ElevatorState::DOORS_OPENING;
            } else if (arrivedAtSurface) {
                elevatorY = floorY;
                elevatorState = ElevatorState::DOORS_OPENING;
            }
            break;
        }

        case ElevatorState::DOORS_OPENING:
            elevatorDoorOpenAmount = min(1.0, elevatorDoorOpenAmount + 0.05);
            if (elevatorDoorOpenAmount >= 1.0) {
                elevatorDoorOpenAmount = 1.0;
                elevatorDwellTicks = ELEVATOR_DWELL_TICKS;
                elevatorState = ElevatorState::DOORS_OPEN;
            }
            break;

        case ElevatorState::DOORS_OPEN:
            if (--elevatorDwellTicks <= 0) {
                elevatorState = ElevatorState::DOORS_CLOSING;
            }
            break;

        case ElevatorState::DOORS_CLOSING:
            elevatorDoorOpenAmount = max(0.0, elevatorDoorOpenAmount - 0.05);
            if (elevatorDoorOpenAmount <= 0.0) {
                elevatorDoorOpenAmount = 0.0;
                elevatorGoingUp = !elevatorGoingUp;
                elevatorState   = ElevatorState::MOVING;
            }
            break;
    }
}

//  View
void applySceneProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    switch (cameraView) {
        case 0:
            gluOrtho2D(0,   WINDOW_WIDTH, 0,   WINDOW_HEIGHT);
            break;                 // Full station
        case 1:
            gluOrtho2D(100, 1000,         60,  560);
            break;                 // Platform & train focus
        case 2:
            gluOrtho2D(0,   420,          60,  360);
            break;                 // Signal close-up
        case 3:
            gluOrtho2D(0,   WINDOW_WIDTH, 260, WINDOW_HEIGHT);
            break;                 // Sky & station wide
        default:
            gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
            break;
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void applyDefaultProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'a': case 'A':
            requestTrainArrival();
            break;

        case 'd': case 'D':
            if (trainState == TrainState::STOPPED) {
                trainState = TrainState::DEPARTING;
                announcementText = "TRAIN NOW DEPARTING - MIND THE GAP";
                announcementTimer = 90;
                logEvent("Train departure requested.");
            }
            break;

        case 's': case 'S':
            triggerEmergencyStop();
            break;

        case 'g': case 'G':
            cycleSignalManual();
            break;

        case 'o': case 'O':
            requestDoorsOpen();
            break;

        case 'c': case 'C':
            requestDoorsClose();
            break;

        case 'p': case 'P':
            isPaused = !isPaused;
            logEvent(isPaused ? "Simulation paused." : "Simulation resumed.");
            break;

        case 'l': case 'L':
            eveningPreviewTicks = EVENING_PREVIEW_TICKS;
            break;

        case 'n': case 'N': {
            nextLightMode = static_cast<LightingMode>((static_cast<int>(activeLightMode) + 1) % 3);
            lightTransition = 0.0f;
            string targetName = (nextLightMode == LightingMode::DAY)   ? "DAY" :
                                 (nextLightMode == LightingMode::EVENING) ? "EVENING" : "NIGHT";
            logEvent("Lighting cycling toward " + targetName);
            break;
        }

        case 'm': case 'M':
            autoMode = !autoMode;
            announcementText = autoMode ? "SIGNAL MODE: AUTOMATIC" : "SIGNAL MODE: MANUAL";
            announcementTimer = 80;
            logEvent(autoMode ? "Signal mode: AUTO." : "Signal mode: MANUAL.");
            break;

        case 'f': case 'F':
            isFoggy = !isFoggy;
            logEvent(isFoggy ? "Fog enabled." : "Fog disabled.");
            break;

        case 'r': case 'R':
            rainIntensity = (rainIntensity == RainIntensity::LIGHT) ? RainIntensity::NONE : RainIntensity::LIGHT;
            logEvent("Light rain toggled.");
            break;

        case 't': case 'T':
            rainIntensity = (rainIntensity == RainIntensity::HEAVY) ? RainIntensity::NONE : RainIntensity::HEAVY;
            logEvent("Heavy rain toggled.");
            break;

        case 'v': case 'V':
            cameraView = (cameraView + 1) % NUM_CAMERA_VIEWS;
            logEvent("Camera switched to " + cameraViewLabel());
            break;

        case 27: // Esc
            exit(0);
            break;

        default:
            break;
    }
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    applySceneProjection();

    drawSky();
    drawSunMoon();
    drawShootingStar();
    drawClouds();
    drawBirds();
    drawBackgroundBuildings();
    drawTrees();
    drawRoad();

    drawElevatedBridge();
    drawStaircase();
    drawElevatorShaft();
    drawRailwayTracks();
    drawPlatform();
    drawSafetyLine();
    drawSignalReflectionStrip();
     drawStationLighting();
    drawWaitingArea();
    drawBenches();
    drawTicketCounter();
    drawTicketGates();
    //drawEscalator();
    drawPlatformSignboards();
    drawDigitalWatch();
    drawAnalogClock();
    drawSignalSystem();


    // Passengers
    drawWalkingPassengers();
    drawWaitingPassengers();
    drawQueue();
    drawExitingPassengers();
    drawStationStaff();

    drawPlatformScreenDoors();

    drawTrain();

    drawRain();
    drawWetReflections();
    drawFogOverlay();

    applyDefaultProjection();
    drawHUD();
    drawControlsPanel();
    drawSpeedGauge();

    glutSwapBuffers();
}


void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    applyDefaultProjection();
}

void timerFunc(int value) {
    if (!isPaused) {
        updateTrain();
        updateSignalFromTrainState();
        updatePassengers();
        updateClockTime();
        updateEscalatorAnimation();
        updateElevatorAnimation();
        updateClouds();
        updateRain();
        updateBirds();
        updateLighting();
        updateGates();
        updateShootingStar();

        windStrength = 1.0f + 0.4 * sin(tickCounter * 0.02 + sunMoonAngle);

        sunMoonAngle += 0.002;
        if (sunMoonAngle > 2 * PI) sunMoonAngle -= (double)(2 * PI);
    }

    glutPostRedisplay();
    glutTimerFunc(30, timerFunc, 0);
}

void initGL() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glPointSize(1.6);
    glLineWidth(1.4);

    srand((unsigned) time(nullptr));

    initClouds();
    initRain();
    initPassengers();
    initBirds();

    currentPalette = paletteForMode(activeLightMode);

    reshape(WINDOW_WIDTH, WINDOW_HEIGHT);

    verifyLayoutNonOverlapping();
}

void verifyLayoutNonOverlapping() {
    struct Zone
    {
        const char *name;
        double left, right;
    };
    Zone zones[] = {
        { "Ticket counter", TICKET_X, TICKET_X + TICKET_W },
        { "Waiting area", WAITING_X, WAITING_X + WAITING_W },
        { "Elevator", ELEVATOR_X, ELEVATOR_X + ELEVATOR_W },
        { "Escalator", ESCALATOR_X, ESCALATOR_X + ESCALATOR_STEPS * ESCALATOR_STEPW },
        { "Staircase", STAIRCASE_X, STAIRCASE_X + STAIRCASE_STEPS * STAIRCASE_STEPW },
        { "Elevated bridge", BRIDGE_X0, BRIDGE_X1 }
    };
    const int zoneCount = (int) (sizeof(zones) / sizeof(zones[0]));

    bool anyOverlap = false;
    for (int i = 0; i < zoneCount; ++i) {
        for (int j = i + 1; j < zoneCount; ++j) {
            bool overlap = zones[i].left < zones[j].right && zones[j].left < zones[i].right;
            if (overlap) {
                anyOverlap = true;
                cerr << "[layout warning] " << zones[i].name << " overlaps " << zones[j].name << endl;
            }
        }

        if (zones[i].left < PLATFORM_LEFT || zones[i].right > PLATFORM_RIGHT) {
            cerr << "[layout warning] " << zones[i].name << " extends past the platform edge" << endl;
        }
    }

    if (!anyOverlap) {
        cout << "[layout check] All " << zoneCount
                   << " ground-level infrastructure zones are clear of one another." << endl;
    }
}

void printStartupBanner() {
    cout << "=====================================================\n";
    cout << "   SMART METRO RAIL STATION SIMULATION -- CG LAB\n";
    cout << "=====================================================\n";
    cout << "  A  Train arrives (needs GREEN signal)\n";
    cout << "  D  Train departs\n";
    cout << "  S  Emergency stop (press again to release)\n";
    cout << "  G  Change signal (MANUAL mode only)\n";
    cout << "  O  Open doors            C  Close doors\n";
    cout << "  P  Pause / resume\n";
    cout << "  L  Quick evening-light preview\n";
    cout << "  N  Cycle Day -> Evening -> Night\n";
    cout << "  M  Toggle AUTO / MANUAL signal mode\n";
    cout << "  F  Toggle fog            R  Toggle light rain\n";
    cout << "  V  Cycle camera view     T  Toggle heavy rain\n";
    cout << "  Esc  Exit\n";
    cout << "=====================================================\n";
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(60, 40);
    glutCreateWindow(WINDOW_TITLE);

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(30, timerFunc, 0);

    printStartupBanner();

    glutMainLoop();
    return 0;
}
