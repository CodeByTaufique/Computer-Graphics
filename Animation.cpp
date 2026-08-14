//
// Created by  TauFique Hassan on 8/6/26.
//
#include <GLUT/glut.h>
#include <iostream>

using namespace std;

double x = -100;
double y = -100;
double speed = 1;

void triangle()
{
    glColor3f(1, 1, 0);

    glBegin(GL_TRIANGLES);

    glVertex2f(x, 0);
    glVertex2f(x + 20, 0);
    glVertex2f(x + 10, 30);

    glEnd();
}

void triangle2()
{
    glColor3f(0, 0, 1);

    glBegin(GL_TRIANGLES);

    glVertex2f(0, y);
    glVertex2f(-20, y + 20);
    glVertex2f(20, y + 20);

    glEnd();
}

void update2(int value)
{
    y += 1;

    if (y > 100)
        y = -100;

    glutPostRedisplay();
    glutTimerFunc(16, update2, 0);
}

void update(int value)
{
    x += 1;

    if (x > 100)
        x = -100;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);

}

void update3(int value) // Bounce
{
    x += speed;

    if (x >= 80 || x <= -100)
        speed = -speed;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    triangle();

    glFlush();
}


void init()
{
    glClearColor(1, 1, 1,1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-100, 100, -100, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(200, 100);

    glutCreateWindow("2D Translation Animation");

    init();

    glutDisplayFunc(display);

    glutTimerFunc(0, update, 0);

    glutMainLoop();

    return 0;
}


/*
 *
*float x = -100;

void car()
{
// Body
glColor3f(0, 0, 1);

glBegin(GL_QUADS);

glVertex2f(x, -20);
glVertex2f(x + 50, -20);
glVertex2f(x + 50, 0);
glVertex2f(x, 0);

glEnd();

// Roof
glColor3f(1, 0, 0);

glBegin(GL_QUADS);

glVertex2f(x + 10, 0);
glVertex2f(x + 20, 15);
glVertex2f(x + 40, 15);
glVertex2f(x + 45, 0);

glEnd();

// Wheels
glColor3f(0, 0, 0);

glBegin(GL_POINTS);

glVertex2f(x + 12, -20);
glVertex2f(x + 38, -20);

glEnd();
}

void update(int value)
{
x += 1;

if (x > 100)
x = -100;

glutPostRedisplay();
glutTimerFunc(16, update, 0);
}

*/