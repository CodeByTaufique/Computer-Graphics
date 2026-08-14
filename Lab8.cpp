//
// Created by TauFique Hassan on 8/6/26.
//

#include <GLUT/glut.h>
#include <iostream>

using namespace std;

double x = -100;
double y = -100;
double speed = 1;

int choice;

void triangle()
{
    if (choice == 1)
    {
        glColor3f(1, 1, 0);

        glBegin(GL_TRIANGLES);

        glVertex2f(x, 0);
        glVertex2f(x + 20, 0);
        glVertex2f(x + 10, 30);

        glEnd();
    }
    else if (choice == 2)
    {
        glColor3f(0, 0, 1);

        glBegin(GL_TRIANGLES);

        glVertex2f(0, y);
        glVertex2f(-20, y + 20);
        glVertex2f(20, y + 20);

        glEnd();
    }
    else if (choice == 3)
    {
        glColor3f(1, 0, 0);

        glBegin(GL_TRIANGLES);

        glVertex2f(x, 0);
        glVertex2f(x + 20, 0);
        glVertex2f(x + 10, 30);

        glEnd();
    }
}

void update(int value)
{
    if (choice == 1)
    {
        x += 1;

        if (x > 100)
            x = -100;
    }
    else if (choice == 2)
    {
        y += 1;

        if (y > 100)
            y = -100;
    }
    else if (choice == 3)
    {
        x += speed;

        if (x >= 80 || x <= -100)
            speed = -speed;
    }

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
    glClearColor(1, 1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-100, 100, -100, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char **argv)
{
    cout << "2D Animation" << endl;
    cout << "1. X-axis animation" << endl;
    cout << "2. Y-axis animation" << endl;
    cout << "3. Bouncing Effect" << endl;
    cout << "Enter Choice: ";
    cin >> choice;

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(200, 100);

    glutCreateWindow("2D Animation");

    init();

    glutDisplayFunc(display);

    glutTimerFunc(0, update, 0);

    glutMainLoop();

    return 0;
}