//
// Created by  TauFique Hassan on 6/10/26.
//

#include <GLUT/glut.h>
#include <iostream>
#include <cmath>

using namespace std;

void showLine(int x1, int y1, int x2, int y2)
{
    glBegin(GL_LINES);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glEnd();
}

void drawPoint(int x, int y)
{
    glBegin(GL_POINTS);
    glVertex2i(x,y);
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

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0,0.0,0.0);
    showLine(0,0,6,6);

    glPointSize(5.0);
    glColor3f(1.0,0.0,0.0);

    Bresenham(0,0,6,6);

    glFlush();
}

void init(void)
{
    glClearColor(1.0,1.0,1.0,1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,10,0,10);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Bresenham");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
}