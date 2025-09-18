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
std::uniform_int_distribution<int> splitDis(1, 4);      // 분할 타입 (1~4)
std::uniform_int_distribution<int> directionDis(0, 7);  // 이동 방향 (0~7)

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
    
    // 새로 추가되는 변수들
    int splitType = 1;   // 분할 상태 (1:상하좌우, 2:대각선, 3:팔방, 4:한방향)
    int moveDirection = 0; // 4번 상태일 때 이동 방향 (0~7: 팔방)
} rect_group;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Split8Rectangle(int groupIndex);
void UpdateAnimation();
void InitializePieces(int groupIndex);

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
        // 클릭된 작은 조각 찾기
        for (int i = 0; i < 10; i++) {
            rect_group& group = rectGroups[i];
            
            if (!group.isActive) continue;
            
            // 8개 조각 중 하나라도 클릭되면 전체 그룹의 조각들이 이동 시작
            for (int j = 0; j < 8; j++) {
                if (!group.pieces[j].isActive) continue;
                
                if (ptinrect(x, y, group.pieces[j])) {
                    // 콘솔에 클릭된 조각 정보 출력
                    std::cout << "=== Small Piece Clicked ===" << std::endl;
                    std::cout << "Group Index: " << i << ", Piece: " << j << std::endl;
                    std::cout << "Split Type: " << group.splitType;
                    
                    switch(group.splitType) {
                        case 1: std::cout << " (All move, show 0~3)"; break;
                        case 2: std::cout << " (All move, show 4~7)"; break; 
                        case 3: std::cout << " (All move, show all)"; break;
                        case 4: std::cout << " (All move same direction, show 0~3)"; break;
                    }
                    std::cout << std::endl;
                    
                    if (group.splitType == 4) {
                        std::cout << "Move Direction: " << group.moveDirection;
                        const char* dirNames[] = {"Up", "Up-Right", "Right", "Down-Right", 
                                                "Down", "Down-Left", "Left", "Up-Left"};
                        std::cout << " (" << dirNames[group.moveDirection] << ")" << std::endl;
                    }
                    
                    std::cout << "=============================" << std::endl;
                    
                    // 이동 시작
                    Split8Rectangle(i);
                    glutPostRedisplay();
                    return;
                }
            }
        }
        
        // 클릭했지만 조각이 없는 경우
        std::cout << "No piece clicked at (" << x << "," << y << ")" << std::endl;
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
        
        // 메인 사각형 설정 (기준용)
        group.mainRect.x1 = numdis(gen);
        group.mainRect.y1 = numdis(gen);
        group.mainRect.x2 = group.mainRect.x1 + rectspace;
        group.mainRect.y2 = group.mainRect.y1 + rectspace;
        group.mainRect.Rvalue = dis(gen) / 256.0f;
        group.mainRect.Gvalue = dis(gen) / 256.0f;
        group.mainRect.Bvalue = dis(gen) / 256.0f;
        group.mainRect.isActive = true;
        
        // 그룹 초기 상태
        group.isActive = true;
        group.splitType = splitDis(gen);        // 1~4 중 랜덤
        group.moveDirection = directionDis(gen); // 0~7 중 랜덤
        
        // 8개 조각들을 원본 위치에 8분할 규칙으로 배치
        InitializePieces(i);
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

    // 모든 사각형 그룹의 8개 조각들 그리기 (원본 사각형은 그리지 않음)
    for (int i = 0; i < 10; i++) {
        rect_group& group = rectGroups[i];
        
        if (!group.isActive) continue;
        
        // 항상 8개의 작은 조각들만 그리기
        for (int j = 0; j < 8; j++) {
            if (!group.pieces[j].isActive) continue;
            
            ret drawsupport;
            drawsupport = morph(drawsupport, group.pieces[j]);
            glColor3f(drawsupport.Rvalue, drawsupport.Gvalue, drawsupport.Bvalue);
            glRectf(drawsupport.x1, drawsupport.y1, drawsupport.x2, drawsupport.y2);
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
            
            // 메인 사각형 재설정 (기준용)
            group.mainRect.x1 = numdis(gen);
            group.mainRect.y1 = numdis(gen);
            group.mainRect.x2 = group.mainRect.x1 + rectspace;
            group.mainRect.y2 = group.mainRect.y1 + rectspace;
            group.mainRect.Rvalue = dis(gen) / 256.0f;
            group.mainRect.Gvalue = dis(gen) / 256.0f;
            group.mainRect.Bvalue = dis(gen) / 256.0f;
            group.mainRect.isActive = true;
            
            // 그룹 상태 리셋
            group.isActive = true;
            group.splitType = splitDis(gen);
            group.moveDirection = directionDis(gen);
            
            // 8개 조각들을 새 위치에 8분할 규칙으로 재배치
            InitializePieces(i);
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

// 8분할 함수 - splitType에 따라 다른 패턴으로 분할
void Split8Rectangle(int groupIndex) {
    if (groupIndex < 0 || groupIndex >= 10) return;
    if (!rectGroups[groupIndex].isActive) return;
    
    rect_group& group = rectGroups[groupIndex];
    ret& original = group.mainRect;
    
    // 이동 시작 정보 출력
    std::cout << ">>> Starting movement for Rectangle " << groupIndex << " <<<" << std::endl;
    
    // 각 조각이 자신의 위치에서 바깥쪽으로 이동하는 방향 벡터
    const GLdouble directions[8][2] = {
        {-1.4, -1.4}, // 0: 좌상 → 좌상대각선으로
        {-1.4, 1.4},  // 1: 좌하 → 좌하대각선으로
        {1.4, -1.4},  // 2: 우상 → 우상대각선으로  
        {1.4, 1.4},   // 3: 우하 → 우하대각선으로
        {-2, 0},      // 4: 좌측 중간 → 왼쪽으로
        {0, -2},      // 5: 상단 중간 → 위로
        {0, 2},       // 6: 하단 중간 → 아래로
        {2, 0}        // 7: 우측 중간 → 오른쪽으로
    };
    
    // 모든 조각의 색상을 동일하게 유지
    for (int i = 0; i < 8; i++) {
        group.pieces[i].Rvalue = original.Rvalue;
        group.pieces[i].Gvalue = original.Gvalue;
        group.pieces[i].Bvalue = original.Bvalue;
        group.pieces[i].lifeTime = 0;
    }
    
    switch (group.splitType) {
        case 1: { // 상하좌우 - 모든 조각 이동하되, 0~3번만 출력
            std::cout << "All pieces move, but only show 0~3 (corners)" << std::endl;
            
            // 모든 조각(0~7번)에 해당 위치에 맞는 이동 속도 부여
            for (int i = 0; i < 8; i++) {
                group.pieces[i].velocityX = directions[i][0];
                group.pieces[i].velocityY = directions[i][1];
            }
            
            // 4~7번 조각을 보이지 않게 (이동은 하되 화면에 표시 안함)
            for (int i = 4; i < 8; i++) {
                group.pieces[i].isActive = false;
            }
            break;
        }
        case 2: { // 대각선 - 모든 조각 이동하되, 4~7번만 출력
            std::cout << "All pieces move, but only show 4~7 (middles)" << std::endl;
            
            // 모든 조각(0~7번)에 해당 위치에 맞는 이동 속도 부여
            for (int i = 0; i < 8; i++) {
                group.pieces[i].velocityX = directions[i][0];
                group.pieces[i].velocityY = directions[i][1];
            }
            
            // 0~3번 조각을 보이지 않게 (이동은 하되 화면에 표시 안함)
            for (int i = 0; i < 4; i++) {
                group.pieces[i].isActive = false;
            }
            break;
        }
        case 3: { // 팔방으로 - 모든 8개 사각형 이동하고 출력
            std::cout << "All 8 pieces move and show" << std::endl;
            
            // 모든 조각이 해당 위치에서 바깥쪽으로 이동
            for (int i = 0; i < 8; i++) {
                group.pieces[i].velocityX = directions[i][0];
                group.pieces[i].velocityY = directions[i][1];
            }
            break;
        }
        case 4: { // 0~3번 사각형이 한방향으로 - 모든 조각 이동하되, 0~3번만 출력
            const char* dirNames[] = {"Up-Left", "Down-Left", "Up-Right", "Down-Right", 
                                    "Left", "Up", "Down", "Right"};
            std::cout << "All pieces move in their respective directions, but only show 0~3" << std::endl;
            
            // Type 4에서는 모든 조각이 동일한 방향으로 이동
            GLdouble moveX = directions[group.moveDirection % 8][0];
            GLdouble moveY = directions[group.moveDirection % 8][1];
            
            // 모든 조각을 같은 방향으로 이동
            for (int i = 0; i < 8; i++) {
                group.pieces[i].velocityX = moveX;
                group.pieces[i].velocityY = moveY;
            }
            
            // 4~7번 조각을 보이지 않게 (이동은 하되 화면에 표시 안함)
            for (int i = 4; i < 8; i++) {
                group.pieces[i].isActive = false;
            }
            break;
        }
    }
    
    std::cout << "Movement started!" << std::endl << std::endl;
}

// 8개의 작은 조각들을 원본 사각형 위치에 배치하는 함수
void InitializePieces(int groupIndex) {
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
    
    // 모든 조각은 원본과 동일한 색상으로 통일
    for (int i = 0; i < 8; i++) {
        group.pieces[i] = original;
        group.pieces[i].isActive = true;
        group.pieces[i].lifeTime = 0;
        group.pieces[i].velocityX = 0.0;
        group.pieces[i].velocityY = 0.0;
        
        // 모든 조각이 동일한 색상을 가지도록 설정
        group.pieces[i].Rvalue = original.Rvalue;
        group.pieces[i].Gvalue = original.Gvalue;
        group.pieces[i].Bvalue = original.Bvalue;
    }
    
    // === 당신의 8분할 규칙에 따라 배치 ===
    
    // 4개 기본 격자 (2x2 분할)
    // 좌상 (0,0) ~ (10,10)
    group.pieces[0].x1 = x1;
    group.pieces[0].y1 = y1;
    group.pieces[0].x2 = x1 + halfWidth;
    group.pieces[0].y2 = y1 + halfHeight;
    
    // 좌하 (0,10) ~ (10,20)
    group.pieces[1].x1 = x1;
    group.pieces[1].y1 = y1 + halfHeight;
    group.pieces[1].x2 = x1 + halfWidth;
    group.pieces[1].y2 = y2;
    
    // 우상 (10,0) ~ (20,10)
    group.pieces[2].x1 = x1 + halfWidth;
    group.pieces[2].y1 = y1;
    group.pieces[2].x2 = x2;
    group.pieces[2].y2 = y1 + halfHeight;
    
    // 우하 (10,10) ~ (20,20)
    group.pieces[3].x1 = x1 + halfWidth;
    group.pieces[3].y1 = y1 + halfHeight;
    group.pieces[3].x2 = x2;
    group.pieces[3].y2 = y2;
    
    // === 4개 중간점 사각형 (겹치는 부분) ===
    // (0,5) ~ (10,15) - 세로 중간 오프셋
    group.pieces[4].x1 = x1;
    group.pieces[4].y1 = y1 + quarterHeight;
    group.pieces[4].x2 = x1 + halfWidth;
    group.pieces[4].y2 = y2 - quarterHeight;
    
    // (5,0) ~ (15,10) - 가로 중간 오프셋
    group.pieces[5].x1 = x1 + quarterWidth;
    group.pieces[5].y1 = y1;
    group.pieces[5].x2 = x2 - quarterWidth;
    group.pieces[5].y2 = y1 + halfHeight;
    
    // (5,10) ~ (15,20) - 가로 중간, 세로 하단
    group.pieces[6].x1 = x1 + quarterWidth;
    group.pieces[6].y1 = y1 + halfHeight;
    group.pieces[6].x2 = x2 - quarterWidth;
    group.pieces[6].y2 = y2;
    
    // (10,5) ~ (20,15) - 우측, 세로 중간
    group.pieces[7].x1 = x1 + halfWidth;
    group.pieces[7].y1 = y1 + quarterHeight;
    group.pieces[7].x2 = x2;
    group.pieces[7].y2 = y2 - quarterHeight;
    
    // 그룹을 "항상 분할된 상태"로 설정
    group.isSplit = true;
}

// 애니메이션 업데이트
void UpdateAnimation() {
    for (int i = 0; i < 10; i++) {
        rect_group& group = rectGroups[i];
        
        if (!group.isActive) continue;
        
        bool hasActivePiece = false;
        
        // 8개의 조각들 애니메이션 처리
        for (int j = 0; j < 8; j++) {
            ret& piece = group.pieces[j];
            
            if (!piece.isActive) continue;
            
            // 이동 중인 조각들만 위치 업데이트 및 축소
            if (abs(piece.velocityX) > 0.01 || abs(piece.velocityY) > 0.01) {
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
                
                GLdouble newWidth = currentWidth * 0.96;  // 4% 축소
                GLdouble newHeight = currentHeight * 0.96;
                
                piece.x1 = centerX - newWidth / 2;
                piece.x2 = centerX + newWidth / 2;
                piece.y1 = centerY - newHeight / 2;
                piece.y2 = centerY + newHeight / 2;
                
                piece.lifeTime++;
                
                // 삭제 조건: 크기가 5 이하 또는 300프레임(5초) 경과
                if (abs(piece.x2 - piece.x1) <= 5 || abs(piece.y2 - piece.y1) <= 5 || piece.lifeTime > 300) {
                    piece.isActive = false;
                } else {
                    hasActivePiece = true;
                }
            } else {
                // 이동하지 않는 조각들은 그대로 유지
                hasActivePiece = true;
            }
        }
        
        // 모든 조각이 비활성화되면 그룹 자체를 비활성화 (r키로만 재생성)
        if (!hasActivePiece) {
            std::cout << "All pieces in group " << i << " disappeared" << std::endl;
            group.isActive = false;
        }
    }
}