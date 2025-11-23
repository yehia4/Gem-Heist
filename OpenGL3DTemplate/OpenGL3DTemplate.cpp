#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <glut.h>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
// --- Input & Movement Settings ---
#define MOVE_SPEED 0.5
#define MOUSE_SENSITIVITY 0.1

// Window center for mouse locking
int windowWidth = 1000;
int windowHeight = 600;
// Function Prototypes
void Display();
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Idle();

// --- Global Variables for Player State ---
// Player Position (X, Y, Z) - Starting at (0, 1, 5)
double playerX = 0.0;
double playerY = 1.0; // Head height
double playerZ = 5.0;

// Camera Orientation (Yaw = rotation Y, Pitch = rotation X)
double cameraYaw = 0.0;
double cameraPitch = 0.0;

// Camera View Mode: false = First Person, true = Third Person
bool isThirdPerson = false;

// Third Person Camera Settings (from proposal: "45-degree angle, about 5 units away")
double cameraDistance = 5.0;
double cameraHeightOffset = 1.5;


// Mouse Motion Handler (Looking around)
void MouseMotion(int x, int y) {
    // Calculate the offset from the center of the window
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    int dx = x - centerX;
    int dy = y - centerY;

    // Update angles based on mouse movement
    cameraYaw += dx * MOUSE_SENSITIVITY;
    cameraPitch -= dy * MOUSE_SENSITIVITY;

    // Clamp Pitch to prevent flipping upside down (stops at looking straight up/down)
    if (cameraPitch > 89.0) cameraPitch = 89.0;
    if (cameraPitch < -89.0) cameraPitch = -89.0;

    // Re-center the mouse to allow continuous rotation
    // (Note: This might be annoying during debugging, but is standard for games)
    if (dx != 0 || dy != 0) {
        glutWarpPointer(centerX, centerY);
    }

    glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
    double radianYaw = DEG2RAD(cameraYaw);

    switch (key) {
    case GLUT_KEY_ESCAPE:
        exit(EXIT_SUCCESS);
        break;

        // --- Movement Keys (WASD) ---
    case 'w': case 'W': // Move Forward
        playerX += sin(radianYaw) * MOVE_SPEED;
        playerZ -= cos(radianYaw) * MOVE_SPEED;
        break;
    case 's': case 'S': // Move Backward
        playerX -= sin(radianYaw) * MOVE_SPEED;
        playerZ += cos(radianYaw) * MOVE_SPEED;
        break;
    case 'd': case 'D': // Strafe Right (Yaw + 90 degrees)
        playerX += cos(radianYaw) * MOVE_SPEED;
        playerZ += sin(radianYaw) * MOVE_SPEED;
        break;
    case 'a': case 'A': // Strafe Left (Yaw - 90 degrees)
        playerX -= cos(radianYaw) * MOVE_SPEED;
        playerZ -= sin(radianYaw) * MOVE_SPEED;
        break;

        // --- Camera Toggle (Q) ---
    case 'q': case 'Q':
        isThirdPerson = !isThirdPerson; // Toggle the boolean
        break;

        // --- Jump (Space) Placeholder ---
    case ' ':
        // Logic for jumping will go here later
        break;
    }

    glutPostRedisplay();
}

// Main Initialization
void main(int argc, char** argv) {
	glutInit(&argc, argv);

	// Double buffering for smooth animation, RGB color, Depth buffer for 3D
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(1000, 600);
	glutInitWindowPosition(150, 150);

	// Project Title from GEM.pdf
	glutCreateWindow("GEM Heist");

	// Register Callbacks
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutIdleFunc(Idle);
    glutPassiveMotionFunc(MouseMotion); // Register mouse move without click
    glutSetCursor(GLUT_CURSOR_NONE);

	// Basic OpenGL Settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	// Set clear color to a dark night sky (dark blue-gray)
	glClearColor(0.1f, 0.1f, 0.15f, 0.0f);

	glutMainLoop();
}


// Render the Scene
void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // --- Camera Math ---
    double lookX = playerX + sin(DEG2RAD(cameraYaw)) * cos(DEG2RAD(cameraPitch));
    double lookY = playerY + sin(DEG2RAD(cameraPitch));
    double lookZ = playerZ - cos(DEG2RAD(cameraYaw)) * cos(DEG2RAD(cameraPitch));

    double eyeX, eyeY, eyeZ;

    if (isThirdPerson) {
        eyeX = playerX - sin(DEG2RAD(cameraYaw)) * cameraDistance;
        eyeY = playerY + cameraHeightOffset;
        eyeZ = playerZ + cos(DEG2RAD(cameraYaw)) * cameraDistance;
        lookX = playerX; lookY = playerY; lookZ = playerZ;
    }
    else {
        eyeX = playerX; eyeY = playerY; eyeZ = playerZ;
    }

    gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);

    // --- NEW: Ceiling Light ---
    // We define this AFTER gluLookAt so it is placed in the "World"
    // Position: (0, 20, 0) = High above center. 1.0 = Point Light
    GLfloat lightPos[] = { 0.0f, 20.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // --- Draw Floor ---
    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f); // Made slightly lighter for visibility
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f); // Normal facing UP
    glVertex3f(-50.0f, 0.0f, -50.0f);
    glVertex3f(-50.0f, 0.0f, 50.0f);
    glVertex3f(50.0f, 0.0f, 50.0f);
    glVertex3f(50.0f, 0.0f, -50.0f);
    glEnd();
    glPopMatrix();

    // --- Draw Player (Third Person) ---
    if (isThirdPerson) {
        glPushMatrix();
        glTranslatef(playerX, playerY / 2.0, playerZ);
        glColor3f(0.0f, 0.0f, 0.8f);
        glScalef(0.5f, 1.0f, 0.5f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    glutSwapBuffers();
}

// Window Resize Handling
void Reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


// Idle Function for Animation
void Idle() {
	glutPostRedisplay();
}