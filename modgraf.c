#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ===================== VARIÁVEIS GLOBAIS =====================

// Animação
float angle = 0.0;
float sunAngle = 0.0;   // Ângulo para o Sol girar em si mesmo
float cubeAngle = 0.0;  // Ângulo para o nosso objeto novo

// Sol
float sunX = 0.0, sunY = 1.0, sunZ = 0.0;

int animationEnabled = 1; // ligar/desligar a animação
int collisionDetected = 0; // atualizar isso quando implementar colisão

// Janela / Projeção
int projectionMode = 0; // 0 = perspectiva, 1 = ortográfica
int windowWidth = 1280, windowHeight = 720;

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
// Variáveis para prender o mouse
int centerX, centerY;
int ignoreNextMouse = 0; // Trava para evitar o loop infinito
int paused = 0;

// Iluminação
int lightEnabled = 1;
GLfloat lightPos[] = { 0.0, 1.0, 0.0, 1.0 };

// ===================== CÉU ESTRELADO =====================
#define NUM_STARS 1000
float starsX[NUM_STARS];
float starsY[NUM_STARS];
float starsZ[NUM_STARS];

void initSky() {
    for(int i = 0; i < NUM_STARS; i++) {
        float r = 400.0;
        

        float theta = (rand() % 360) * 3.14159 / 180.0;
        float phi = ((rand() % 180) - 90) * 3.14159 / 180.0;
        
        // Converte polar para cartesiano
        starsX[i] = r * cos(phi) * cos(theta);
        starsY[i] = r * sin(phi);
        starsZ[i] = r * cos(phi) * sin(theta);
    }
}

void drawSky() {
    glDisable(GL_LIGHTING);
    

    glColor3f(0.8, 0.9, 1.0);
    glPointSize(1.5);
    
    glBegin(GL_POINTS);
    for(int i = 0; i < NUM_STARS; i++) {
        glVertex3f(starsX[i], starsY[i], starsZ[i]);
    }
    glEnd();
    
    if (lightEnabled) {
        glEnable(GL_LIGHTING);
    }
}

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
    if (collisionDetected) {
        glColor3f(1.0, 0.0, 0.0);
    } else {
        glColor3f(1.0, 1.0, 1.0);
    }

    sprintf(buffer, "Colisao: %s", collisionDetected ? "SIM" : "Nao");
    glRasterPos2i(10, windowHeight - 120);
    for (int i = 0; buffer[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    glColor3f(1.0, 1.0, 1.0);

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
    centerX = w / 2;
    centerY = h / 2;
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

    // Planeta Marte
    glPushMatrix();
        GLfloat mat1Amb[]  = { 0.3, 0.1, 0.05, 1.0 }; // Sombra: Marrom/Ferrugem bem escuro
        GLfloat mat1Dif[]  = { 0.7, 0.3, 0.15, 1.0 }; // Cor principal: Vermelho terracota
        GLfloat mat1Spec[] = { 0.1, 0.1, 0.1, 1.0 };  // Reflexo: Muito fraco (superfície fosca)
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat1Amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat1Dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat1Spec);
        glMaterialf(GL_FRONT, GL_SHININESS, 5.0);

        float dist1 = 5.0, ang1 = 0.0;
        glTranslatef(dist1 * cos(ang1 * 3.14159/180.0), 1.0, dist1 * sin(ang1 * 3.14159/180.0));
        glutSolidSphere(1.0, 30, 30);
    glPopMatrix();

    // Planeta Saturno
    glPushMatrix();
        float dist2 = 9.0, ang2 = 120.0;
        glTranslatef(dist2 * cos(ang2 * 3.14159/180.0), 1.0, dist2 * sin(ang2 * 3.14159/180.0));
        

        glPushMatrix();
            GLfloat satAmb[]  = { 0.2, 0.2, 0.1, 1.0 }; // Sombra amarronzada
            GLfloat satDif[]  = { 0.8, 0.7, 0.4, 1.0 }; // Cor de areia/bege
            GLfloat satSpec[] = { 0.1, 0.1, 0.1, 1.0 }; // Gás não tem muito reflexo
            glMaterialfv(GL_FRONT, GL_AMBIENT, satAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, satDif);
            glMaterialfv(GL_FRONT, GL_SPECULAR, satSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
            glutSolidSphere(1.5, 30, 30);
        glPopMatrix();

        glPushMatrix();
            GLfloat ringAmb[] = { 0.3, 0.3, 0.2, 1.0 };
            GLfloat ringDif[] = { 0.9, 0.8, 0.6, 1.0 };
            glMaterialfv(GL_FRONT, GL_AMBIENT, ringAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, ringDif);
            glMaterialfv(GL_FRONT, GL_SPECULAR, satSpec);

            glRotatef(angle * 2.0, 0.0, 1.0, 0.0); 
            glRotatef(20.0 * sin(angle * 0.05), 1.0, 0.0, 0.0); 
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glScalef(1.0, 1.0, 0.02); 

            glutSolidTorus(0.2, 2.2, 15, 50);
        glPopMatrix();

    glPopMatrix();

    // Planeta Urano
    glPushMatrix();
        GLfloat mat3Amb[]  = { 0.1, 0.2, 0.3, 1.0 }; // Sombra: Azul profundo/escuro
        GLfloat mat3Dif[]  = { 0.4, 0.7, 0.9, 1.0 }; // Cor principal: Azul pálido/ciano claro
        GLfloat mat3Spec[] = { 0.3, 0.4, 0.5, 1.0 }; // Reflexo: Suave e levemente azulado
        
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat3Amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat3Dif);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat3Spec);
        

        glMaterialf(GL_FRONT, GL_SHININESS, 30.0);

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
        glRotatef(sunAngle, 0.0, 1.0, 0.0);
        glutSolidSphere(2.0, 30, 30);
    glPopMatrix();


    GLfloat noEmission[] = { 0.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);

    // Planeta Terra + Lua
    glPushMatrix();

        glTranslatef(15.0 * cos(cubeAngle * 0.005), 3.0, 15.0 * sin(cubeAngle * 0.005));
        //Planeta
        glPushMatrix();
            GLfloat cubeAmb[]  = { 0.0, 0.3, 0.3, 1.0 }; // Sombra: Ciano escuro
            GLfloat cubeDif[]  = { 0.1, 0.7, 0.8, 1.0 }; // Cor principal: Azul água vibrante
            GLfloat cubeSpec[] = { 0.8, 1.0, 1.0, 1.0 }; // Reflexo: Quase branco, levemente azulado
            glMaterialfv(GL_FRONT, GL_AMBIENT, cubeAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, cubeDif);
            glMaterialfv(GL_FRONT, GL_SPECULAR, cubeSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, 90.0);



            glRotatef(cubeAngle, 0.0, 1.0, 0.0);
            glRotatef(cubeAngle * 1.5, 1.0, 0.0, 0.0);
            
            glutSolidCube(2.0);

            GLfloat contAmb[]  = { 0.0, 0.3, 0.0, 1.0 }; 
            GLfloat contDif[]  = { 0.1, 0.8, 0.1, 1.0 }; 
            GLfloat contSpec[] = { 0.1, 0.1, 0.1, 1.0 };
            glMaterialfv(GL_FRONT, GL_AMBIENT, contAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, contDif);
            glMaterialfv(GL_FRONT, GL_SPECULAR, contSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, 10.0);

            //Continentes
            glBegin(GL_QUADS);
                glNormal3f(0.0, 0.0, 1.0);
                glVertex3f(-0.6, -0.3, 1.01);
                glVertex3f( 0.5, -0.3, 1.01);
                glVertex3f( 0.5,  0.7, 1.01);
                glVertex3f(-0.6,  0.7, 1.01);

                glNormal3f(0.0, 1.0, 0.0);
                glVertex3f(-0.8, 1.01, -0.4);
                glVertex3f(-0.8, 1.01,  0.7);
                glVertex3f( 0.3, 1.01,  0.7);
                glVertex3f( 0.3, 1.01, -0.4);

                glNormal3f(1.0, 0.0, 0.0);
                glVertex3f(1.01, -0.5, -0.5);
                glVertex3f(1.01, -0.5,  0.5);
                glVertex3f(1.01,  0.5,  0.5);
                glVertex3f(1.01,  0.5, -0.5);

            glEnd();
        glPopMatrix();

        //Lua
        glPushMatrix();
            GLfloat ballAmb[] = { 0.2, 0.2, 0.2, 1.0 };  // Sombra: Cinza bem escuro
            GLfloat ballDif[] = { 0.6, 0.6, 0.6, 1.0 };  // Cor principal: Cinza médio/claro
            GLfloat ballSpec[] = { 0.1, 0.1, 0.1, 1.0 }; // Reflexo: Muito fraco (superfície fosca)
            glMaterialfv(GL_FRONT, GL_AMBIENT, ballAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, ballDif);
            glMaterialfv(GL_FRONT, GL_SPECULAR, ballSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, 5.0);

            glTranslatef(2.0 * cos(cubeAngle * 0.03), 0.0, 2.0 * sin(cubeAngle * 0.03));

            glutSolidSphere(0.2, 30, 30);
        glPopMatrix();

    glPopMatrix();

    // ===================== FOGUETE (3º Objeto Composto) =====================
    glPushMatrix();
        // 1. Órbita e Animação: Flutuando e "quicando" suavemente ao redor do Sol
        // Usamos sin() no eixo Y para ele subir e descer enquanto orbita
        glTranslatef(8.0 * cos(cubeAngle * 0.02), 5.0 + 1.5 * sin(cubeAngle * 0.05), 8.0 * sin(cubeAngle * 0.02));

        // 2. Aponta o foguete para o céu (O cilindro/cone nascem no eixo Z)
        glRotatef(-90.0, 1.0, 0.0, 0.0);
        
        // 3. Faz o foguete girar no próprio eixo (efeito de estabilização)
        glRotatef(angle * 5.0, 0.0, 0.0, 1.0); 

        // ----------------- CORPO (Cilindro Branco) -----------------
        glPushMatrix();
            GLfloat rocketAmb[]  = { 0.7, 0.7, 0.7, 1.0 };
            GLfloat rocketDif[]  = { 0.9, 0.9, 0.9, 1.0 };
            GLfloat rocketSpec[] = { 1.0, 1.0, 1.0, 1.0 };
            glMaterialfv(GL_FRONT, GL_AMBIENT, rocketAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, rocketDif);
            glMaterialfv(GL_FRONT, GL_SPECULAR, rocketSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, 80.0);
            
            glutSolidCylinder(0.4, 1.5, 30, 30);
        glPopMatrix();

        // ----------------- BICO (Cone Vermelho) -----------------
        glPushMatrix();
            GLfloat tipAmb[] = { 0.4, 0.0, 0.0, 1.0 }; 
            GLfloat tipDif[] = { 0.9, 0.1, 0.1, 1.0 };
            glMaterialfv(GL_FRONT, GL_AMBIENT, tipAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, tipDif);
            // Reutiliza o reflexo do corpo
            glMaterialfv(GL_FRONT, GL_SPECULAR, rocketSpec); 
            glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
            
            glTranslatef(0.0, 0.0, 1.5);
            glutSolidCone(0.4, 0.8, 30, 30);
        glPopMatrix();

        // ----------------- MOTOR (Cone Escuro Invertido) -----------------
        glPushMatrix();
            GLfloat engineAmb[] = { 0.1, 0.1, 0.1, 1.0 }; 
            GLfloat engineDif[] = { 0.2, 0.2, 0.2, 1.0 };
            glMaterialfv(GL_FRONT, GL_AMBIENT, engineAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, engineDif);
            glMaterialf(GL_FRONT, GL_SHININESS, 10.0); // Fosco
            
            glRotatef(180.0, 1.0, 0.0, 0.0);
            glutSolidCone(0.25, 0.4, 30, 30);
        glPopMatrix();

    glPopMatrix(); // Fim do Foguete

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
    if (key == 'd' || key == 'D') {
        camX -= dirZ * camSpeed;
        camZ += dirX * camSpeed;
    }
    if (key == 'a' || key == 'A') {
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
    if (key == 't' || key == 'T') {
        animationEnabled = !animationEnabled;
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
        glutSetCursor(GLUT_CURSOR_INHERIT); // Destravar MOUSE
        paused = 1;
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        printf("Clique direito em (%d, %d)\n", x, y);
        glutSetCursor(GLUT_CURSOR_NONE); // Travar MOUSE
        glutWarpPointer(windowWidth / 2, windowHeight / 2);
        paused = 0;
    }
}


void mouseMotion(int x, int y) {
    if(!paused){
        if (ignoreNextMouse) {
            ignoreNextMouse = 0;
            return;
        }

        float offsetX = x - centerX;
        float offsetY = centerY - y;

        yaw += offsetX * sensitivity;
        pitch += offsetY * sensitivity;

        if (pitch > 89.0) pitch = 89.0;
        if (pitch < -89.0) pitch = -89.0;


        updateCameraDirection();
        glutPostRedisplay();

        ignoreNextMouse = 1;
        glutWarpPointer(centerX, centerY);
    }
}

// ===================== TEMPO / ANIMAÇÃO =====================

void timer(int value) {
    // Animações Pulo e Planetas
    if(animationEnabled){
        angle += 1.0;
        sunAngle += 0.2;
        cubeAngle += 2.0;

        if (isJumping) {
            camY += camVelY;
            camVelY -= gravity;

            if (camY <= groundLevel) {
                camY = groundLevel;
                camVelY = 0.0;
                isJumping = 0;
            }
        }
    }

    //Colisão com o Sol
    float dist = sqrt(pow((camX - sunX), 2) + pow((camY - sunY), 2) + pow((camZ - sunZ), 2));

    if(dist <= 2.8) {
        collisionDetected = 1;
        float nx = (camX - sunX) / dist;
        float nz = (camZ - sunZ) / dist;

        camX = sunX + nx * 2.8;
        camZ = sunZ + nz * 2.8;
    }
    else {
        collisionDetected = 0;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
    
}

// ===================== DISPLAY PRINCIPAL =====================

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    setupCamera();

    lightPos[0] = sunX; lightPos[1] = sunY; lightPos[2] = sunZ;
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    drawSky();
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
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Trabalho Modelagem Grafica");

    init();
    initSky();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutSpecialFunc(special);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouseMotion);
    glutTimerFunc(0, timer, 0);

    // Config Mouse Capturado
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(windowWidth / 2, windowHeight / 2);

    glutMainLoop();

    return 0;
}