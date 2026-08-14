//
// Created by  TauFique Hassan on 7/22/26.
//

#include <GLUT/glut.h>
#include <cmath>

using namespace std;

const double PI = 3.14159265358979323846;

void drawPoint(int x, int y)
{
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void Bresenham(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int x = x1;
    int y = y1;

    if (dy <= dx) // m <= 1
    {
        int p = 2 * dy - dx;

        while (x <= x2)
        {
            drawPoint(x, y);

            if (p < 0)
            {
                p += 2 * dy;
            }
            else
            {
                y++;
                p += 2 * (dy - dx);
            }

            x++;
        }
    }
    else // m > 1
    {
        int p = 2 * dx - dy;

        while (y <= y2)
        {
            drawPoint(x, y);

            if (p < 0)
            {
                p += 2 * dx;
            }
            else
            {
                x++;
                p += 2 * (dx - dy);
            }

            y++;
        }
    }
}

void drawCirclePoints(int xc, int yc, int x, int y)
{
    drawPoint(xc + x, yc + y);
    drawPoint(xc - x, yc + y);
    drawPoint(xc + x, yc - y);
    drawPoint(xc - x, yc - y);
    drawPoint(xc + y, yc + x);
    drawPoint(xc - y, yc + x);
    drawPoint(xc + y, yc - x);
    drawPoint(xc - y, yc - x);
}

void MidPointCircle(int xc, int yc, int r)
{
    int x = 0;
    int y = r;
    int p = 1 - r;

    while (x <= y)
    {
        drawCirclePoints(xc, yc, x, y);

        if (p < 0)
            p += 2 * x + 3;
        else
        {
            p += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void filledCircle(int xc, int yc, int r)
{
    for (int y = -r; y <= r; y++)
    {
        int xLimit = (int)round(sqrt((double)(r * r - y * y)));
        Bresenham(xc - xLimit, yc + y, xc + xLimit, yc + y);
    }
}


void drawFlower()
{
    int cx = 200;        // flower center
    int cy = 200;
    int petalR = 65;      // petal circle radius
    int petalDist = 78;   // distance from center to each petal's center
    int numPetals = 8;
    int centerR = 55;

    float petalColors[4][3] = {
        {1.0, 0.55, 0.75},
        {1.0, 0.75, 0.35},
        {0.65, 0.55, 1.0},
        {1.0, 0.9, 0.35}
    };

    //  Filled petals
    for (int i = 0; i < numPetals; i++)
    {
        double angle = i * (360.0 / numPetals) * PI / 180.0;
        int px = cx + (int)round(petalDist * cos(angle));
        int py = cy + (int)round(petalDist * sin(angle));

        float* col = petalColors[i % 4];
        glColor3f(col[0], col[1], col[2]);
        filledCircle(px, py, petalR);
    }

    //  Petal outlines
    glColor3f(0.0, 0.0, 0.0);
    for (int i = 0; i < numPetals; i++)
    {
        double angle = i * (360.0 / numPetals) * PI / 180.0;
        int px = cx + (int)round(petalDist * cos(angle));
        int py = cy + (int)round(petalDist * sin(angle));

        MidPointCircle(px, py, petalR);
    }

    //  Center Circle
    glColor3f(1.0f, 1.0f, 1.0f);
    filledCircle(cx, cy, centerR);

    glColor3f(0.0f, 0.0f, 0.0f);
    MidPointCircle(cx, cy, centerR);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(1.5f);
    drawFlower();
    glFlush();
}

void init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 400, 0, 400);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Colorful Flower - Bresenham and Midpoint Circle");

    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}