//
// Created by  TauFique Hassan on 6/18/26.
//

#include <GLUT/glut.h>
#include <iostream>
#include <cmath>

using namespace std;

float a1,a2,b1,b2;
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

    if (dy <= dx)
    {
        int p = 2 * dy - dx;
        while (x <= x2)
        {
            drawPoint(x,y);
            if (p < 0)
            {
                p += 2 * dy;
            } else
            {
                y++;
                p += 2 * (dy - dx);
            }
            x++;
        }
    } else
    {
        int p = 2 * dy - dx;
        while (y <= y2)
        {
            drawPoint(x,y);
            if (p < 0)
            {
                p += 2 * dx;
            } else
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
    glClear (GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 0.0, 0.0);

    glPointSize(3.0);

    Bresenham(a1,b1,a2,b2);
    glFlush();
}

void init(void)
{
    glClearColor(1.0,1.0,1.0,1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,100,0,100);
}

int main(int argc,char **argv)
{
    cout << "Enter value of x1,y1 : ";
    cin >> a1 >> b1;

    cout << "Enter value of x2,y2 : ";
    cin >> a2 >> b2;

    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Bresenham");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
}