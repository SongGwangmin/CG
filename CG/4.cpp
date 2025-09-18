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

std::list<ret> rectlist;
std::list<ret>::iterator rectlist_iter;
int rectcount = 0;

bool isLeftMousePressed = false; // 왼쪽 마우스 버튼이 눌린 상태인지 확인하는 변수

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

    // 처음에 20개의 사각형 생성
    for (int i = 0; i < 20; ++i) {
        ret newrect;
        newrect.x1 = numdis(gen);
        newrect.y1 = numdis(gen);
        newrect.x2 = newrect.x1 + rectspace;
        newrect.y2 = newrect.y1 + rectspace;
        newrect.Rvalue = dis(gen) / 256.0f;
        newrect.Gvalue = dis(gen) / 256.0f;
        newrect.Bvalue = dis(gen) / 256.0f;
        rectlist.push_back(newrect);
        rectcount++;
    }

    // 새로운 특별한 사각형 생성 (크기는 원래의 2배)
    ret specialRect;
    specialRect.x1 = 250; // 화면 중앙 근처
    specialRect.y1 = 250;
    specialRect.x2 = specialRect.x1 + (rectspace * 2); // 가로 2배
    specialRect.y2 = specialRect.y1 + (rectspace * 2); // 세로 2배
    specialRect.Rvalue = 0.0f; // 검은색으로 설정
    specialRect.Gvalue = 0.0f;
    specialRect.Bvalue = 0.0f;
    
    rectlist.push_back(specialRect);
    rectlist_iter = std::prev(rectlist.end()); // 마지막 요소를 가리킴
    rectcount++;

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
        // 특별한 사각형은 왼쪽 마우스 버튼이 눌린 상태에서만 표시
        if (it == rectlist_iter && !isLeftMousePressed) {
            continue; // 특별한 사각형을 건너뛰기
        }
        
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
    case 'r': // 리셋: 모든 것을 초기 상태로 되돌리기
        {
            // 리스트 완전히 비우기
            rectlist.clear();
            rectcount = 0;
            isLeftMousePressed = false;
            
            // 처음에 20개의 사각형 생성
            for (int i = 0; i < 20; ++i) {
                ret newrect;
                newrect.x1 = numdis(gen);
                newrect.y1 = numdis(gen);
                newrect.x2 = newrect.x1 + rectspace;
                newrect.y2 = newrect.y1 + rectspace;
                newrect.Rvalue = dis(gen) / 256.0f;
                newrect.Gvalue = dis(gen) / 256.0f;
                newrect.Bvalue = dis(gen) / 256.0f;
                rectlist.push_back(newrect);
                rectcount++;
            }

            // 새로운 특별한 사각형 생성 (원래 크기로 복구)
            ret specialRect;
            specialRect.x1 = 250; // 화면 중앙 근처
            specialRect.y1 = 250;
            specialRect.x2 = specialRect.x1 + (rectspace * 2); // 가로 2배
            specialRect.y2 = specialRect.y1 + (rectspace * 2); // 세로 2배
            specialRect.Rvalue = 0.0f; // 검은색으로 설정
            specialRect.Gvalue = 0.0f;
            specialRect.Bvalue = 0.0f;
            
            rectlist.push_back(specialRect);
            rectlist_iter = std::prev(rectlist.end()); // 마지막 요소를 가리킴
            rectcount++; // 특별한 사각형도 카운트에 포함
            break;
        }

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
            isLeftMousePressed = true;
            // 특별한 사각형을 마우스 위치로 이동
            if (rectlist_iter != rectlist.end()) {
                GLdouble rectWidth = rectlist_iter->x2 - rectlist_iter->x1;
                GLdouble rectHeight = rectlist_iter->y2 - rectlist_iter->y1;
                rectlist_iter->x1 = x - rectWidth / 2;
                rectlist_iter->y1 = y - rectHeight / 2;
                rectlist_iter->x2 = rectlist_iter->x1 + rectWidth;
                rectlist_iter->y2 = rectlist_iter->y1 + rectHeight;
                glutPostRedisplay();
            }
        }
        else if (state == GLUT_UP) {
            isLeftMousePressed = false;
            // 마우스를 놓으면 특별한 사각형의 색상을 다시 검은색으로 변경
            if (rectlist_iter != rectlist.end()) {
                rectlist_iter->Rvalue = 0.0f;
                rectlist_iter->Gvalue = 0.0f;
                rectlist_iter->Bvalue = 0.0f;
            }
            glutPostRedisplay();
        }
    }
    break;
    case GLUT_RIGHT_BUTTON:
    {
        if (state == GLUT_DOWN) {
            // rectcount가 20 미만이면 새로운 사각형 추가
            if (rectcount < 20) {
                ret newrect;
                // 클릭한 위치를 중심으로 사각형 생성
                newrect.x1 = x - rectspace / 2;
                newrect.y1 = y - rectspace / 2;
                newrect.x2 = x + rectspace / 2;
                newrect.y2 = y + rectspace / 2;
                
                // 화면 경계를 벗어나지 않도록 조정
                if (newrect.x1 < 0) {
                    newrect.x2 += -newrect.x1;
                    newrect.x1 = 0;
                }
                if (newrect.x2 > 500) {
                    newrect.x1 -= (newrect.x2 - 500);
                    newrect.x2 = 500;
                }
                if (newrect.y1 < 0) {
                    newrect.y2 += -newrect.y1;
                    newrect.y1 = 0;
                }
                if (newrect.y2 > 500) {
                    newrect.y1 -= (newrect.y2 - 500);
                    newrect.y2 = 500;
                }
                
                newrect.Rvalue = dis(gen) / 256.0f;
                newrect.Gvalue = dis(gen) / 256.0f;
                newrect.Bvalue = dis(gen) / 256.0f;
                
                // 특별한 사각형 앞에 추가 (특별한 사각형이 마지막에 유지되도록)
                rectlist.insert(rectlist_iter, newrect);
                rectcount++;
                
                // 특별한 사각형 크기 감소 (x1, y1 5씩 증가, x2, y2 5씩 감소)
                if (rectlist_iter != rectlist.end()) {
                    rectlist_iter->x1 += 5;
                    rectlist_iter->y1 += 5;
                    rectlist_iter->x2 -= 5;
                    rectlist_iter->y2 -= 5;
                    
                    // 사각형 크기가 너무 작아지지 않도록 최소 크기 보장
                    if (rectlist_iter->x2 <= rectlist_iter->x1) {
                        GLdouble centerX = (rectlist_iter->x1 + rectlist_iter->x2) / 2;
                        rectlist_iter->x1 = centerX - 5;
                        rectlist_iter->x2 = centerX + 5;
                    }
                    if (rectlist_iter->y2 <= rectlist_iter->y1) {
                        GLdouble centerY = (rectlist_iter->y1 + rectlist_iter->y2) / 2;
                        rectlist_iter->y1 = centerY - 5;
                        rectlist_iter->y2 = centerY + 5;
                    }
                }
                
                glutPostRedisplay();
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
    // 왼쪽 마우스 버튼이 눌린 상태에서 특별한 사각형을 마우스를 따라 이동
    if (isLeftMousePressed && rectlist_iter != rectlist.end()) {
        GLdouble rectWidth = rectlist_iter->x2 - rectlist_iter->x1;
        GLdouble rectHeight = rectlist_iter->y2 - rectlist_iter->y1;
        rectlist_iter->x1 = x - rectWidth / 2;
        rectlist_iter->y1 = y - rectHeight / 2;
        rectlist_iter->x2 = rectlist_iter->x1 + rectWidth;
        rectlist_iter->y2 = rectlist_iter->y1 + rectHeight;
        
        // 다른 사각형들과의 충돌 검사
        for (auto it = rectlist.begin(); it != rectlist.end(); ) {
            if (it != rectlist_iter && isColliding(*rectlist_iter, *it)) {
                // 충돌한 사각형의 색상을 특별한 사각형에 적용
                rectlist_iter->Rvalue = it->Rvalue;
                rectlist_iter->Gvalue = it->Gvalue;
                rectlist_iter->Bvalue = it->Bvalue;
                
                // 특별한 사각형 크기 증가 (x1, y1 5씩 감소, x2, y2 5씩 증가)
                rectlist_iter->x1 -= 5;
                rectlist_iter->y1 -= 5;
                rectlist_iter->x2 += 5;
                rectlist_iter->y2 += 5;
                
                // 충돌한 사각형 삭제
                it = rectlist.erase(it);
                rectcount--;
            } else {
                ++it;
            }
        }
        
        glutPostRedisplay();
    }
}

bool isColliding(const ret& a, const ret& b) {
    return !(a.x2 <= b.x1 || // A가 B의 왼쪽에 있음
        a.x1 >= b.x2 || // A가 B의 오른쪽에 있음
        a.y2 <= b.y1 || // A가 B의 아래에 있음
        a.y1 >= b.y2);  // A가 B의 위에 있음
}