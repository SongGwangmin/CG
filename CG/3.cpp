#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>
#include <algorithm>
#include <cmath>

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

int first = 0;

ret showingrect[5];
int rectCount = 0;          // 현재 생성된 사각형 개수
int selectedRect = -1;      // 선택된 사각형 인덱스 (-1이면 선택 없음)
bool followMode = false;    // 따라가기 모드 여부
GLdouble targetX, targetY;  // 목표 위치

// 이전 위치 저장용 배열 (순간이동을 위해)
ret previousPos[5];

// 대각선 이동 및 튕기기 모드용 변수
bool bounceMode = false;    // 튕기기 모드 여부
GLdouble velocityX[5];      // 각 사각형의 X 방향 속도
GLdouble velocityY[5];      // 각 사각형의 Y 방향 속도
const GLdouble BOUNCE_SPEED = 3.0; // 이동 속도

// 지그재그 이동 모드용 변수
bool zigzagMode = false;    // 지그재그 모드 여부
GLdouble zigzagSpeedX[5];   // 지그재그 X 방향 기본 속도
GLdouble zigzagYOffset[5];  // Y 방향 오프셋 (벽에 닿을 때마다 변경)
const GLdouble ZIGZAG_BASE_SPEED = 2.0; // 기본 이동 속도
const GLdouble ZIGZAG_Y_CHANGE = 20.0;  // Y 좌표 변경량

// 형태 변화 모드용 변수
bool morphMode = false;     // 형태 변화 모드 여부
std::uniform_real_distribution<GLdouble> morphDis(-5.0, 5.0); // -5에서 5 사이 난수

// 색상 변화 모드용 변수
bool colorMode = false;     // 색상 변화 모드 여부
std::uniform_real_distribution<GLdouble> colorDis(0.0, 1.0); // 0.0에서 1.0 사이 난수


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


// 따라가기 애니메이션 업데이트 함수 (순간이동 방식)
void UpdateFollowAnimation() {
    if (!followMode || selectedRect == -1) return;
    
    // 현재 모든 사각형의 위치를 이전 위치로 저장
    for (int i = 0; i < rectCount; ++i) {
        previousPos[i] = showingrect[i];
    }
    
    // 선택된 사각형을 목표 위치로 순간이동
    GLdouble currentCenterX = (showingrect[selectedRect].x1 + showingrect[selectedRect].x2) / 2;
    GLdouble currentCenterY = (showingrect[selectedRect].y1 + showingrect[selectedRect].y2) / 2;
    
    GLdouble dx = targetX - currentCenterX;
    GLdouble dy = targetY - currentCenterY;
    
    showingrect[selectedRect].x1 += dx;
    showingrect[selectedRect].y1 += dy;
    showingrect[selectedRect].x2 += dx;
    showingrect[selectedRect].y2 += dy;
    
    // 체인처럼 연결된 순간이동 방식
    for (int i = 0; i < rectCount; ++i) {
        if (i != selectedRect) {
            int targetIndex = -1; // 따라갈 대상의 인덱스
            
            if (selectedRect == 0) {
                // 0번이 선택된 경우: 1번은 0번을, 2번은 1번을, 3번은 2번을, 4번은 3번을 따라감
                if (i > 0) {
                    targetIndex = i - 1;
                }
            } else if (selectedRect == 1) {
                // 1번이 선택된 경우: 0번은 1번을, 2번은 0번을, 3번은 2번을, 4번은 3번을 따라감
                if (i == 0) {
                    targetIndex = 1; // 0번은 1번을 따라감
                } else if (i == 2) {
                    targetIndex = 0; // 2번은 0번을 따라감
                } else if (i > 2) {
                    targetIndex = i - 1; // 3번은 2번을, 4번은 3번을 따라감
                }
            } else if (selectedRect == 2) {
                // 2번이 선택된 경우: 1번은 2번을, 0번은 1번을, 3번은 2번을, 4번은 3번을 따라감
                if (i == 1) {
                    targetIndex = 2; // 1번은 2번을 따라감
                } else if (i == 0) {
                    targetIndex = 1; // 0번은 1번을 따라감
                } else if (i == 3) {
                    targetIndex = 2; // 3번은 2번을 따라감
                } else if (i == 4) {
                    targetIndex = 3; // 4번은 3번을 따라감
                }
            } else if (selectedRect == 3) {
                // 3번이 선택된 경우: 2번은 3번을, 1번은 2번을, 0번은 1번을, 4번은 3번을 따라감
                if (i == 2) {
                    targetIndex = 3; // 2번은 3번을 따라감
                } else if (i == 1) {
                    targetIndex = 2; // 1번은 2번을 따라감
                } else if (i == 0) {
                    targetIndex = 1; // 0번은 1번을 따라감
                } else if (i == 4) {
                    targetIndex = 3; // 4번은 3번을 따라감
                }
            } else if (selectedRect == 4) {
                // 4번이 선택된 경우: 3번은 4번을, 2번은 3번을, 1번은 2번을, 0번은 1번을 따라감
                if (i == 3) {
                    targetIndex = 4; // 3번은 4번을 따라감
                } else if (i == 2) {
                    targetIndex = 3; // 2번은 3번을 따라감
                } else if (i == 1) {
                    targetIndex = 2; // 1번은 2번을 따라감
                } else if (i == 0) {
                    targetIndex = 1; // 0번은 1번을 따라감
                }
            }
            
            // 따라갈 대상이 있고, 해당 대상이 유효한 범위 내에 있을 때
            if (targetIndex >= 0 && targetIndex < rectCount) {
                // 한 타이머 전의 목표 사각형 위치로 순간이동
                GLdouble prevCenterX = (previousPos[targetIndex].x1 + previousPos[targetIndex].x2) / 2;
                GLdouble prevCenterY = (previousPos[targetIndex].y1 + previousPos[targetIndex].y2) / 2;
                GLdouble currentCenterX = (showingrect[i].x1 + showingrect[i].x2) / 2;
                GLdouble currentCenterY = (showingrect[i].y1 + showingrect[i].y2) / 2;
                
                GLdouble fdx = prevCenterX - currentCenterX;
                GLdouble fdy = prevCenterY - currentCenterY;
                
                // 순간이동: 이전 위치로 바로 이동
                showingrect[i].x1 += fdx;
                showingrect[i].y1 += fdy;
                showingrect[i].x2 += fdx;
                showingrect[i].y2 += fdy;
            }
        }
    }
}

// 대각선 이동 및 튕기기 업데이트 함수
void UpdateBounceAnimation() {
    if (!bounceMode) return;
    
    for (int i = 0; i < rectCount; ++i) {
        // 현재 위치 업데이트
        showingrect[i].x1 += velocityX[i];
        showingrect[i].y1 += velocityY[i];
        showingrect[i].x2 += velocityX[i];
        showingrect[i].y2 += velocityY[i];
        
        // 경계 충돌 검사 및 튕기기 (500x500 윈도우)
        // 왼쪽 경계 충돌
        if (showingrect[i].x1 <= 0) {
            showingrect[i].x1 = 0;
            showingrect[i].x2 = showingrect[i].x2 - showingrect[i].x1;
            velocityX[i] = -velocityX[i]; // X 방향 반전
        }
        // 오른쪽 경계 충돌
        else if (showingrect[i].x2 >= 500) {
            showingrect[i].x2 = 500;
            showingrect[i].x1 = 500 - (showingrect[i].x2 - showingrect[i].x1);
            velocityX[i] = -velocityX[i]; // X 방향 반전
        }
        
        // 위쪽 경계 충돌
        if (showingrect[i].y1 <= 0) {
            showingrect[i].y1 = 0;
            showingrect[i].y2 = showingrect[i].y2 - showingrect[i].y1;
            velocityY[i] = -velocityY[i]; // Y 방향 반전
        }
        // 아래쪽 경계 충돌
        else if (showingrect[i].y2 >= 500) {
            showingrect[i].y2 = 500;
            showingrect[i].y1 = 500 - (showingrect[i].y2 - showingrect[i].y1);
            velocityY[i] = -velocityY[i]; // Y 방향 반전
        }
    }
}

// 지그재그 애니메이션 업데이트 함수 (직선 형태)
void UpdateZigzagAnimation() {
    if (!zigzagMode) return;
    
    for (int i = 0; i < rectCount; ++i) {
        // X 방향으로만 이동 (직선)
        showingrect[i].x1 += zigzagSpeedX[i];
        showingrect[i].x2 += zigzagSpeedX[i];
        
        // 경계 충돌 검사 및 처리 (500x500 윈도우)
        // X 방향 경계 충돌 - 방향 반전 + Y 좌표 변경
        if (showingrect[i].x1 <= 0) {
            // 현재 사각형의 실제 폭과 높이 계산
            GLdouble rectWidth = showingrect[i].x2 - showingrect[i].x1;
            GLdouble rectHeight = showingrect[i].y2 - showingrect[i].y1;
            
            showingrect[i].x1 = 0;
            showingrect[i].x2 = rectWidth; // 실제 폭 유지
            zigzagSpeedX[i] = -zigzagSpeedX[i]; // X 방향 반전
            
            // Y 좌표 랜덤하게 변경 (지그재그 효과)
            std::uniform_real_distribution<GLdouble> yChangeDis(-ZIGZAG_Y_CHANGE, ZIGZAG_Y_CHANGE);
            GLdouble yChange = yChangeDis(gen);
            
            showingrect[i].y1 += yChange;
            showingrect[i].y2 += yChange;
            
            // Y 경계 체크 후 조정
            if (showingrect[i].y1 < 0) {
                GLdouble overflow = -showingrect[i].y1;
                showingrect[i].y1 = 0;
                showingrect[i].y2 = rectHeight; // 실제 높이 유지
            }
            else if (showingrect[i].y2 > 500) {
                GLdouble overflow = showingrect[i].y2 - 500;
                showingrect[i].y2 = 500;
                showingrect[i].y1 = 500 - rectHeight; // 실제 높이 유지
            }
        }
        else if (showingrect[i].x2 >= 500) {
            // 현재 사각형의 실제 폭과 높이 계산
            GLdouble rectWidth = showingrect[i].x2 - showingrect[i].x1;
            GLdouble rectHeight = showingrect[i].y2 - showingrect[i].y1;
            
            showingrect[i].x2 = 500;
            showingrect[i].x1 = 500 - rectWidth; // 실제 폭 유지
            zigzagSpeedX[i] = -zigzagSpeedX[i]; // X 방향 반전
            
            // Y 좌표 랜덤하게 변경 (지그재그 효과)
            std::uniform_real_distribution<GLdouble> yChangeDis(-ZIGZAG_Y_CHANGE, ZIGZAG_Y_CHANGE);
            GLdouble yChange = yChangeDis(gen);
            
            showingrect[i].y1 += yChange;
            showingrect[i].y2 += yChange;
            
            // Y 경계 체크 후 조정
            if (showingrect[i].y1 < 0) {
                GLdouble overflow = -showingrect[i].y1;
                showingrect[i].y1 = 0;
                showingrect[i].y2 = rectHeight; // 실제 높이 유지
            }
            else if (showingrect[i].y2 > 500) {
                GLdouble overflow = showingrect[i].y2 - 500;
                showingrect[i].y2 = 500;
                showingrect[i].y1 = 500 - rectHeight; // 실제 높이 유지
            }
        }
    }
}

// 형태 변화 애니메이션 업데이트 함수
void UpdateMorphAnimation() {
    if (!morphMode) return;
    
    for (int i = 0; i < rectCount; ++i) {
        // 원래 좌표 저장
        GLdouble originalX1 = showingrect[i].x1;
        GLdouble originalY1 = showingrect[i].y1;
        GLdouble originalX2 = showingrect[i].x2;
        GLdouble originalY2 = showingrect[i].y2;
        
        // -5에서 5 사이의 난수로 각 좌표 변경
        showingrect[i].x1 += morphDis(gen);
        showingrect[i].y1 += morphDis(gen);
        showingrect[i].x2 += morphDis(gen);
        showingrect[i].y2 += morphDis(gen);
        
        // 좌표 정규화: 더 작은 값이 x1, y1에 오도록
        if (showingrect[i].x1 > showingrect[i].x2) {
            std::swap(showingrect[i].x1, showingrect[i].x2);
        }
        if (showingrect[i].y1 > showingrect[i].y2) {
            std::swap(showingrect[i].y1, showingrect[i].y2);
        }
        
        // 윈도우 경계 체크 및 조정 (500x500)
        // X 좌표 경계 체크
        if (showingrect[i].x1 < 0) {
            GLdouble offset = -showingrect[i].x1;
            showingrect[i].x1 = 0;
            showingrect[i].x2 += offset;
        }
        if (showingrect[i].x2 > 500) {
            GLdouble offset = showingrect[i].x2 - 500;
            showingrect[i].x2 = 500;
            showingrect[i].x1 -= offset;
            if (showingrect[i].x1 < 0) {
                showingrect[i].x1 = 0;
            }
        }
        
        // Y 좌표 경계 체크
        if (showingrect[i].y1 < 0) {
            GLdouble offset = -showingrect[i].y1;
            showingrect[i].y1 = 0;
            showingrect[i].y2 += offset;
        }
        if (showingrect[i].y2 > 500) {
            GLdouble offset = showingrect[i].y2 - 500;
            showingrect[i].y2 = 500;
            showingrect[i].y1 -= offset;
            if (showingrect[i].y1 < 0) {
                showingrect[i].y1 = 0;
            }
        }
        
        // 최소 크기 보장 (너무 작아지지 않도록)
        if (showingrect[i].x2 - showingrect[i].x1 < 5) {
            GLdouble centerX = (showingrect[i].x1 + showingrect[i].x2) / 2;
            showingrect[i].x1 = centerX - 2.5;
            showingrect[i].x2 = centerX + 2.5;
        }
        if (showingrect[i].y2 - showingrect[i].y1 < 5) {
            GLdouble centerY = (showingrect[i].y1 + showingrect[i].y2) / 2;
            showingrect[i].y1 = centerY - 2.5;
            showingrect[i].y2 = centerY + 2.5;
        }
    }
}

// 색상 변화 애니메이션 업데이트 함수
void UpdateColorAnimation() {
    if (!colorMode) return;
    
    for (int i = 0; i < rectCount; ++i) {
        // 각 사각형의 색상을 지속적으로 변경
        showingrect[i].Rvalue = colorDis(gen);
        showingrect[i].Gvalue = colorDis(gen);
        showingrect[i].Bvalue = colorDis(gen);
    }
}

// 타이머 콜백 (glutTimerFunc 등에서 호출될 함수 예시)
void TimerFunc(int value) {
    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        glutTimerFunc(1000, TimerFunc, 0); // 1초마다 갱신
    }
    
    // 따라가기 애니메이션 업데이트
    if (followMode) {
        UpdateFollowAnimation();
    }
    
    // 대각선 튕기기 애니메이션 업데이트
    if (bounceMode) {
        UpdateBounceAnimation();
    }
    
    // 지그재그 애니메이션 업데이트
    if (zigzagMode) {
        UpdateZigzagAnimation();
    }
    
    // 형태 변화 애니메이션 업데이트
    if (morphMode) {
        UpdateMorphAnimation();
    }
    
    // 색상 변화 애니메이션 업데이트
    if (colorMode) {
        UpdateColorAnimation();
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunc, 0); // 60 FPS로 계속 업데이트
}

void Motion(int x, int y)
{
    if (followMode && selectedRect != -1) {
        targetX = x;
        targetY = y;
    }
}

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

    // 처음에는 사각형을 생성하지 않음
    rectCount = 0;
    
    // 배열 초기화 (빈 상태로)
    for (int i = 0; i < 5; ++i) {
        showingrect[i].x1 = -1; // 사용하지 않는 상태 표시
        previousPos[i].x1 = -1; // 이전 위치도 초기화
        velocityX[i] = 0.0; // 속도 초기화
        velocityY[i] = 0.0;
        zigzagSpeedX[i] = 0.0; // 지그재그 변수 초기화
        zigzagYOffset[i] = 0.0;
    }

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    
    // 애니메이션용 타이머 시작
    glutTimerFunc(16, TimerFunc, 0); // 60 FPS
    
    glutMainLoop();
}

GLvoid drawScene()
{
    glClearColor(Rvalue, Gvalue, Bvalue, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPolygonMode(GL_BACK, GL_FILL);
    
    // 실제로 생성된 사각형만 그리기
    for (int i = 0; i < rectCount; ++i) {
        ret morphed;
        morph(morphed, showingrect[i]);
        
        // 선택된 사각형은 다른 색으로 표시
        if (i == selectedRect && followMode) {
            glColor3f(1.0f, 1.0f, 1.0f); // 흰색으로 강조
        } else {
            glColor3f(showingrect[i].Rvalue, showingrect[i].Gvalue, showingrect[i].Bvalue);
        }
        
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
    case '1': // 대각선 이동 및 튕기기 모드
        // 다른 모드 해제 (2번, 3번, 4번 모드 포함)
        followMode = false;
        selectedRect = -1;
        zigzagMode = false; // 2번 모드 해제
        morphMode = false;  // 3번 모드 해제
        colorMode = false;  // 4번 모드 해제
        
        // 튕기기 모드 토글
        bounceMode = !bounceMode;
        
        if (bounceMode) {
            // 각 사각형에 랜덤한 대각선 속도 부여
            std::uniform_real_distribution<GLdouble> speedDis(-BOUNCE_SPEED, BOUNCE_SPEED);
            for (int i = 0; i < rectCount; ++i) {
                velocityX[i] = speedDis(gen);
                velocityY[i] = speedDis(gen);
                
                // 속도가 0에 너무 가깝다면 최소 속도 보장
                if (abs(velocityX[i]) < 1.0) {
                    velocityX[i] = (velocityX[i] >= 0) ? 1.0 : -1.0;
                }
                if (abs(velocityY[i]) < 1.0) {
                    velocityY[i] = (velocityY[i] >= 0) ? 1.0 : -1.0;
                }
            }
        }
        break;
    case '2': // 지그재그 이동 모드
        // 다른 모드 해제 (1번, 3번, 4번 모드 포함)
        followMode = false;
        selectedRect = -1;
        bounceMode = false; // 1번 모드 해제
        morphMode = false;  // 3번 모드 해제
        colorMode = false;  // 4번 모드 해제
        
        // 지그재그 모드 토글
        zigzagMode = !zigzagMode;
        
        if (zigzagMode) {
            // 각 사각형에 랜덤한 지그재그 패턴 부여
            std::uniform_real_distribution<GLdouble> speedDis(-ZIGZAG_BASE_SPEED, ZIGZAG_BASE_SPEED);
            
            for (int i = 0; i < rectCount; ++i) {
                zigzagSpeedX[i] = speedDis(gen);
                zigzagYOffset[i] = 0.0; // Y 오프셋 초기화
                
                // X 방향 최소 속도 보장
                if (abs(zigzagSpeedX[i]) < 0.5) {
                    zigzagSpeedX[i] = (zigzagSpeedX[i] >= 0) ? 0.5 : -0.5;
                }
            }
        }
        break;
    case '3': // 형태 변화 모드
        // 다른 모드 해제 (1번, 2번, 4번 모드 포함)
        followMode = false;
        selectedRect = -1;
        bounceMode = false;  // 1번 모드 해제
        zigzagMode = false;  // 2번 모드 해제
        colorMode = false;   // 4번 모드 해제
        
        // 형태 변화 모드 토글
        morphMode = !morphMode;
        break;
    case '4': // 색상 변화 모드
        // 다른 모드 해제 (1번, 2번, 3번 모드 포함)
        followMode = false;
        selectedRect = -1;
        bounceMode = false;  // 1번 모드 해제
        zigzagMode = false;  // 2번 모드 해제
        morphMode = false;   // 3번 모드 해제
        
        // 색상 변화 모드 토글
        colorMode = !colorMode;
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
            // 1) 기존 사각형 클릭 검사 (선택 및 따라가기)
            for (int i = 0; i < rectCount; ++i) {
                if (ptinrect(x, y, showingrect[i])) {
                    selectedRect = i;
                    followMode = true;
                    targetX = x;
                    targetY = y;
                    glutPostRedisplay();
                    return;
                }
            }
            
            // 2) 빈 공간 클릭 시 새 사각형 생성
            if (rectCount < 5) {
                showingrect[rectCount].x1 = x - rectspace/2;
                showingrect[rectCount].y1 = y - rectspace/2;
                showingrect[rectCount].x2 = x + rectspace/2;
                showingrect[rectCount].y2 = y + rectspace/2;
                showingrect[rectCount].Rvalue = dis(gen) / 256.0f;
                showingrect[rectCount].Gvalue = dis(gen) / 256.0f;
                showingrect[rectCount].Bvalue = dis(gen) / 256.0f;
                
                // 대각선 이동 모드가 켜져있으면 새 사각형에도 랜덤 속도 부여
                if (bounceMode) {
                    std::uniform_real_distribution<GLdouble> speedDis(-BOUNCE_SPEED, BOUNCE_SPEED);
                    velocityX[rectCount] = speedDis(gen);
                    velocityY[rectCount] = speedDis(gen);
                    
                    // 속도가 0에 너무 가깝다면 최소 속도 보장
                    if (abs(velocityX[rectCount]) < 1.0) {
                        velocityX[rectCount] = (velocityX[rectCount] >= 0) ? 1.0 : -1.0;
                    }
                    if (abs(velocityY[rectCount]) < 1.0) {
                        velocityY[rectCount] = (velocityY[rectCount] >= 0) ? 1.0 : -1.0;
                    }
                }
                // 지그재그 모드가 켜져있으면 새 사각형에도 지그재그 패턴 부여
                else if (zigzagMode) {
                    std::uniform_real_distribution<GLdouble> speedDis(-ZIGZAG_BASE_SPEED, ZIGZAG_BASE_SPEED);
                    
                    zigzagSpeedX[rectCount] = speedDis(gen);
                    zigzagYOffset[rectCount] = 0.0; // Y 오프셋 초기화
                    
                    if (abs(zigzagSpeedX[rectCount]) < 0.5) {
                        zigzagSpeedX[rectCount] = (zigzagSpeedX[rectCount] >= 0) ? 0.5 : -0.5;
                    }
                }
                // 형태 변화 모드가 켜져있으면 새 사각형도 형태 변화 적용 (특별한 초기화 필요 없음)
                else if (morphMode) {
                    // 형태 변화 모드는 특별한 초기화가 필요하지 않음
                    // UpdateMorphAnimation에서 자동으로 처리됨
                }
                // 색상 변화 모드가 켜져있으면 새 사각형도 색상 변화 적용 (특별한 초기화 필요 없음)
                else if (colorMode) {
                    // 색상 변화 모드는 특별한 초기화가 필요하지 않음
                    // UpdateColorAnimation에서 자동으로 처리됨
                } 
                else {
                    // 모든 모드가 꺼져있으면 속도를 0으로 설정
                    velocityX[rectCount] = 0.0;
                    velocityY[rectCount] = 0.0;
                    zigzagSpeedX[rectCount] = 0.0;
                    zigzagYOffset[rectCount] = 0.0;
                }
                
                rectCount++;
                glutPostRedisplay();
            }
        }
        else if (state == GLUT_UP) {
           
        }
    }
    break;
    case GLUT_RIGHT_BUTTON:
    {
        if (state == GLUT_DOWN) {
            // 우클릭으로 따라가기 모드 해제
            followMode = false;
            selectedRect = -1;
            glutPostRedisplay();
        }
    }
    break;
    default:
        break;

    }
}

bool isColliding(const ret& a, const ret& b) {
    return !(a.x2 <= b.x1 || // A가 B의 왼쪽에 있음
        a.x1 >= b.x2 || // A가 B의 오른쪽에 있음
        a.y2 <= b.y1 || // A가 B의 아래에 있음
        a.y1 >= b.y2);  // A가 B의 위에 있음
}