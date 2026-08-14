//
// Created by  TauFique Hassan on 7/9/26.
//

#include <GLUT/glut.h>
#include <cmath>

#include <iostream>

using namespace std;

float xc, yc, r;

void drawPoint (float x, float y)
{
    glBegin(GL_POINTS);
    glVertex2f(x,y);
    glEnd();
}

void circlePoints (float xc, float yc, float x, float y)
{
    drawPoint(xc + x, yc + y);
    drawPoint(xc - x, yc + y);
    drawPoint(xc + x, yc -y);
    drawPoint(xc - x, yc - y);

    drawPoint(xc + y, yc + x);
    drawPoint(xc - y, yc + x);
    drawPoint(xc + y, yc - x);
    drawPoint(xc - y, yc - x);
}

void drawCircle (float xc, float yc, float r)
{
    float x = 0;
    float y = r;
    float p = 1 - r;

    while (x <= y)
    {
        circlePoints(xc,yc,x,y);
        if (p < 0)
        {
            p = p + 2 * x + 1;
        } else
        {
            p = p + 2 * (x - y) + 1;
            y--;
        }
        x++;
    }
}

void drawFilledCircle(float cx, float cy, float r)
{
    glBegin(GL_POLYGON);

    for (int i = 0; i < 360; i++)
    {
        float theta = i * 3.1416 / 180.0;
        float x = r * cos(theta);
        float y = r * sin(theta);

        glVertex2f(cx + x, cy + y);
    }

    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Green background
    glColor3f(0.0, 0.42, 0.25);
    glBegin(GL_POLYGON);
    glVertex2f(-0.9, 0.5);
    glVertex2f(-0.9, -0.5);
    glVertex2f(0.9, -0.5);
    glVertex2f(0.9, 0.5);
    glEnd();

    // Circle
    glColor3f(1.0, 0.0, 0.0);
    drawFilledCircle(-0.12, 0.0, 0.28);

    glFlush();
}

void init(void)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(700, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Bangladesh Flag");

    init();

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}


