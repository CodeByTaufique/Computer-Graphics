//
// Created by  TauFique Hassan on 6/18/26.
//

#include <GLUT/glut.h>
#include <iostream>

using namespace std;

int xc, yc, r;

void drawPoint(int x, int y)
{
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void plotCirclePoints(int xc, int yc, int x, int y)
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

void MidpointCircle(int xc, int yc, int r)
{
    int x = 0;
    int y = r;

    int p = 1 - r;

    while (x <= y)
    {
        plotCirclePoints(xc, yc, x, y);

        if (p < 0)
        {
            p = p + 2 * x + 3;
        }
        else
        {
            p = p + 2 * (x - y) + 5;
            y--;
        }

        x++;
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0, 0.0, 0.0);
    glPointSize(3.0);

    MidpointCircle(xc, yc, r);

    glFlush();
}

void init()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-100, 100, -100, 100);
}

int main(int argc, char** argv)
{
    cout << "Enter center (xc yc): ";
    cin >> xc >> yc;

    cout << "Enter radius: ";
    cin >> r;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Midpoint Circle Drawing Algorithm");

    init();
    glutDisplayFunc(display);

    glutMainLoop();

    return 0;
}