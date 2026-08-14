//
// Created by  TauFique Hassan on 5/14/26.
//

// First Lab

#include <GLUT/glut.h>
#include <GLFW/glfw3.h>

#include <iostream>

using namespace std;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    /* glColor3f(0.7,0.45,1.0);
    glBegin(GL_POLYGON);

    glVertex3f(0.30,0.25,0);
    glVertex3f(0.40,0.20,0); // 1
    glVertex3f(0.40,0.60,0); // 2

    glVertex3f(0.30,0.55,0);
    glVertex3f(0.20,0.60,0); // 3
    glVertex3f(0.20,0.20,0); // 4
   */

    /*
    glBegin(GL_TRIANGLES);
    glColor3f(0.2,0.5,0.73);

    glVertex3f(-0.6,0.75,0); // 1
    glVertex3f(-0.6,0.25,0); // 2
    glVertex3f(-0.3,0.50,0); // 3

    glVertex3f(-0.3,0.50,0); // 4
    glVertex3f(0.0,0.75,0); // 5
    glVertex3f(0.0,0.25,0); // 6
    */

    glBegin(GL_POLYGON);
    glColor3f(0.7,0.45,1.0);

    glVertex3f(0.4,0.8,0); //1
    glVertex3f(0.0,0.6,0); //2
    glVertex3f(0.8,0.6,0); //8
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.4,0.25,0.7);

    glVertex3f(0.2,0.6,0); //3
    glVertex3f(0.2,0.3,0);

    glVertex3f(0.6,0.3,0);
    glVertex3f(0.6,0.3,0);
    glVertex3f(0.6,0.6,0); //7

    glEnd();
    glutSwapBuffers();
}

void init (void)
{
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


