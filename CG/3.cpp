#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>
#include <algorithm>

#define rectspace 30

std::random_device rd;

// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::mt19937 gen(rd());

static int mousexstart, mouseystart;
static int mousexend, mouseyend;

// 0 부터 99 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_int_distribution<int> dis(0, 256);
std::uniform_int_distribution<int> numdis(0, 500 - rectspace);

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

GLdouble Rvalue = 0.2;
GLdouble Gvalue = 0.2;
GLdouble Bvalue = 0.2;

int width;
int height;

int rectcount = 0;

ret morph(ret& after, ret& before) {
    int halfwidth = width / 2;
    int halfheight = height / 2;
    after.x1 = (before.x1 - halfwidth) / halfwidth;
    after.y1 = (before.y1 - halfheight) / -halfheight;
    after.x2 = (before.x2 - halfwidth) / halfwidth;
    after.y2 = (before.y2 - halfheight) / -halfheight;

    //(showingrect[0].x1 - 250) / 250, (showingrect[0].y1 - 250) / -250

    // 색상은 랜덤으로 새로 지정
    after.Rvalue = before.Rvalue;
    after.Gvalue = before.Gvalue;
    after.Bvalue = before.Bvalue;

    return after;
}

bool isColliding(const ret& a, const ret& b);

bool ptinrect(int x, int y, ret& rect) {
    return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}

void Mouse(int button, int state, int x, int y);


// 타이머 콜백 (glutTimerFunc 등에서 호출될 함수 예시)
void TimerFunc(int value) {
    if (!timerRunning) return;

    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        glutPostRedisplay();
        glutTimerFunc(1000, TimerFunc, 0); // 1초마다 갱신
    }
}

void Motion(int x, int y);

void main(int argc, char** argv)
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Example1");
    //--- GLEW 초기화하기
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
    glutMotionFunc(Motion);
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
    //morph(morphed, *(it));
    //glColor3f(it->Rvalue, it->Gvalue, it->Bvalue);
    glRectf(morphed.x1, morphed.y1, morphed.x2, morphed.y2);

    // 그리기 부분 구현: 그리기 관련 부분이 여기에포함된다.
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);

}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'q': // 프로그램 종료
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

void Mouse(int button, int state, int x, int y)
{
    switch (button) {
    case GLUT_LEFT_BUTTON:
    {
        if (state == GLUT_DOWN) {
            
        }
        else if (state == GLUT_UP) {
           
        }
    }
    break;
    case GLUT_RIGHT_BUTTON:
    {
        if (state == GLUT_DOWN) {

        }
    }
    break;
    default:
        break;

    }
}

void Motion(int x, int y)
{
    
}

bool isColliding(const ret& a, const ret& b) {
    return !(a.x2 <= b.x1 || // A가 B의 왼쪽에 있음
        a.x1 >= b.x2 || // A가 B의 오른쪽에 있음
        a.y2 <= b.y1 || // A가 B의 아래에 있음
        a.y1 >= b.y2);  // A가 B의 위에 있음
}