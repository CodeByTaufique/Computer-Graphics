//
// Created by TauFique Hassan on 8/6/26.
//

#include <GL/freeglut.h>

// Rotation angle
float angle = 0.0f;

// Horizontal movement
float moveX = -2.5f;
bool moveRight = true;
float speed = 0.02f;

void init()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    // Camera
    gluLookAt(
            0.0, 0.0, 8.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0
    );

    glPushMatrix();

    glTranslatef(moveX, 0.0f, -3.0f);

    glRotatef(angle, 1.0f, 1.0f, 0.0f);

    glScalef(1.5f, 1.5f, 1.5f);

    glColor3f(0.0f, 0.5f, 1.0f);

    // Draw Cube
    glutSolidCube(1.0);

    glPopMatrix();

    glutSwapBuffers();
}

void animate()
{
    angle += 0.5f;

    if (angle >= 360.0f)
        angle = 0.0f;

    if (moveRight)
    {
        moveX += speed;

        if (moveX >= 2.5f)
            moveRight = false;
    }
    else
    {
        moveX -= speed;

        if (moveX <= -2.5f)
            moveRight = true;
    }

    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(800, 600);

    glutCreateWindow("Lab 12 - 3D Cube Animation");

    init();

    glutDisplayFunc(display);

    glutIdleFunc(animate);

    glutMainLoop();

    return 0;
}