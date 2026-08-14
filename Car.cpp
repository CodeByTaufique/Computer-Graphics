//
// Created by  TauFique Hassan on 7/22/26.
//

//
// Created by TauFique Hassan on 7/22/26.
// Simple Colorful Car using Bresenham Line Algorithm
// and Midpoint Circle Algorithm
//

#include <GLUT/glut.h>
#include <cmath>

using namespace std;

//---------------- Plot Pixel ----------------//
void plot(int x, int y)
{
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

//---------------- Bresenham Line Algorithm ----------------//
void Bresenham(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
        plot(x1, y1);

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

//---------------- Midpoint Circle Algorithm ----------------//
void circlePoints(int xc, int yc, int x, int y)
{
    plot(xc + x, yc + y);
    plot(xc - x, yc + y);
    plot(xc + x, yc - y);
    plot(xc - x, yc - y);
    plot(xc + y, yc + x);
    plot(xc - y, yc + x);
    plot(xc + y, yc - x);
    plot(xc - y, yc - x);
}

void MidPointCircle(int xc, int yc, int r)
{
    int x = 0;
    int y = r;
    int p = 1 - r;

    while (x <= y)
    {
        circlePoints(xc, yc, x, y);

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
        int x = (int)sqrt(r * r - y * y);
        Bresenham(xc - x, yc + y, xc + x, yc + y);
    }
}

void drawCar()
{
    //  Body
    glColor3f(0.9, 0.1, 0.1);

    for (int y = 150; y <= 220; y++)
        Bresenham(100, y, 300, y);

    //  Roof
    glColor3f(1.0, 1.0, 1.0);

    Bresenham(140,220,180,270);
    Bresenham(180,270,240,270);
    Bresenham(240,270,280,220);

    // ===== Window =====
    glColor3f(1.0,1.0,0.0);

    for(int y=221; y<=269; y++)
        Bresenham(181,y,239,y);

    //  Door
    glColor3f(1.0,0.0,0.0);
    Bresenham(200,150,200,220);

    //  Wheels
    glColor3f(0.05,0.05,0.05);
    filledCircle(150,130,22);
    filledCircle(250,130,22);

    //  Rims
    glColor3f(0.8,0.8,0.8);
    filledCircle(150,130,10);
    filledCircle(250,130,10);

    glColor3f(0.0,0.0,0.0);

    // Body
    Bresenham(100,150,300,150);
    Bresenham(300,150,300,220);
    Bresenham(300,220,100,220);
    Bresenham(100,220,100,150);

    // Roof
    Bresenham(140,220,180,270);
    Bresenham(180,270,240,270);
    Bresenham(240,270,280,220);

    // Window
    Bresenham(180,220,180,270);
    Bresenham(180,270,240,270);
    Bresenham(240,270,240,220);
    Bresenham(180,220,240,220);

    // Door
    Bresenham(200,150,200,220);

    // Wheels
    MidPointCircle(150,130,22);
    MidPointCircle(250,130,22);

    MidPointCircle(150,130,10);
    MidPointCircle(250,130,10);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glPointSize(2);

    drawCar();

    glFlush();
}

void init()
{
    glClearColor(0.7,0.9,1.0,1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0,400,0,400);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600,600);
    glutCreateWindow("Car using Bresenham Line & Midpoint Circle");

    init();

    glutDisplayFunc(display);

    glutMainLoop();

    return 0;
}