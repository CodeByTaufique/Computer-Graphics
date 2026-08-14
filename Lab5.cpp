//
// Created by  TauFique Hassan on 7/9/26.
//

#include <GLUT/glut.h>
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

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.0,1.0,0.0);
    glPointSize(3.0);
    drawCircle(xc, yc, r);
    glFlush();
}

void init(void)
{
    glClearColor(1.0,1.0,1.0,1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-100,100,-100,100);
}

int main(int argc,char **argv)
{
    cout << "Enter center (xc yc): ";
    cin >> xc >> yc;
    cout << "Enter radius: ";
    cin >> r;
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Circle Drawing Algorithm");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}