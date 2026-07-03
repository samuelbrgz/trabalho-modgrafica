#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ===================== VARIÁVEIS GLOBAIS =====================

// Animação
float angle = 0.0;

// Sol
float sunX = 0.0, sunY = 1.0, sunZ = 0.0;

int animationEnabled = 1; // ligar/desligar a animação
int collisionDetected = 0; // atualizar isso quando implementar colisão

// Janela / Projeção
int projectionMode = 0; // 0 = perspectiva, 1 = ortográfica
int windowWidth = 600, windowHeight = 600;

// Câmera - posição e direção
float camX = 0.0, camY = 0.0, camZ = 5.0;
float yaw = -90.0, pitch = 0.0;
float dirX = 0.0, dirY = 0.0, dirZ = -1.0;
float camSpeed = 0.1;

// Câmera - pulo
float camVelY = 0.0;
int isJumping = 0;
float gravity = 0.01;
float jumpForce = 0.2;
float groundLevel = 0.0;

// Mouse
int lastMouseX = 300, lastMouseY = 300;
int firstMouse = 1;
float sensitivity = 0.3;

// Iluminação
int lightEnabled = 1;
GLfloat lightPos[] = { 0.0, 1.0, 0.0, 1.0 };

// ===================== HUD ========================
void drawHUD() {
    // Salva o estado da luz e desliga (texto não deve reagir à luz)
    glDisable(GL_LIGHTING);

    // Muda pra projeção ortográfica 2D correspondente aos pixels da tela
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Cor do texto
    glColor3f(1.0, 1.0, 1.0);

    char buffer[100];

    // Posição da câmera
    sprintf(buffer, "Posicao: X=%.1f Y=%.1f Z=%.1f", camX, camY, camZ);
    glRasterPos2i(10, windowHeight - 20);
    for (int i = 0; buffer[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    // Tipo de projeção
    sprintf(buffer, "Projecao: %s", projectionMode == 0 ? "Perspectiva" : "Ortografica");
    glRasterPos2i(10, windowHeight - 45);
    for (int i = 0; buffer[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    // Estado da animação
    sprintf(buffer, "Animacao: %s", animationEnabled ? "Ligada" : "Desligada");
    glRasterPos2i(10, windowHeight - 70);
    for (int i = 0; buffer[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    // Estado da iluminação
    sprintf(buffer, "Iluminacao: %s", lightEnabled ? "Ligada" : "Desligada");
    glRasterPos2i(10, windowHeight - 95);
    for (int i = 0; buffer[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    // Indicação de colisão
    sprintf(buffer, "Colisao: %s", collisionDetected ? "SIM" : "Nao");
    glRasterPos2i(10, windowHeight - 120);
    for (int i = 0; buffer[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    // Desfaz a projeção 2D, volta pro estado 3D normal
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // Restaura iluminação se estava ligada
    if (lightEnabled) {
        glEnable(GL_LIGHTING);
    }
}

// ===================== CÂMERA =====================

void updateCameraDirection() {
    dirX = cos(yaw * 3.14159 / 180.0) * cos(pitch * 3.14159 / 180.0);
    dirY = sin(pitch * 3.14159 / 180.0);
    dirZ = sin(yaw * 3.14159 / 180.0) * cos(pitch * 3.14159 / 180.0);
}

void setupCamera() {
    if (projectionMode == 1) {
        // Ortográfica: vista diagonal fixa (isométrica)
        gluLookAt(20.0, 15.0, 20.0,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);
    } else {
        // Perspectiva: câmera FPS livre
        gluLookAt(camX, camY, camZ,
                  camX + dirX, camY + dirY, camZ + dirZ,
                  0.0, 1.0, 0.0);
    }
}

// ===================== PROJEÇÃO =====================

void setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)windowWidth / (float)windowHeight;

    if (projectionMode == 0) {
        gluPerspective(75.0, aspect, 1.0, 500.0);
    } else {
        float size = 12.0;
        glOrtho(-size * aspect, size * aspect, -size, size, 1.0, 100.0);
    }

    glMatrixMode(GL_MODELVIEW);
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    setupProjection();
}

// ===================== ILUMINAÇÃO =====================

void setupLighting() {
    GLfloat ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    if (lightEnabled) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    } else {
        glDisable(GL_LIGHTING);
    }
}

// ===================== CENA / OBJETOS =====================

void drawGrid() {
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(1.0);

    glBegin(GL_LINES);
        for (int i = -25; i <= 25; i++) {
            glVertex3f((float)i, -0.94, -25.0);
            glVertex3f((float)i, -0.94, 25.0);
            glVertex3f(-25.0, -0.94, (float)i);
            glVertex3f(25.0, -0.94, (float)i);
        }
    glEnd();
}

void drawFloor() {
    GLfloat matAmbient[]  = { 0.4, 0.1, 0.5, 1.0 };
    GLfloat matDiffuse[]  = { 0.4, 0.1, 0.5, 1.0 };
    GLfloat matSpecular[] = { 0.1, 0.1, 0.1, 1.0 };

    glPushMatrix();
        glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, 10.0);

        glTranslatef(0.0, -1.0, 0.0);
        glScalef(50.0, 0.1, 50.0);
        glutSolidCube(1.0);
    glPopMatrix();
}

void drawObject() {
    // Planeta 1
    glPushMatrix();
        GLfloat mat1Amb[] = { 0.6, 0.2, 0.1, 1.0 };
        GLfloat mat1Dif[] = { 0.8, 0.3, 0.1, 1.0 };
        GLfloat mat1Spec[] = { 0.3, 0.3, 0.3, 1.0 };
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat1Amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat1Dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat1Spec);
        glMaterialf(GL_FRONT, GL_SHININESS, 20.0);

        float dist1 = 5.0, ang1 = 0.0;
        glTranslatef(dist1 * cos(ang1 * 3.14159/180.0), 1.0, dist1 * sin(ang1 * 3.14159/180.0));
        glutSolidSphere(1.0, 30, 30);
    glPopMatrix();

    // Planeta 2
    glPushMatrix();
        GLfloat mat2Amb[] = { 0.1, 0.2, 0.6, 1.0 };
        GLfloat mat2Dif[] = { 0.2, 0.4, 0.9, 1.0 };
        GLfloat mat2Spec[] = { 0.5, 0.5, 0.5, 1.0 };
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat2Amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat2Dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat2Spec);
        glMaterialf(GL_FRONT, GL_SHININESS, 40.0);

        float dist2 = 9.0, ang2 = 120.0;
        glTranslatef(dist2 * cos(ang2 * 3.14159/180.0), 1.0, dist2 * sin(ang2 * 3.14159/180.0));
        glutSolidSphere(1.5, 30, 30);
    glPopMatrix();

    // Planeta 3
    glPushMatrix();
        GLfloat mat3Amb[] = { 0.1, 0.4, 0.1, 1.0 };
        GLfloat mat3Dif[] = { 0.2, 0.7, 0.3, 1.0 };
        GLfloat mat3Spec[] = { 0.2, 0.2, 0.2, 1.0 };
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat3Amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat3Dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat3Spec);
        glMaterialf(GL_FRONT, GL_SHININESS, 15.0);

        float dist3 = 13.0, ang3 = 240.0;
        glTranslatef(dist3 * cos(ang3 * 3.14159/180.0), 1.0, dist3 * sin(ang3 * 3.14159/180.0));
        glutSolidSphere(0.8, 30, 30);
    glPopMatrix();

    // Sol
    glPushMatrix();
        GLfloat sunAmb[]      = { 1.0, 1.0, 0.3, 1.0 };
        GLfloat sunDif[]      = { 1.0, 1.0, 0.3, 1.0 };
        GLfloat sunSpec[]     = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat sunEmission[] = { 0.8, 0.8, 0.2, 1.0 };
        glMaterialfv(GL_FRONT, GL_AMBIENT, sunAmb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, sunDif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, sunSpec);
        glMaterialfv(GL_FRONT, GL_EMISSION, sunEmission);
        glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

        glTranslatef(sunX, sunY, sunZ);
        glutSolidSphere(2.0, 30, 30);
    glPopMatrix();

    // Reseta emissão pro padrão (evita que objetos seguintes "herdem" o brilho do sol)
    GLfloat noEmission[] = { 0.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
}

void drawScene() {
    drawFloor();
    drawGrid();
    drawObject();
}

// ===================== CALLBACKS DE ENTRADA =====================

void keyboard(unsigned char key, int x, int y) {
    if (key == 'w' || key == 'W') {
        camX += cos(yaw * 3.14159 / 180.0) * camSpeed;
        camZ += sin(yaw * 3.14159 / 180.0) * camSpeed;
    }
    if (key == 's' || key == 'S') {
        camX -= cos(yaw * 3.14159 / 180.0) * camSpeed;
        camZ -= sin(yaw * 3.14159 / 180.0) * camSpeed;
    }
    if (key == 'a' || key == 'A') {
        camX -= dirZ * camSpeed;
        camZ += dirX * camSpeed;
    }
    if (key == 'd' || key == 'D') {
        camX += dirZ * camSpeed;
        camZ -= dirX * camSpeed;
    }
    if (key == 'r' || key == 'R') {
        camX = 0.0; camY = 0.0; camZ = 5.0;
        yaw = -90.0; pitch = 0.0;
        updateCameraDirection();
    }
    if (key == 'p' || key == 'P') {
        projectionMode = 1 - projectionMode;
        setupProjection();
    }
    if (key == 'l' || key == 'L') {
        lightEnabled = !lightEnabled;
        setupLighting();
    }
    if (key == ' ') {
        if (!isJumping) {
            camVelY = jumpForce;
            isJumping = 1;
        }
    }
    if (key == 27) {
        exit(0);
    }

    glutPostRedisplay();
}

void special(int key, int x, int y) {
    float turnSpeed = 2.0;

    if (key == GLUT_KEY_LEFT)  yaw -= turnSpeed;
    if (key == GLUT_KEY_RIGHT) yaw += turnSpeed;
    if (key == GLUT_KEY_UP) {
        pitch += turnSpeed;
        if (pitch > 89.0) pitch = 89.0;
    }
    if (key == GLUT_KEY_DOWN) {
        pitch -= turnSpeed;
        if (pitch < -89.0) pitch = -89.0;
    }

    updateCameraDirection();
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        printf("Clique esquerdo em (%d, %d)\n", x, y);
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        printf("Clique direito em (%d, %d)\n", x, y);
    }
}

void mouseMotion(int x, int y) {
    if (firstMouse) {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = 0;
        return;
    }

    float offsetX = x - lastMouseX;
    float offsetY = lastMouseY - y;

    lastMouseX = x;
    lastMouseY = y;

    yaw += offsetX * sensitivity;
    pitch += offsetY * sensitivity;

    if (pitch > 89.0) pitch = 89.0;
    if (pitch < -89.0) pitch = -89.0;

    updateCameraDirection();
    glutPostRedisplay();
}

// ===================== TEMPO / ANIMAÇÃO =====================

void timer(int value) {
    angle += 1.0;

    if (isJumping) {
        camY += camVelY;
        camVelY -= gravity;

        if (camY <= groundLevel) {
            camY = groundLevel;
            camVelY = 0.0;
            isJumping = 0;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ===================== DISPLAY PRINCIPAL =====================

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    setupCamera();

    lightPos[0] = sunX;
    lightPos[1] = sunY;
    lightPos[2] = sunZ;
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    drawScene();
    drawHUD();

    glutSwapBuffers();
}

// ===================== INICIALIZAÇÃO =====================

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    setupProjection();
    setupLighting();
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Trabalho Modelagem Grafica");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutSpecialFunc(special);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouseMotion);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();

    return 0;
}