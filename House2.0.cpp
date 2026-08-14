//
// Created by  TauFique Hassan on 7/23/26.
//

#include <GLUT/glut.h>
#include <iostream>

using namespace std;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Roof
    glBegin(GL_POLYGON);
    glColor3f(1.0, 0.0, 0.0);

    glVertex3f(0.0, 0.8, 0);
    glVertex3f(-0.8, 0.2, 0);
    glVertex3f(0.8, 0.2, 0);

    glEnd();

    // House body
    glBegin(GL_POLYGON);
    glColor3f(1.0, 1.0, 0.0);

    glVertex3f(-0.5, -0.8, 0);
    glVertex3f(0.5, -0.8, 0);
    glVertex3f(0.5, 0.2, 0);
    glVertex3f(-0.5, 0.2, 0);

    glEnd();

    // Door
    glBegin(GL_POLYGON);
    glColor3f(0.4, 0.2, 0.0);

    glVertex3f(-0.15, -0.8, 0);
    glVertex3f(0.15, -0.8, 0);
    glVertex3f(0.15, -0.3, 0);
    glVertex3f(-0.15, -0.3, 0);

    glEnd();

    // Window 1
    glBegin(GL_POLYGON);
    glColor3f(0.6, 0.85, 1.0);

    glVertex3f(0.2, -0.1, 0);
    glVertex3f(0.4, -0.1, 0);
    glVertex3f(0.4, 0.1, 0);
    glVertex3f(0.2, 0.1, 0);

    glEnd();

    // Window 1 frame
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(2.0);

    glBegin(GL_LINES);

    glVertex3f(0.2, -0.1, 0);
    glVertex3f(0.4, -0.1, 0);

    glVertex3f(0.4, -0.1, 0);
    glVertex3f(0.4, 0.1, 0);

    glVertex3f(0.4, 0.1, 0);
    glVertex3f(0.2, 0.1, 0);

    glVertex3f(0.2, 0.1, 0);
    glVertex3f(0.2, -0.1, 0);

    glVertex3f(0.3, -0.1, 0);
    glVertex3f(0.3, 0.1, 0);

    glVertex3f(0.2, 0.0, 0);
    glVertex3f(0.4, 0.0, 0);
    glEnd();

    // Window 2
    glBegin(GL_POLYGON);
    glColor3f(0.6, 0.85, 1.0);

    glVertex3f(-0.4, -0.1, 0);
    glVertex3f(-0.2, -0.1, 0);
    glVertex3f(-0.2, 0.1, 0);
    glVertex3f(-0.4, 0.1, 0);

    glEnd();

    // Window 2 frame
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(2.0);

    glBegin(GL_LINES);
    glVertex3f(-0.4, -0.1, 0);
    glVertex3f(-0.2, -0.1, 0);

    glVertex3f(-0.2, -0.1, 0);
    glVertex3f(-0.2, 0.1, 0);

    glVertex3f(-0.2, 0.1, 0);
    glVertex3f(-0.4, 0.1, 0);

    glVertex3f(-0.4, 0.1, 0);
    glVertex3f(-0.4, -0.1, 0);

    glVertex3f(-0.3, -0.1, 0);
    glVertex3f(-0.3, 0.1, 0);

    glVertex3f(-0.4, 0.0, 0);
    glVertex3f(-0.2, 0.0, 0);
    glEnd();

    glutSwapBuffers();
}

void init(void)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Hello");

    init();

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}