//
// Created by  TauFique Hassan on 5/21/26.
//

#include <GLUT/glut.h>

#include <iostream>

using namespace std;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0,1.0,1.0);

    /*
    glBegin(GL_POLYGON);

    glVertex3f(0.6,0.6,0);  //8
    glVertex3f(0.5,1.0,0); //1
    glVertex3f(0.4,0.6,0);  //2

    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(0.4,0.6,0);  //2
    glVertex3f(0.0,0.5,0);  //3
    glVertex3f(0.4,0.4,0);  //4
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(0.4,0.4,0);
    glVertex3f(0.5,-0.0,0);
    glVertex3f(0.6,0.4,0);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(0.6,0.4,0);  //6
    glVertex3f(1.0,0.5,0);  //7
    glVertex3f(0.6,0.6,0);  //8
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(0.6,0.6,0);
    glVertex3f(0.4,0.6,0);
    glVertex3f(0.4,0.4,0);
    glVertex3f(0.6,0.4,0);
    glEnd();

    */

    // First Star
    glBegin(GL_POLYGON);
    glColor3f(1.0,0.0,0.0);

    glVertex3f(-0.4,0.6,0);
    glVertex3f(-0.5,1,0);
    glVertex3f(-0.6,0.6,0);
    glVertex3f(-1,0.5,0);
    glVertex3f(-0.6,0.4,0);
    glVertex3f(-0.5,0,0);
    glVertex3f(-0.4,0.4,0);
    glVertex3f(0,0.5,0);

    glEnd();

    // 2nd Star
    glBegin(GL_POLYGON);
    glColor3f(0.0,1.0,0.0);

    glVertex3f(-0.4,-0.4,0);
    glVertex3f(-0.5,0,0);
    glVertex3f(-0.6,-0.6,0);
    glVertex3f(-1,-0.5,0);
    glVertex3f(-0.6,-0.6,0);
    glVertex3f(-0.5,-1,0);
    glVertex3f(-0.4,-0.6,0);
    glVertex3f(0,-0.5,0);

    glEnd();

    // 3rd Star
    glBegin(GL_POLYGON);
    glColor3f(0.0,0.0,1.0);

    glVertex3f(0.6,-0.4,0);
    glVertex3f(0.5,0,0);
    glVertex3f(0.4,-0.4,0);
    glVertex3f(0,-0.5,0);
    glVertex3f(0.4,-0.6,0);
    glVertex3f(0.5,-1,0);
    glVertex3f(0.6,-0.6,0);
    glVertex3f(1,-0.5,0);

    glEnd();

    // 4th Star
    glBegin(GL_POLYGON);
    glColor3f(1.0,1.1,1.0);

    glVertex3f(0.4,0.6,0);
    glVertex3f(0.5,1,0);
    glVertex3f(0.6,0.6,0);
    glVertex3f(1,0.5,0);
    glVertex3f(0.6,0.4,0);
    glVertex3f(0.5,0,0);
    glVertex3f(0.4,0.4,0);
    glVertex3f(0,0.5,0);

    glEnd();

    glutSwapBuffers();
}

void init (void)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Hello");

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}