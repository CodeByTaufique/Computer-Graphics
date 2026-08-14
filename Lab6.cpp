//
// Created by  TauFique Hassan on 7/23/26.
//

#include <GLUT/glut.h>
#include <cmath>
#include <iostream>

using namespace std;

float Ax,Ay,Bx,By,Cx,Cy;
float tx = 5, ty = 3;

void shape (float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    glColor3f(1,1,0);

    glBegin(GL_POLYGON);
    glVertex2f(Ax,Ay);
    glVertex2f(Bx,By);
    glVertex2f(Cx,Cy);
    glEnd();
}

void translation(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{

    glColor3f(1, 0, 0);

    glBegin(GL_POLYGON);
    glVertex2f(Ax + tx, Ay + ty);
    glVertex2f(Bx + tx, By + ty);
    glVertex2f(Cx + tx, Cy + ty);
    glEnd();
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 'w' || key == 'W')
        ty += 5;
    else if (key == 's' || key == 'S')
        ty -= 5;
    else if (key == 'a' || key == 'A')
        tx -= 5;
    else if (key == 'd' || key == 'D')
        tx += 5;

    glutPostRedisplay();
}

void scaling(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    float sx,sy;
    cout << "Enter Sx Value : ";
    cin >> sx;

    cout << "Enter Sy Value : ";
    cin >> sy;

    glColor3f(0, 1, 0);

    glBegin(GL_POLYGON);
    glVertex2f(Ax * sx, Ay * sy);
    glVertex2f(Bx * sx, By * sy);
    glVertex2f(Cx * sx, Cy * sy);
    glEnd();
}

void reflection(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    glColor3f(1, 0, 0);

    glBegin(GL_POLYGON);

    // Reflection about X-axis
        glVertex2f(Ax, -Ay);
        glVertex2f(Bx, -By);
        glVertex2f(Cx, -Cy);

    // Reflection about Y-axis
        //glVertex2f(-Ax, Ay);
        //glVertex2f(-Bx, By);
        //glVertex2f(-Cx, Cy);

    // Reflection about Origin
        //glVertex2f(-Ax, -Ay);
        //glVertex2f(-Bx, -By);
        //glVertex2f(-Cx, -Cy);

    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    shape(Ax, Ay, Bx, By, Cx, Cy);
    //translation(Ax, Ay, Bx, By, Cx, Cy);
    reflection(Ax, Ay, Bx, By, Cx, Cy);
    //scaling(Ax, Ay, Bx, By, Cx, Cy);

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
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}