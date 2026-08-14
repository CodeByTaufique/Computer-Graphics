//
// Created by  TauFique Hassan on 5/18/26.
//

#include <GLUT/glut.h>

void display() {

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_TRIANGLE_FAN);
    // center point
    glVertex2f( 0.00f,  0.00f);

    // spike 1 - top
    glVertex2f( 0.00f,  0.45f);
    glVertex2f( 0.10f,  0.15f);
    // spike 2 - upper right
    glVertex2f( 0.43f,  0.14f);
    glVertex2f( 0.16f, -0.06f);
    // spike 3 - lower right
    glVertex2f( 0.26f, -0.40f);
    glVertex2f( 0.00f, -0.20f);
    // spike 4 - lower left
    glVertex2f(-0.26f, -0.40f);
    glVertex2f(-0.16f, -0.06f);
    // spike 5 - upper left
    glVertex2f(-0.43f,  0.14f);
    glVertex2f(-0.10f,  0.15f);

    // close back to spike 1 tip
    glVertex2f( 0.00f,  0.45f);
    glEnd();

    // ── 4-spike star (GL_TRIANGLE_FAN) ──
   /* glColor3f(0.06f, 0.43f, 0.34f);
    glBegin(GL_TRIANGLE_FAN);
    // center point
    glVertex2f( 0.55f,  0.00f);

    // spike 1 - top
    glVertex2f( 0.55f,  0.45f);
    glVertex2f( 0.68f,  0.13f);
    // spike 2 - right
    glVertex2f( 1.00f,  0.00f);
    glVertex2f( 0.68f, -0.13f);
    // spike 3 - bottom
    glVertex2f( 0.55f, -0.45f);
    glVertex2f( 0.42f, -0.13f);
    // spike 4 - left
    glVertex2f( 0.10f,  0.00f);
    glVertex2f( 0.42f,  0.13f);

    // close back to spike 1 tip
    glVertex2f( 0.55f,  0.45f);
    glEnd(); */

    /*  glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0, 1.0, 1.0);

    glVertex3f(0.0,0.0,0);
    glVertex3f(0.0,0.8,0);
    glVertex3f(0.2,0.3,0);
    glVertex3f(0.8,0.2,0.0);
    glVertex3f(0.4,-0.1,0.0);
    glVertex3f(0.6,-0.8,0.0);
    glVertex3f(0.0,-0.4,0);
    glVertex3f(-0.6,-0.8,0.0);
    glVertex3f(-0.4,-0.1,0.0);
    glVertex3f(-0.8,0.2,0.0);
    glVertex3f(-0.2,0.3,0);
    glVertex3f(0.0,0.8,0);

    glEnd(); */

    glutSwapBuffers();
}

void init(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(900,800);
    glutInitWindowPosition(0,0);
    glutCreateWindow("Stars");
    init();
    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}