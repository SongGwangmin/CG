#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>
#include <list>
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

GLdouble Rvalue = 0.0;
GLdouble Gvalue = 0.0;
GLdouble Bvalue = 1.0;

int width;
int height;

ret rects[10] = {
    // Body — x=50, y=100, w=180, h=80  -> x2=230, y2=180
    {50.0, 100.0, 230.0, 180.0, 0.823529, 0.650980, 0.474510, 1},
    // Head — x=230, y=90, w=48, h=48 -> x2=278, y2=138
    {230.0, 90.0, 278.0, 138.0, 0.878431, 0.721569, 0.572549, 2},
    // Ear — x=255, y=65, w=18, h=25 -> x2=273, y2=90
    {255.0, 65.0, 273.0, 90.0, 0.717647, 0.525490, 0.349020, 3},
    // Tail — x=30, y=120, w=20, h=8 -> x2=50, y2=128
    {30.0, 120.0, 50.0, 128.0, 0.717647, 0.525490, 0.349020, 0},
    // Front leg — x=80, y=180, w=18, h=40 -> x2=98, y2=220
    {80.0, 180.0, 98.0, 220.0, 0.717647, 0.525490, 0.349020, 1},
    // Back leg — x=165, y=180, w=14, h=44 -> x2=179, y2=224
    {165.0, 180.0, 179.0, 224.0, 0.717647, 0.525490, 0.349020, 1},
    // Paw (front) — x=95, y=220, w=12, h=6 -> x2=107, y2=226
    {95.0, 220.0, 107.0, 226.0, 0.549020, 0.352941, 0.235294, 0},
    // Collar — x=278, y=120, w=10, h=12 -> x2=288, y2=132
    {278.0, 120.0, 288.0, 132.0, 1.000000, 0.380392, 0.380392, 3},
    // Snout — x=278, y=110, w=12, h=10 -> x2=290, y2=120
    {278.0, 110.0, 290.0, 120.0, 0.780392, 0.607843, 0.439216, 3},
    // Belly stripe — x=120, y=180, w=30, h=10 -> x2=150, y2=190
    {120.0, 180.0, 150.0, 190.0, 0.792157, 0.627451, 0.478431, 1}
};


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

int startcount = 0;

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

    int i = 0;
    while (i < 10) {
        ret* newrect = new ret;
        newrect->x1 = rects[i].x1;
        newrect->y1 = rects[i].y1;
        newrect->x2 = rects[i].x2;
        newrect->y2 = rects[i].y2;
        newrect->Rvalue = rects[i].Rvalue;
        newrect->Gvalue = rects[i].Gvalue;
        newrect->Bvalue = rects[i].Bvalue;
        rectlist.push_back(*newrect);
        rectcount++;
        i++;
    }

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

    for (auto it = rectlist.begin(); it != rectlist.end(); ++it) {
        ret morphed;
        morph(morphed, *(it));
        glColor3f(it->Rvalue, it->Gvalue, it->Bvalue);
        glRectf(morphed.x1, morphed.y1, morphed.x2, morphed.y2);
    }

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
            if (!rectlist.empty()) {
                for (auto it = rectlist.rbegin(); it != rectlist.rend(); ++it) {
                    if (ptinrect(x, y, *it)) {
                        rectlist_iter = std::prev(it.base());
                        mousexstart = x;
                        mouseystart = y;
                        break;
                    }
                    rectlist_iter = rectlist.end();
                }
            }
        }
        else if (state == GLUT_UP) {
            if (!rectlist.empty() && rectlist_iter != rectlist.end()) {
                for (auto it = rectlist.begin(); it != rectlist.end(); ++it) {
                    if (it != rectlist_iter && isColliding(*rectlist_iter, *it)) {
                        // 더 작은 값으로 x1, y1 설정
                        rectlist_iter->x1 = std::min(rectlist_iter->x1, it->x1);
                        rectlist_iter->y1 = std::min(rectlist_iter->y1, it->y1);

                        // 더 큰 값으로 x2, y2 설정
                        rectlist_iter->x2 = std::max(rectlist_iter->x2, it->x2);
                        rectlist_iter->y2 = std::max(rectlist_iter->y2, it->y2);

                        // 새로운 색상 적용
                        rectlist_iter->Rvalue = dis(gen) / 256.0f;
                        rectlist_iter->Gvalue = dis(gen) / 256.0f;
                        rectlist_iter->Bvalue = dis(gen) / 256.0f;

                        // 충돌한 사각형 삭제
                        rectlist.erase(it);
                        rectcount--;
                        break; // erase 후 반복자가 무효화되므로 루프 종료
                    }
                }
                rectlist_iter = rectlist.end();
            }
        }
    }
    break;
    case GLUT_RIGHT_BUTTON:
    {
        if (state == GLUT_DOWN) {
            if (!rectlist.empty()) {
                for (auto it = rectlist.rbegin(); it != rectlist.rend(); ++it) {
                    if (ptinrect(x, y, *it)) {
                        // 우클릭으로 사각형 누르면 못움직이게 하기
                        mousexstart = -1;
                        mouseystart = -1;
                        rectlist_iter = std::prev(it.base());

                        if (rectcount < 30) { // 2개를 추가할 것이므로 29 미만일 때
                            // rectlist_iter의 사각형 범위 저장
                            GLdouble minX = rectlist_iter->x1 - 10;
                            GLdouble minY = rectlist_iter->y1 - 10;
                            GLdouble maxX = rectlist_iter->x2 + 10;
                            GLdouble maxY = rectlist_iter->y2 + 10;

                            // 화면 범위(0~500)를 벗어나지 않도록 경계 검사
                            if (minX < 0) minX = 0;
                            if (minY < 0) minY = 0;
                            if (maxX > 500) maxX = 500;
                            if (maxY > 500) maxY = 500;

                            // 첫 번째 새로운 사각형 생성
                            std::uniform_real_distribution<GLdouble> xDis(minX, maxX);
                            std::uniform_real_distribution<GLdouble> yDis(minY, maxY);

                            ret newrect1;
                            newrect1.x1 = xDis(gen);
                            newrect1.y1 = yDis(gen);
                            newrect1.x2 = xDis(gen);
                            newrect1.y2 = yDis(gen);

                            // newrect1 정규화: x2 > x1, y2 > y1이 되도록 보장
                            if (newrect1.x2 < newrect1.x1) {
                                std::swap(newrect1.x1, newrect1.x2);
                            }
                            if (newrect1.y2 < newrect1.y1) {
                                std::swap(newrect1.y1, newrect1.y2);
                            }

                            newrect1.Rvalue = dis(gen) / 256.0f;
                            newrect1.Gvalue = dis(gen) / 256.0f;
                            newrect1.Bvalue = dis(gen) / 256.0f;



                            // 두 번째 새로운 사각형 생성
                            ret newrect2;
                            newrect2.x1 = xDis(gen);
                            newrect2.y1 = yDis(gen);
                            newrect2.x2 = xDis(gen);
                            newrect2.y2 = yDis(gen);

                            // newrect2 정규화: x2 > x1, y2 > y1이 되도록 보장
                            if (newrect2.x2 < newrect2.x1) {
                                std::swap(newrect2.x1, newrect2.x2);
                            }
                            if (newrect2.y2 < newrect2.y1) {
                                std::swap(newrect2.y1, newrect2.y2);
                            }

                            newrect2.Rvalue = dis(gen) / 256.0f;
                            newrect2.Gvalue = dis(gen) / 256.0f;
                            newrect2.Bvalue = dis(gen) / 256.0f;

                            // 두 개의 새로운 사각형을 리스트에 추가
                            rectlist.push_back(newrect1);
                            rectlist.push_back(newrect2);
                            rectcount += 2;

                            // 원래 사각형 삭제
                            rectlist.erase(rectlist_iter);
                            rectlist_iter = rectlist.end();
                            rectcount--;

                            glutPostRedisplay();
                        }

                        break;
                    }
                    rectlist_iter = rectlist.end();
                }
            }
        }
    }
    break;
    default:
        break;

    }
}

void Motion(int x, int y)
{
    if (!rectlist.empty()) {
        if (mousexstart != -1) {
            if (rectlist_iter != rectlist.end()) {
                mousexend = x;
                mouseyend = y;

                int dx = mousexend - mousexstart;
                int dy = mouseyend - mouseystart;
                mousexstart = mousexend;
                mouseystart = mouseyend;

                rectlist_iter->x1 += dx;
                rectlist_iter->y1 += dy;
                rectlist_iter->x2 += dx;
                rectlist_iter->y2 += dy;
                glutPostRedisplay();
            }
        }
    }
}

bool isColliding(const ret& a, const ret& b) {
    return !(a.x2 <= b.x1 || // A가 B의 왼쪽에 있음
        a.x1 >= b.x2 || // A가 B의 오른쪽에 있음
        a.y2 <= b.y1 || // A가 B의 아래에 있음
        a.y1 >= b.y2);  // A가 B의 위에 있음
}