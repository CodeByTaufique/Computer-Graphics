//
// Created by  TauFique Hassan on 7/30/26.
//

#include <GLUT/glut.h>
#include <cmath>
#include <iostream>

#define PI 3.1416

using namespace std;
double Ax,Ay,Bx,By,Cx,Cy;
double theta = 0;
void shape (double Ax, double Ay, double Bx, double By, double Cx, double Cy)
{
    glColor3f(1,1,0);

    glBegin(GL_LINE_LOOP);
        glVertex2d(Ax,Ay);
        glVertex2d(Bx,By);
        glVertex2d(Cx,Cy);
    glEnd();
}

void shearing (double Ax, double Ay, double Bx, double By, double Cx, double Cy)
{
    int choice;
    double Shx,Shy;
    cout << "1. X-axis Shearing" << endl;
    cout << "2. Y-axis Shearing" << endl;
    cout << "Enter your choice: ";
    cin >> choice;

    if (choice == 1)
    {
        cout << "Enter Shx Value : ";
        cin >> Shx;

        glColor3f(1,0,0);

        glBegin(GL_LINE_LOOP);
            glVertex2d(Ax,Ay);
            glVertex2d(Bx*Shx,By);
            glVertex2d(Cx*Shx,Cy);
        glEnd();
    } else if (choice == 2)
    {
        cout << "Enter Shy Value : ";
        cin >> Shy;

        glColor3f(0,0,1);

        glBegin(GL_LINE_LOOP);
        glVertex2d(Ax,Ay);
        glVertex2d(Bx,By*Shy);
        glVertex2d(Cx,Cy*Shy);
        glEnd();
    } else
    {
        cout << "Invalid Choice" << endl;
    }
}

void rotation (double Ax, double Ay, double Bx, double By, double Cx, double Cy)
{
    double rad = theta * PI / 180.0;

    glColor3f(1,0,1);
    glBegin(GL_LINE_LOOP);

    glVertex2f(Ax * cos(rad) - Ay * sin(rad),
               Ax * sin(rad) + Ay * cos(rad));
    glVertex2f(Bx * cos(rad) - By * sin(rad),
               Bx * sin(rad) + By * cos(rad));
    glVertex2f(Cx * cos(rad) - Cy * sin(rad),
               Cx * sin(rad) + Cy * cos(rad));
    glEnd();
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 'a' || key == 'A')
        theta += 15;
    else if (key == 'd' || key == 'D')
        theta -= 15;
    else if (key == 'r' || key == 'R')
        theta = 0;

    glutPostRedisplay();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    shape(Ax, Ay, Bx, By, Cx, Cy);

    rotation(Ax, Ay, Bx, By, Cx, Cy);
    //shearing(Ax, Ay, Bx, By, Cx, Cy);

    glFlush();
}

void init()
{
    glClearColor(1, 1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-100, 100, -100, 100);
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
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}