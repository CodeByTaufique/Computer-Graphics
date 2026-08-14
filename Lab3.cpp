//
// Created by  TauFique Hassan on 6/11/26.
//

#include <GLUT/glut.h>
#include <iostream>
#include <cmath>

using namespace std;

void drawPoint(float x, float y)
{
    glBegin(GL_POINTS);
    glVertex2f(x,y);
    glEnd();
}

void DDA(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    float steps = max(abs(dx),abs(dy));

    float x = x1;
    float y = y1;

    float xInc = dx / steps;
    float yInc = dy / steps;

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

    glColor3f(1.0,0.0,0.0);
    glPointSize(5.0);

    glBegin(GL_POINTS);
    DDA(1,1,15,10);

    glFlush();
}

void init(void)
{
    glClearColor(1.0,1.0,1.0,1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,20,0,20);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("DDA");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
}