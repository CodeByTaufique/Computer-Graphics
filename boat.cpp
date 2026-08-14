//
// Created by  TauFique Hassan on 5/15/26.
//

#include <GLUT/glut.h>
#include <GLFW/glfw3.h>

#include <iostream>

using namespace std;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POLYGON);
    glColor3f(0.0,1.0,1.0);

    glVertex3f(0.0,0.8,0); //1
    glVertex3f(-0.4,0.0,0); //2
    glVertex3f(0.4,0.0,0); //7
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(1.0,1.0,1.0);

    glVertex3f(-0.8,0.0,0); //3
    glVertex3f(-0.4,-0.8,0); //4
    glVertex3f(0.4,-0.8,0); //5
    glVertex3f(0.8,0.0,0); //6

    glEnd();

    glutSwapBuffers();
}

void init (void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Hello");

    init();

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}