//
// Created by  TauFique Hassan on 7/9/26.
//

#include <GLUT/glut.h>
#include <cmath>
#include <iostream>

using namespace std;

float Ax, Ay, Bx, By, Cx, Cy;

// Main Shape
void shape(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    glColor3f(0, 0, 1);

    glBegin(GL_POLYGON);
        glVertex2f(Ax, Ay);
        glVertex2f(Bx, By);
        glVertex2f(Cx, Cy);
    glEnd();
}

void translation(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    float tx = 5, ty = 1;

    glColor3f(1, 0, 0);

    glBegin(GL_POLYGON);
        glVertex2f(Ax + tx, Ay + ty);
        glVertex2f(Bx + tx, By + ty);
        glVertex2f(Cx + tx, Cy + ty);
    glEnd();
}


void rotation(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    float theta = 90;
    float rad = theta * 3.1416 / 180.0;

    glColor3f(1, 0, 0);

    glBegin(GL_POLYGON);
        glVertex2f(Ax * cos(rad) - Ay * sin(rad),
                   Ax * sin(rad) + Ay * cos(rad));

        glVertex2f(Bx * cos(rad) - By * sin(rad),
                   Bx * sin(rad) + By * cos(rad));

        glVertex2f(Cx * cos(rad) - Cy * sin(rad),
                   Cx * sin(rad) + Cy * cos(rad));
    glEnd();
}

void reflection(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    glColor3f(1, 0, 0);

    glBegin(GL_POLYGON);

        // Reflection about X-axis
        // glVertex2f(Ax, -Ay);
        // glVertex2f(Bx, -By);
        // glVertex2f(Cx, -Cy);

        // Reflection about Y-axis
        glVertex2f(-Ax, Ay);
        glVertex2f(-Bx, By);
        glVertex2f(-Cx, Cy);

        // Reflection about Origin
        // glVertex2f(-Ax, -Ay);
        // glVertex2f(-Bx, -By);
        // glVertex2f(-Cx, -Cy);

    glEnd();
}

void shearing(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    float shx = 1;
    float shy = 1;

    glColor3f(1, 0, 0);

    glBegin(GL_POLYGON);

        // X-axis Shearing
        glVertex2f(Ax + shx * Ay, Ay);
        glVertex2f(Bx + shx * By, By);
        glVertex2f(Cx + shx * Cy, Cy);

        // Y-axis Shearing
        /*
        glVertex2f(Ax, Ay + shy * Ax);
        glVertex2f(Bx, By + shy * Bx);
        glVertex2f(Cx, Cy + shy * Cx);
        */

    glEnd();
}

void scaling(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    float sx = 2;
    float sy = 2;

    glColor3f(1, 0, 0);

    glBegin(GL_POLYGON);
        glVertex2f(Ax * sx, Ay * sy);
        glVertex2f(Bx * sx, By * sy);
        glVertex2f(Cx * sx, Cy * sy);
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    shape(Ax, Ay, Bx, By, Cx, Cy);

    //translation(Ax, Ay, Bx, By, Cx, Cy);
    //rotation(Ax, Ay, Bx, By, Cx, Cy);
    //reflection(Ax, Ay, Bx, By, Cx, Cy);
    //shearing(Ax, Ay, Bx, By, Cx, Cy);
    scaling(Ax, Ay, Bx, By, Cx, Cy);

    glFlush();
}

void init()
{
    glClearColor(1, 1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-20, 20, -20, 20);
}

int main(int argc, char **argv)
{
    cout << "Enter the coordinates of the triangle vertices. " << endl;
    cout << "Enter the x & y coordinate of vertex A: ";
    cin >> Ax >> Ay;
    cout << "Enter the x & y coordinate of vertex B: ";
    cin >> Bx >> By;
    cout << "Enter the x & y coordinate of vertex C: ";
    cin >> Cx >> Cy;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(200, 100);
    glutCreateWindow("2D Transformations");

    init();
    glutDisplayFunc(display);

    glutMainLoop();

    return 0;
}