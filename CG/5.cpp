#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>
#include <algorithm>

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_int_distribution<int> dis(0, 256);
std::uniform_int_distribution<int> numdis(0, 500 - 50);

#define rectspace 50

typedef struct RET {
    GLdouble x1, y1, x2, y2;
    GLdouble Rvalue = 0.0;
    GLdouble Gvalue = 0.0;
    GLdouble Bvalue = 0.0;
    
    // 애니메이션 필드들
    GLdouble velocityX = 0.0;
    GLdouble velocityY = 0.0;
    bool isActive = true;
    int lifeTime = 0;
} ret;

// 8개의 ret을 담는 새로운 구조체
typedef struct RECT_GROUP {
    ret mainRect;        // 원본 사각형
    ret pieces[8];       // 8개의 분할된 조각들
    bool isSplit = false; // 분할되었는지 여부
    bool isActive = true; // 전체 그룹이 활성 상태인지
} rect_group;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Split8Rectangle(int groupIndex);
void UpdateAnimation();

bool timerRunning = false;

GLdouble Rvalue = 0.0;
GLdouble Gvalue = 0.0;
GLdouble Bvalue = 1.0;

// 10개의 사각형 그룹 관리
rect_group rectGroups[10];

ret morph(ret& after, ret& before) {
    after.x1 = (before.x1 - 250) / 250;
    after.y1 = (before.y1 - 250) / -250;
    after.x2 = (before.x2 - 250) / 250;
    after.y2 = (before.y2 - 250) / -250;

    after.Rvalue = before.Rvalue;
    after.Gvalue = before.Gvalue;
    after.Bvalue = before.Bvalue;

    return after;
}

static ret hidingrect[10];
static ret showingrect[10];

bool ptinrect(int x, int y, ret& rect) {
    return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}

void Mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        // 클릭된 사각형 그룹 찾기 (분할되지 않은 원본만)
        for (int i = 0; i < 10; i++) {
            rect_group& group = rectGroups[i];
            
            if (!group.isActive || group.isSplit) continue;
            
            if (ptinrect(x, y, group.mainRect)) {
                Split8Rectangle(i);
                glutPostRedisplay();
                return;
            }
        }
    }
}

// 타이머 콜백
void TimerFunc(int value) {
    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
    }
    
    // 애니메이션 업데이트
    UpdateAnimation();
    
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunc, 0); // 60 FPS
}

void main(int argc, char** argv)
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(500, 500);
    glutCreateWindow("8-Split Rectangle System");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "GLEW Initialized\n";
    
    // 10개의 사각형 그룹 초기화
    for (int i = 0; i < 10; ++i) {
        rect_group& group = rectGroups[i];
        
        // 메인 사각형 설정
        group.mainRect.x1 = numdis(gen);
        group.mainRect.y1 = numdis(gen);
        group.mainRect.x2 = group.mainRect.x1 + rectspace;
        group.mainRect.y2 = group.mainRect.y1 + rectspace;
        group.mainRect.Rvalue = dis(gen) / 256.0f;
        group.mainRect.Gvalue = dis(gen) / 256.0f;
        group.mainRect.Bvalue = dis(gen) / 256.0f;
        group.mainRect.isActive = true;
        
        // 그룹 초기 상태
        group.isSplit = false;
        group.isActive = true;
        
        // 8개 조각들 비활성화 상태로 초기화
        for (int j = 0; j < 8; j++) {
            group.pieces[j].isActive = false;
        }
    }
    
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    
    // 애니메이션 타이머 시작
    glutTimerFunc(16, TimerFunc, 0);
    
    glutMainLoop();
}

GLvoid drawScene()
{
    glClearColor(Rvalue, Gvalue, Bvalue, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 모든 사각형 그룹 그리기
    for (int i = 0; i < 10; i++) {
        rect_group& group = rectGroups[i];
        
        if (!group.isActive) continue;
        
        if (!group.isSplit) {
            // 분할되지 않은 경우: 원본 사각형 그리기
            ret drawsupport;
            drawsupport = morph(drawsupport, group.mainRect);
            glColor3f(drawsupport.Rvalue, drawsupport.Gvalue, drawsupport.Bvalue);
            glRectf(drawsupport.x1, drawsupport.y1, drawsupport.x2, drawsupport.y2);
        } else {
            // 분할된 경우: 8개의 조각들 그리기
            for (int j = 0; j < 8; j++) {
                if (!group.pieces[j].isActive) continue;
                
                ret drawsupport;
                drawsupport = morph(drawsupport, group.pieces[j]);
                glColor3f(drawsupport.Rvalue, drawsupport.Gvalue, drawsupport.Bvalue);
                glRectf(drawsupport.x1, drawsupport.y1, drawsupport.x2, drawsupport.y2);
            }
        }
    }
    
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c': // 청록색 (초록+파랑)
        timerRunning = false;
        Rvalue = 0.0;
        Gvalue = 1.0;
        Bvalue = 1.0;
        break;

    case 'm': // 자홍색 (빨강+파랑)
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 0.0;
        Bvalue = 1.0;
        break;

    case 'y': // 노랑색 (빨강+초록)
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 1.0;
        Bvalue = 0.0;
        break;

    case 'a': // 랜덤색
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        timerRunning = false;
        break;

    case 'w': // 백색
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 1.0;
        Bvalue = 1.0;
        break;

    case 'k': // 검정색
        timerRunning = false;
        Rvalue = 0.0;
        Gvalue = 0.0;
        Bvalue = 0.0;
        break;

    case 't': // 타이머 시작/종료 (배경색 변경)
        timerRunning = !timerRunning;
        break;

    case 'r': // 리셋 - 원래 10개 사각형으로 복귀
        for (int i = 0; i < 10; ++i) {
            rect_group& group = rectGroups[i];
            
            // 메인 사각형 재설정
            group.mainRect.x1 = numdis(gen);
            group.mainRect.y1 = numdis(gen);
            group.mainRect.x2 = group.mainRect.x1 + rectspace;
            group.mainRect.y2 = group.mainRect.y1 + rectspace;
            group.mainRect.Rvalue = dis(gen) / 256.0f;
            group.mainRect.Gvalue = dis(gen) / 256.0f;
            group.mainRect.Bvalue = dis(gen) / 256.0f;
            group.mainRect.isActive = true;
            
            // 그룹 상태 리셋
            group.isSplit = false;
            group.isActive = true;
            
            // 8개 조각들 비활성화
            for (int j = 0; j < 8; j++) {
                group.pieces[j].isActive = false;
            }
        }
        break;

    case 'q': // 프로그램 종료
        glutLeaveMainLoop();
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

// 8분할 함수 - 당신의 규칙대로 정확히 구현!
void Split8Rectangle(int groupIndex) {
    if (groupIndex < 0 || groupIndex >= 10) return;
    if (!rectGroups[groupIndex].isActive || rectGroups[groupIndex].isSplit) return;
    
    rect_group& group = rectGroups[groupIndex];
    ret& original = group.mainRect;
    
    GLdouble x1 = original.x1, y1 = original.y1;
    GLdouble x2 = original.x2, y2 = original.y2;
    GLdouble width = x2 - x1;
    GLdouble height = y2 - y1;
    GLdouble halfWidth = width / 2;
    GLdouble halfHeight = height / 2;
    GLdouble quarterWidth = width / 4;
    GLdouble quarterHeight = height / 4;
    
    // 그룹을 분할 상태로 설정
    group.isSplit = true;
    
    // 모든 조각은 원본 색상 유지
    for (int i = 0; i < 8; i++) {
        group.pieces[i] = original; // 원본 정보 복사
        group.pieces[i].isActive = true;
        group.pieces[i].lifeTime = 0;
    }
    
    // === 4개 기본 격자 (2x2 분할) ===
    // 좌상 (0,0) ~ (10,10)
    group.pieces[0].x1 = x1;
    group.pieces[0].y1 = y1;
    group.pieces[0].x2 = x1 + halfWidth;
    group.pieces[0].y2 = y1 + halfHeight;
    group.pieces[0].velocityX = -1.5; group.pieces[0].velocityY = -1.5;
    
    // 좌하 (0,10) ~ (10,20)
    group.pieces[1].x1 = x1;
    group.pieces[1].y1 = y1 + halfHeight;
    group.pieces[1].x2 = x1 + halfWidth;
    group.pieces[1].y2 = y2;
    group.pieces[1].velocityX = -1.5; group.pieces[1].velocityY = 1.5;
    
    // 우상 (10,0) ~ (20,10)
    group.pieces[2].x1 = x1 + halfWidth;
    group.pieces[2].y1 = y1;
    group.pieces[2].x2 = x2;
    group.pieces[2].y2 = y1 + halfHeight;
    group.pieces[2].velocityX = 1.5; group.pieces[2].velocityY = -1.5;
    
    // 우하 (10,10) ~ (20,20)
    group.pieces[3].x1 = x1 + halfWidth;
    group.pieces[3].y1 = y1 + halfHeight;
    group.pieces[3].x2 = x2;
    group.pieces[3].y2 = y2;
    group.pieces[3].velocityX = 1.5; group.pieces[3].velocityY = 1.5;
    
    // === 4개 중간점 사각형 ===
    // (0,5) ~ (10,15) - 세로 중간 오프셋
    group.pieces[4].x1 = x1;
    group.pieces[4].y1 = y1 + quarterHeight;
    group.pieces[4].x2 = x1 + halfWidth;
    group.pieces[4].y2 = y2 - quarterHeight;
    group.pieces[4].velocityX = -2.0; group.pieces[4].velocityY = 0;
    
    // (5,0) ~ (15,10) - 가로 중간 오프셋
    group.pieces[5].x1 = x1 + quarterWidth;
    group.pieces[5].y1 = y1;
    group.pieces[5].x2 = x2 - quarterWidth;
    group.pieces[5].y2 = y1 + halfHeight;
    group.pieces[5].velocityX = 0; group.pieces[5].velocityY = -2.0;
    
    // (5,10) ~ (15,20) - 가로 중간, 세로 하단
    group.pieces[6].x1 = x1 + quarterWidth;
    group.pieces[6].y1 = y1 + halfHeight;
    group.pieces[6].x2 = x2 - quarterWidth;
    group.pieces[6].y2 = y2;
    group.pieces[6].velocityX = 0; group.pieces[6].velocityY = 2.0;
    
    // (10,5) ~ (20,15) - 우측, 세로 중간
    group.pieces[7].x1 = x1 + halfWidth;
    group.pieces[7].y1 = y1 + quarterHeight;
    group.pieces[7].x2 = x2;
    group.pieces[7].y2 = y2 - quarterHeight;
    group.pieces[7].velocityX = 2.0; group.pieces[7].velocityY = 0;
}

// 애니메이션 업데이트
void UpdateAnimation() {
    for (int i = 0; i < 10; i++) {
        rect_group& group = rectGroups[i];
        
        if (!group.isActive || !group.isSplit) continue;
        
        bool hasActivePiece = false;
        
        // 8개의 조각들 애니메이션 처리
        for (int j = 0; j < 8; j++) {
            ret& piece = group.pieces[j];
            
            if (!piece.isActive) continue;
            
            // 위치 업데이트
            piece.x1 += piece.velocityX;
            piece.y1 += piece.velocityY;
            piece.x2 += piece.velocityX;
            piece.y2 += piece.velocityY;
            
            // 크기 축소 (중심점 기준)
            GLdouble centerX = (piece.x1 + piece.x2) / 2;
            GLdouble centerY = (piece.y1 + piece.y2) / 2;
            GLdouble currentWidth = piece.x2 - piece.x1;
            GLdouble currentHeight = piece.y2 - piece.y1;
            
            GLdouble newWidth = currentWidth * 0.995;  // 0.5% 축소
            GLdouble newHeight = currentHeight * 0.995;
            
            piece.x1 = centerX - newWidth / 2;
            piece.x2 = centerX + newWidth / 2;
            piece.y1 = centerY - newHeight / 2;
            piece.y2 = centerY + newHeight / 2;
            
            piece.lifeTime++;
            
            // 삭제 조건: 크기가 10 이하 또는 600프레임(10초) 경과
            if (abs(piece.x2 - piece.x1) <= 10 || abs(piece.y2 - piece.y1) <= 10 || piece.lifeTime > 600) {
                piece.isActive = false;
            } else {
                hasActivePiece = true;
            }
        }
        
        // 모든 조각이 비활성화되면 그룹 전체를 비활성화
        if (!hasActivePiece) {
            group.isActive = false;
        }
    }
    
    // 비활성 사각형 제거
    for (auto& group : rectGroups) {
        if (!group.isActive) continue;
        
        // 원본 사각형만 체크
        if (!group.isSplit && !group.mainRect.isActive) {
            group.isActive = false;
        }
    }
    
    // 최종적으로 비활성화된 그룹은 제거
    std::vector<rect_group> activeGroups;
    for (auto& group : rectGroups) {
        if (group.isActive) {
            activeGroups.push_back(group);
        }
    }
    std::copy(activeGroups.begin(), activeGroups.end(), rectGroups);
}