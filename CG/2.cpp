#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>

std::random_device rd;

// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::mt19937 gen(rd());

// 0 ���� 99 ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_int_distribution<int> dis(0, 256);

typedef struct RET {
    GLdouble x1, y1, x2, y2;
    GLdouble Rvalue = 0.0;
    GLdouble Gvalue = 0.0;
    GLdouble Bvalue = 0.0;
    int level = 3;
} ret;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);

bool timerRunning = false;

GLdouble Rvalue = 0.0;
GLdouble Gvalue = 0.0;
GLdouble Bvalue = 1.0;

ret morph(ret& after, ret& before) {
    after.x1 = (before.x1 - 250) / 250;
    after.y1 = (before.y1 - 250) / -250;
    after.x2 = (before.x2 - 250) / 250;
    after.y2 = (before.y2 - 250) / -250;

    //(showingrect[0].x1 - 250) / 250, (showingrect[0].y1 - 250) / -250

    // ������ �������� ���� ����
    after.Rvalue = before.Rvalue;
    after.Gvalue = before.Gvalue;
    after.Bvalue = before.Bvalue;

    return after;
}

static ret hidingrect[4] = {
    {  0,   0, 250, 250 }, // ���� �Ʒ�
    {250,   0, 500, 250 }, // ������ �Ʒ�
    {  0, 250, 250, 500 }, // ���� ��
    {250, 250, 500, 500 }  // ������ ��
};

static ret showingrect[4] = {
    {  0,   0, 250, 250 },
    {250,   0, 500, 250 },
    {  0, 250, 250, 500 },
    {250, 250, 500, 500 }
};

bool ptinrect(int x, int y, ret& rect) {
    return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}

void Mouse(int button, int state, int x, int y)
{
    switch (button) {
    case GLUT_LEFT_BUTTON: {
        if (state == GLUT_DOWN) { // left click - in: change rectcolor, out: change bgcolor
            for (int i = 0; i < 4; i++) {
                if (ptinrect(x, y, showingrect[i])) {
                    showingrect[i].Rvalue = dis(gen) / 256.0f;
                    showingrect[i].Gvalue = dis(gen) / 256.0f;
                    showingrect[i].Bvalue = dis(gen) / 256.0f;
                    glutPostRedisplay();
                    break;
                }
                else if (ptinrect(x, y, hidingrect[i])) {
                    GLdouble Rv = dis(gen) / 256.0f;
                    GLdouble Gv = dis(gen) / 256.0f;
                    GLdouble Bv = dis(gen) / 256.0f;

                    for (int i = 0; i < 4; i++) {
                        hidingrect[i].Rvalue = Rv;
                        hidingrect[i].Gvalue = Gv;
                        hidingrect[i].Bvalue = Bv;
                    }
                    glutPostRedisplay();
                    break;
                }
            }
        }
    }
                         break;
    case GLUT_RIGHT_BUTTON: {
        if (state == GLUT_DOWN) {
            for (int i = 0; i < 4; i++) {
                if (ptinrect(x, y, showingrect[i])) {
                    if (showingrect[i].level > 0) {
                        showingrect[i].level--;
                        showingrect[i].x1 += 25;
                        showingrect[i].y1 += 25;
                        showingrect[i].x2 -= 25;
                        showingrect[i].y2 -= 25;
                    }
                    glutPostRedisplay();
                    break;
                }
                else if (ptinrect(x, y, hidingrect[i])) {
                    if (showingrect[i].level < 3) {
                        showingrect[i].level++;
                        showingrect[i].x1 -= 25;
                        showingrect[i].y1 -= 25;
                        showingrect[i].x2 += 25;
                        showingrect[i].y2 += 25;
                    }

                    glutPostRedisplay();
                    break;
                }
            }
        }
    }
                          break;
    default:
        break;

    }
}

// Ÿ�̸� �ݹ� (glutTimerFunc ��� ȣ��� �Լ� ����)
void TimerFunc(int value) {
    if (!timerRunning) return;

    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        glutPostRedisplay();
        glutTimerFunc(1000, TimerFunc, 0); // 1�ʸ��� ����
    }
}


void main(int argc, char** argv)
{
    //--- ������ �����ϱ�
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Example1");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "GLEW Initialized\n";
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutMainLoop();
}

GLvoid drawScene()
{
    glClearColor(Rvalue, Gvalue, Bvalue, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);


    static int start = 0;



    if (start == 0) {
        for (int i = 0; i < 4; i++) {
            hidingrect[i].Rvalue = 0.5;
            hidingrect[i].Gvalue = 0.5;
            hidingrect[i].Bvalue = 0.5;


            showingrect[i].Rvalue = dis(gen) / 256.0f;
            showingrect[i].Gvalue = dis(gen) / 256.0f;
            showingrect[i].Bvalue = dis(gen) / 256.0f;
        }
        start = 1;
    }

    for (int i{}; i < 4; ++i) {
        ret drawsupport;
        drawsupport = morph(drawsupport, hidingrect[i]);
        glColor3f(drawsupport.Rvalue, drawsupport.Gvalue, drawsupport.Bvalue);
        glRectf(drawsupport.x1, drawsupport.y1, drawsupport.x2, drawsupport.y2);

        drawsupport = morph(drawsupport, showingrect[i]);
        glColor3f(drawsupport.Rvalue, drawsupport.Gvalue, drawsupport.Bvalue);
        glRectf(drawsupport.x1, drawsupport.y1, drawsupport.x2, drawsupport.y2);
    }
    // �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡���Եȴ�.
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    //--- �ʿ��� ������� include
    //--- ������ ����ϰ� �ݹ��Լ� ����
   // glut �ʱ�ȭ
   // ���÷��� ��� ����
   // �������� ��ġ ����
   // �������� ũ�� ����
   // ������ ���� (������ �̸�)
    // glew �ʱ�ȭ
   // ��� �Լ��� ����
   // �ٽ� �׸��� �Լ� ����
   // �̺�Ʈ ó�� ����
   //--- �ݹ� �Լ�: ��� �ݹ� �Լ�
   // �������� ��blue�� �� ����
   // ������ ������ ��ü��ĥ�ϱ�
   // ȭ�鿡 ����ϱ�
   //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c': // û�ϻ� (�ʷ�+�Ķ�)
        timerRunning = false;
        Rvalue = 0.0;
        Gvalue = 1.0;
        Bvalue = 1.0;
        break;

    case 'm': // ��ȫ�� (����+�Ķ�)
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 0.0;
        Bvalue = 1.0;
        break;

    case 'y': // ����� (����+�ʷ�)
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 1.0;
        Bvalue = 0.0;
        break;

    case 'a': // ������
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        timerRunning = false;
        break;

    case 'w': // ���
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 1.0;
        Bvalue = 1.0;
        break;

    case 'k': // ������
        timerRunning = false;
        Rvalue = 0.0;
        Gvalue = 0.0;
        Bvalue = 0.0;
        break;

    case 't': // Ÿ�̸� ����
        if (!timerRunning) {
            timerRunning = true;
            glutTimerFunc(100, TimerFunc, 0); // 1�ʸ��� ���� �� ����
        }
        break;

    case 's': // Ÿ�̸� ����
        timerRunning = false;
        break;

    case 'q': // ���α׷� ����
        glutLeaveMainLoop();
        break;

    default:
        break;
    }

    glutPostRedisplay();
}