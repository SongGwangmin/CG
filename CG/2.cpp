#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>
#include <list>

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

int width;
int height;

std::list<ret> rectlist;
std::list<ret>::iterator rectlist_iter;
int rectcount = 0;

ret morph(ret& after, ret& before) {
	int halfwidth = width / 2;
	int halfheight = height / 2;
    after.x1 = (before.x1 - halfwidth) / halfwidth;
    after.y1 = (before.y1 - halfheight) / -halfheight;
    after.x2 = (before.x2 - halfwidth) / halfwidth;
    after.y2 = (before.y2 - halfheight) / -halfheight;

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
    case GLUT_LEFT_BUTTON:
    {

    }
    break;
    case GLUT_RIGHT_BUTTON:
    {

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
    width = glutGet(GLUT_WINDOW_WIDTH);
    height = glutGet(GLUT_WINDOW_HEIGHT);
    
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

	ret drawsupport;
	drawsupport.x1 = 0;
	drawsupport.y1 = 0;
	drawsupport.x2 = 250;
    drawsupport.y2 = 250;

    glPolygonMode(GL_BACK, GL_FILL);
    ret morphed;
    morph(morphed, drawsupport);
    //glColor3f(0, 0, 0);
    glRectf(morphed.x1, morphed.y1, morphed.x2, morphed.y2);
    
    // �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡���Եȴ�.
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);

}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'q': // ���α׷� ����
        glutLeaveMainLoop();
        break;
    case 'a':
    {

    }
        break;

    default:
        break;
    }

    glutPostRedisplay();
}