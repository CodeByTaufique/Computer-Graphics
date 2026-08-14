//
// Created by TauFique Hassan on 6/11/26.
//

#include <GLUT/glut.h>
#include <iostream>
#include <cmath>

float xx1, yy1, xx2, yy2;

using namespace std;

void showLine(int x1, int y1, int x2, int y2)
{
    glBegin(GL_LINES);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glEnd();
}

void drawPoint(float x, float y)
{
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

void DDA(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    float x = x1;
    float y = y1;

    float xInc = (float)dx / steps;
    float yInc = (float)dy / steps;

    for (int i = 0; i <= steps; i++)
    {
        drawPoint(x,y);
        x += xInc;
        y += yInc;
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0, 0.0, 0.0);
    //showLine(xx1, yy1, xx2, yy2);

    glColor3f(1.0, 0.0, 0.0);
    glPointSize(5.0);

    DDA(xx1, yy1, xx2, yy2);
    glFlush();
}

void init(void)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 10, 0, 10);
}

int main(int argc, char** argv)
{
    cout << "Enter Coordinates of x1,y1 : ";
    cin >> xx1 >> yy1;
    cout << "Enter Coordinates of x2,y2 : " ;
    cin >> xx2 >> yy2;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("DDA");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
}