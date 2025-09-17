#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>
#include <algorithm>
#include <cmath>

#define rectspace 30

std::random_device rd;

// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::mt19937 gen(rd());

static int mousexstart, mouseystart;
static int mousexend, mouseyend;

// 0 ���� 99 ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
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
int rectCount = 0;          // ���� ������ �簢�� ����
int selectedRect = -1;      // ���õ� �簢�� �ε��� (-1�̸� ���� ����)
bool followMode = false;    // ���󰡱� ��� ����
GLdouble targetX, targetY;  // ��ǥ ��ġ

// ���� ��ġ ����� �迭 (�����̵��� ����)
ret previousPos[5];

// �밢�� �̵� �� ƨ��� ���� ����
bool bounceMode = false;    // ƨ��� ��� ����
GLdouble velocityX[5];      // �� �簢���� X ���� �ӵ�
GLdouble velocityY[5];      // �� �簢���� Y ���� �ӵ�
const GLdouble BOUNCE_SPEED = 3.0; // �̵� �ӵ�

// ������� �̵� ���� ����
bool zigzagMode = false;    // ������� ��� ����
GLdouble zigzagSpeedX[5];   // ������� X ���� �⺻ �ӵ�
GLdouble zigzagSpeedY[5];   // ������� Y ���� �⺻ �ӵ� (�Ʒ�/���� �̵�)
const GLdouble ZIGZAG_BASE_SPEED = 5.0; // �⺻ �̵� �ӵ� (X��)
const GLdouble ZIGZAG_Y_SPEED = 10.0;   // Y ���� �⺻ �ӵ� (������ 10.0!)

// ���� ��ȭ ���� ����
bool morphMode = false;     // ���� ��ȭ ��� ����
std::uniform_real_distribution<GLdouble> morphDis(-5.0, 5.0); // -5���� 5 ���� ����

// ���� ��ȭ ���� ����
bool colorMode = false;     // ���� ��ȭ ��� ����
std::uniform_real_distribution<GLdouble> colorDis(0.0, 1.0); // 0.0���� 1.0 ���� ����

// ���� �ִϸ��̼� �����ϱ� ���� ����
bool randomFollowMode = false;  // ���� �ִϸ��̼� �����ϱ� ��� ����
int randomSelectedRect = -1;    // �������� ���õ� �簢�� �ε���
ret previousTargetState;        // Ÿ�� �簢���� ���� ���� ���� (3�� ����)


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

bool isColliding(const ret& a, const ret& b);

bool ptinrect(int x, int y, ret& rect) {
    return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}

void Mouse(int button, int state, int x, int y);

// ���� �ִϸ��̼� �����ϱ� ������Ʈ �Լ�
void UpdateRandomFollowAnimation() {
    if (!randomFollowMode || randomSelectedRect == -1 || rectCount < 2) return;
    
    // ���� ��� �簢���� ��ġ�� ���� ��ġ�� ����
    for (int i = 0; i < rectCount; ++i) {
        previousPos[i] = showingrect[i];
    }
    
    // Ÿ�� �簢���� ���� ���� ���� (3�� ����)
    previousTargetState = showingrect[randomSelectedRect];
    
    // Ÿ�� �簢���� ���� Ȱ��ȭ�� �ִϸ��̼� ����
    if (bounceMode) {
        // 1�� ���: �밢�� ƨ��� (Ÿ�� �簢����, �ӵ� 3�� ����)
        showingrect[randomSelectedRect].x1 += velocityX[randomSelectedRect] * 5.0;
        showingrect[randomSelectedRect].y1 += velocityY[randomSelectedRect] * 5.0;
        showingrect[randomSelectedRect].x2 += velocityX[randomSelectedRect] * 5.0;
        showingrect[randomSelectedRect].y2 += velocityY[randomSelectedRect] * 5.0;
        
        // ��� �浹 ó�� - �ùٸ� ũ�� ���� ���
        GLdouble rectWidth = showingrect[randomSelectedRect].x2 - showingrect[randomSelectedRect].x1;
        GLdouble rectHeight = showingrect[randomSelectedRect].y2 - showingrect[randomSelectedRect].y1;
        
        if (showingrect[randomSelectedRect].x1 <= 0) {
            showingrect[randomSelectedRect].x2 = rectWidth;
            showingrect[randomSelectedRect].x1 = 0;
            velocityX[randomSelectedRect] = -velocityX[randomSelectedRect];
        }
        else if (showingrect[randomSelectedRect].x2 >= 500) {
            showingrect[randomSelectedRect].x1 = 500 - rectWidth;
            showingrect[randomSelectedRect].x2 = 500;
            velocityX[randomSelectedRect] = -velocityX[randomSelectedRect];
        }
        if (showingrect[randomSelectedRect].y1 <= 0) {
            showingrect[randomSelectedRect].y2 = rectHeight;
            showingrect[randomSelectedRect].y1 = 0;
            velocityY[randomSelectedRect] = -velocityY[randomSelectedRect];
        }
        else if (showingrect[randomSelectedRect].y2 >= 500) {
            showingrect[randomSelectedRect].y1 = 500 - rectHeight;
            showingrect[randomSelectedRect].y2 = 500;
            velocityY[randomSelectedRect] = -velocityY[randomSelectedRect];
        }
    }
    else if (zigzagMode) {
        // 2�� ���: ������� (Ÿ�� �簢����, 5�� ��� ��� ����)
        showingrect[randomSelectedRect].x1 += zigzagSpeedX[randomSelectedRect];
        showingrect[randomSelectedRect].x2 += zigzagSpeedX[randomSelectedRect];
        
        if (showingrect[randomSelectedRect].x1 <= 0 || showingrect[randomSelectedRect].x2 >= 500) {
            zigzagSpeedX[randomSelectedRect] = -zigzagSpeedX[randomSelectedRect];
            // 5�� ��忡���� Y ��ǥ ���� ���� - �⺻ ������� ���� ����
        }
    }
    else if (morphMode) {
        // 3�� ���: ���� ��ȭ (Ÿ�� �簢����)
        showingrect[randomSelectedRect].x1 += morphDis(gen);
        showingrect[randomSelectedRect].y1 += morphDis(gen);
        showingrect[randomSelectedRect].x2 += morphDis(gen);
        showingrect[randomSelectedRect].y2 += morphDis(gen);
        
        // ����ȭ
        if (showingrect[randomSelectedRect].x1 > showingrect[randomSelectedRect].x2) {
            std::swap(showingrect[randomSelectedRect].x1, showingrect[randomSelectedRect].x2);
        }
        if (showingrect[randomSelectedRect].y1 > showingrect[randomSelectedRect].y2) {
            std::swap(showingrect[randomSelectedRect].y1, showingrect[randomSelectedRect].y2);
        }
    }
    else if (colorMode) {
        // 4�� ���: ���� ��ȭ (Ÿ�� �簢����)
        showingrect[randomSelectedRect].Rvalue = colorDis(gen);
        showingrect[randomSelectedRect].Gvalue = colorDis(gen);
        showingrect[randomSelectedRect].Bvalue = colorDis(gen);
    }
    
    // ������ �簢������ Ÿ�� �簢���� �����ϱ�
    for (int i = 0; i < rectCount; ++i) {
        if (i != randomSelectedRect) {
            if (bounceMode || zigzagMode) {
                // 1,2�� ���: �����̵� ���󰡱� ��� (ü�� ����)
                // �� �簢���� ���� ũ�� ���
                GLdouble rectWidth = showingrect[i].x2 - showingrect[i].x1;
                GLdouble rectHeight = showingrect[i].y2 - showingrect[i].y1;
                
                // Ÿ�� �簢���� ���� �߽� ��ǥ (�ٷ� ���� ��ġ)
                GLdouble prevTargetCenterX = (previousPos[randomSelectedRect].x1 + previousPos[randomSelectedRect].x2) / 2;
                GLdouble prevTargetCenterY = (previousPos[randomSelectedRect].y1 + previousPos[randomSelectedRect].y2) / 2;
                
                // Ÿ���� �ٷ� ���� ��ǥ�� �����̵�
                showingrect[i].x1 = prevTargetCenterX - rectWidth / 2;
                showingrect[i].y1 = prevTargetCenterY - rectHeight / 2;
                showingrect[i].x2 = prevTargetCenterX + rectWidth / 2;
                showingrect[i].y2 = prevTargetCenterY + rectHeight / 2;
            }
            else if (morphMode) {
                // 3�� ���: ��ǥ ��ȭ�� �����ϱ�
                GLdouble dx1 = showingrect[randomSelectedRect].x1 - previousTargetState.x1;
                GLdouble dy1 = showingrect[randomSelectedRect].y1 - previousTargetState.y1;
                GLdouble dx2 = showingrect[randomSelectedRect].x2 - previousTargetState.x2;
                GLdouble dy2 = showingrect[randomSelectedRect].y2 - previousTargetState.y2;
                
                showingrect[i].x1 += dx1;
                showingrect[i].y1 += dy1;
                showingrect[i].x2 += dx2;
                showingrect[i].y2 += dy2;
                
                // ����ȭ
                if (showingrect[i].x1 > showingrect[i].x2) {
                    std::swap(showingrect[i].x1, showingrect[i].x2);
                }
                if (showingrect[i].y1 > showingrect[i].y2) {
                    std::swap(showingrect[i].y1, showingrect[i].y2);
                }
            }
            else if (colorMode) {
                // 4�� ���: ���� �����ϱ�
                showingrect[i].Rvalue = showingrect[randomSelectedRect].Rvalue;
                showingrect[i].Gvalue = showingrect[randomSelectedRect].Gvalue;
                showingrect[i].Bvalue = showingrect[randomSelectedRect].Bvalue;
            }
        }
    }
}

// ���󰡱� �ִϸ��̼� ������Ʈ �Լ� (�����̵� ���)
void UpdateFollowAnimation() {
    if (!followMode || selectedRect == -1) return;
    
    // ���� ��� �簢���� ��ġ�� ���� ��ġ�� ����
    for (int i = 0; i < rectCount; ++i) {
        previousPos[i] = showingrect[i];
    }
    
    // ���õ� �簢���� ��ǥ ��ġ�� �����̵�
    GLdouble currentCenterX = (showingrect[selectedRect].x1 + showingrect[selectedRect].x2) / 2;
    GLdouble currentCenterY = (showingrect[selectedRect].y1 + showingrect[selectedRect].y2) / 2;
    
    GLdouble dx = targetX - currentCenterX;
    GLdouble dy = targetY - currentCenterY;
    
    showingrect[selectedRect].x1 += dx;
    showingrect[selectedRect].y1 += dy;
    showingrect[selectedRect].x2 += dx;
    showingrect[selectedRect].y2 += dy;
    
    // ü��ó�� ����� �����̵� ���
    for (int i = 0; i < rectCount; ++i) {
        if (i != selectedRect) {
            int targetIndex = -1; // ���� ����� �ε���
            
            if (selectedRect == 0) {
                // 0���� ���õ� ���: 1���� 0����, 2���� 1����, 3���� 2����, 4���� 3���� ����
                if (i > 0) {
                    targetIndex = i - 1;
                }
            } else if (selectedRect == 1) {
                // 1���� ���õ� ���: 0���� 1����, 2���� 0����, 3���� 2����, 4���� 3���� ����
                if (i == 0) {
                    targetIndex = 1; // 0���� 1���� ����
                } else if (i == 2) {
                    targetIndex = 0; // 2���� 0���� ����
                } else if (i > 2) {
                    targetIndex = i - 1; // 3���� 2����, 4���� 3���� ����
                }
            } else if (selectedRect == 2) {
                // 2���� ���õ� ���: 1���� 2����, 0���� 1����, 3���� 2����, 4���� 3���� ����
                if (i == 1) {
                    targetIndex = 2; // 1���� 2���� ����
                } else if (i == 0) {
                    targetIndex = 1; // 0���� 1���� ����
                } else if (i == 3) {
                    targetIndex = 2; // 3���� 2���� ����
                } else if (i == 4) {
                    targetIndex = 3; // 4���� 3���� ����
                }
            } else if (selectedRect == 3) {
                // 3���� ���õ� ���: 2���� 3����, 1���� 2����, 0���� 1����, 4���� 3���� ����
                if (i == 2) {
                    targetIndex = 3; // 2���� 3���� ����
                } else if (i == 1) {
                    targetIndex = 2; // 1���� 2���� ����
                } else if (i == 0) {
                    targetIndex = 1; // 0���� 1���� ����
                } else if (i == 4) {
                    targetIndex = 3; // 4���� 3���� ����
                }
            } else if (selectedRect == 4) {
                // 4���� ���õ� ���: 3���� 4����, 2���� 3����, 1���� 2����, 0���� 1���� ����
                if (i == 3) {
                    targetIndex = 4; // 3���� 4���� ����
                } else if (i == 2) {
                    targetIndex = 3; // 2���� 3���� ����
                } else if (i == 1) {
                    targetIndex = 2; // 1���� 2���� ����
                } else if (i == 0) {
                    targetIndex = 1; // 0���� 1���� ����
                }
            }
            
            // ���� ����� �ְ�, �ش� ����� ��ȿ�� ���� ���� ���� ��
            if (targetIndex >= 0 && targetIndex < rectCount) {
                // �� Ÿ�̸� ���� ��ǥ �簢�� ��ġ�� �����̵�
                GLdouble prevCenterX = (previousPos[targetIndex].x1 + previousPos[targetIndex].x2) / 2;
                GLdouble prevCenterY = (previousPos[targetIndex].y1 + previousPos[targetIndex].y2) / 2;
                GLdouble currentCenterX = (showingrect[i].x1 + showingrect[i].x2) / 2;
                GLdouble currentCenterY = (showingrect[i].y1 + showingrect[i].y2) / 2;
                
                GLdouble fdx = prevCenterX - currentCenterX;
                GLdouble fdy = prevCenterY - currentCenterY;
                
                // �����̵�: ���� ��ġ�� �ٷ� �̵�
                showingrect[i].x1 += fdx;
                showingrect[i].y1 += fdy;
                showingrect[i].x2 += fdx;
                showingrect[i].y2 += fdy;
            }
        }
    }
}

// �밢�� �̵� �� ƨ��� ������Ʈ �Լ�
void UpdateBounceAnimation() {
    if (!bounceMode || randomFollowMode) return;  // ���� ����� ���� �������� ����
    
    for (int i = 0; i < rectCount; ++i) {
        // ���� ��ġ ������Ʈ
        showingrect[i].x1 += velocityX[i];
        showingrect[i].y1 += velocityY[i];
        showingrect[i].x2 += velocityX[i];
        showingrect[i].y2 += velocityY[i];
        
        // ��� �浹 �˻� �� ƨ��� (500x500 ������) - �ùٸ� ũ�� ���� ���
        GLdouble rectWidth = showingrect[i].x2 - showingrect[i].x1;
        GLdouble rectHeight = showingrect[i].y2 - showingrect[i].y1;
        
        // ���� ��� �浹
        if (showingrect[i].x1 <= 0) {
            showingrect[i].x2 = rectWidth;
            showingrect[i].x1 = 0;
            velocityX[i] = -velocityX[i]; // X ���� ����
        }
        // ������ ��� �浹
        else if (showingrect[i].x2 >= 500) {
            showingrect[i].x1 = 500 - rectWidth;
            showingrect[i].x2 = 500;
            velocityX[i] = -velocityX[i]; // X ���� ����
        }
        
        // ���� ��� �浹
        if (showingrect[i].y1 <= 0) {
            showingrect[i].y2 = rectHeight;
            showingrect[i].y1 = 0;
            velocityY[i] = -velocityY[i]; // Y ���� ����
        }
        // �Ʒ��� ��� �浹
        else if (showingrect[i].y2 >= 500) {
            showingrect[i].y1 = 500 - rectHeight;
            showingrect[i].y2 = 500;
            velocityY[i] = -velocityY[i]; // Y ���� ����
        }
    }
}

// ������� �ִϸ��̼� ������Ʈ �Լ� (������ ����)
void UpdateZigzagAnimation() {
    if (!zigzagMode || randomFollowMode) return;  // ���� ����� ���� �������� ����
    
    for (int i = 0; i < rectCount; ++i) {
        // X �������θ� �̵� (��������� �ٽ�)
        showingrect[i].x1 += zigzagSpeedX[i];
        showingrect[i].x2 += zigzagSpeedX[i];
        
        GLdouble rectWidth = showingrect[i].x2 - showingrect[i].x1;
        GLdouble rectHeight = showingrect[i].y2 - showingrect[i].y1;
        
        // X ���� ��� �浹 - ���� ���� + Y ��ǥ �Ʒ��� �̵�
        if (showingrect[i].x1 <= 0) {
            showingrect[i].x2 = rectWidth;
            showingrect[i].x1 = 0;
            zigzagSpeedX[i] = -zigzagSpeedX[i]; // X ���� ����
            
            // Y ��ǥ�� �Ʒ��� �̵� (������� ȿ��)
            showingrect[i].y1 += zigzagSpeedY[i];
            showingrect[i].y2 += zigzagSpeedY[i];
        }
        else if (showingrect[i].x2 >= 500) {
            showingrect[i].x1 = 500 - rectWidth;
            showingrect[i].x2 = 500;
            zigzagSpeedX[i] = -zigzagSpeedX[i]; // X ���� ����
            
            // Y ��ǥ�� �Ʒ��� �̵� (������� ȿ��)
            showingrect[i].y1 += zigzagSpeedY[i];
            showingrect[i].y2 += zigzagSpeedY[i];
        }
        
        // Y ���� ��� �浹 - Y ���� ���� (��/�Ʒ� ���� ������ ���� ��ȯ)
        if (showingrect[i].y1 <= 0) {
            showingrect[i].y2 = rectHeight;
            showingrect[i].y1 = 0;
            zigzagSpeedY[i] = abs(zigzagSpeedY[i]); // �Ʒ������� �̵��ϵ��� ����� ����
        }
        else if (showingrect[i].y2 >= 500) {
            showingrect[i].y1 = 500 - rectHeight;
            showingrect[i].y2 = 500;
            zigzagSpeedY[i] = -abs(zigzagSpeedY[i]); // �������� �̵��ϵ��� ������ ����
        }
    }
}

// ���� ��ȭ �ִϸ��̼� ������Ʈ �Լ�
void UpdateMorphAnimation() {
    if (!morphMode || randomFollowMode) return;  // ���� ����� ���� �������� ����
    
    for (int i = 0; i < rectCount; ++i) {
        // ���� ��ǥ ����
        GLdouble originalX1 = showingrect[i].x1;
        GLdouble originalY1 = showingrect[i].y1;
        GLdouble originalX2 = showingrect[i].x2;
        GLdouble originalY2 = showingrect[i].y2;
        
        // -5���� 5 ������ ������ �� ��ǥ ����
        showingrect[i].x1 += morphDis(gen);
        showingrect[i].y1 += morphDis(gen);
        showingrect[i].x2 += morphDis(gen);
        showingrect[i].y2 += morphDis(gen);
        
        // ��ǥ ����ȭ: �� ���� ���� x1, y1�� ������
        if (showingrect[i].x1 > showingrect[i].x2) {
            std::swap(showingrect[i].x1, showingrect[i].x2);
        }
        if (showingrect[i].y1 > showingrect[i].y2) {
            std::swap(showingrect[i].y1, showingrect[i].y2);
        }
        
        // ������ ��� üũ �� ���� (500x500)
        // X ��ǥ ��� üũ
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
        
        // Y ��ǥ ��� üũ
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
        
        // �ּ� ũ�� ���� (�ʹ� �۾����� �ʵ���)
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

// ���� ��ȭ �ִϸ��̼� ������Ʈ �Լ�
void UpdateColorAnimation() {
    if (!colorMode || randomFollowMode) return;  // ���� ����� ���� �������� ����
    
    for (int i = 0; i < rectCount; ++i) {
        // �� �簢���� ������ ���������� ����
        showingrect[i].Rvalue = colorDis(gen);
        showingrect[i].Gvalue = colorDis(gen);
        showingrect[i].Bvalue = colorDis(gen);
    }
}

// Ÿ�̸� �ݹ� (glutTimerFunc ��� ȣ��� �Լ� ����)
void TimerFunc(int value) {
    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        glutTimerFunc(1000, TimerFunc, 0); // 1�ʸ��� ����
    }
    
    // ���󰡱� �ִϸ��̼� ������Ʈ
    if (followMode) {
        UpdateFollowAnimation();
    }
    
    // ���� �ִϸ��̼� �����ϱ� ������Ʈ
    if (randomFollowMode) {
        UpdateRandomFollowAnimation();
    } else {
        // �Ϲ� �ִϸ��̼� ������Ʈ (���� ��� �ƴ� ����)
        if (bounceMode) {
            UpdateBounceAnimation();
        }
        
        if (zigzagMode) {
            UpdateZigzagAnimation();
        }
        
        if (morphMode) {
            UpdateMorphAnimation();
        }
        
        if (colorMode) {
            UpdateColorAnimation();
        }
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunc, 0); // 60 FPS�� ��� ������Ʈ
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

    // ó������ �簢���� �������� ����
    rectCount = 0;
    
    // �迭 �ʱ�ȭ (�� ���·�)
    for (int i = 0; i < 5; ++i) {
        showingrect[i].x1 = -1; // ������� �ʴ� ���� ǥ��
        previousPos[i].x1 = -1; // ���� ��ġ�� �ʱ�ȭ
        velocityX[i] = 0.0; // �ӵ� �ʱ�ȭ
        velocityY[i] = 0.0;
        zigzagSpeedX[i] = 0.0; // ������� ���� �ʱ�ȭ
        zigzagSpeedY[i] = 0.0; // ������� Y �ӵ� �ʱ�ȭ
    }

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    
    // �ִϸ��̼ǿ� Ÿ�̸� ����
    glutTimerFunc(16, TimerFunc, 0); // 60 FPS
    
    glutMainLoop();
}

GLvoid drawScene()
{
    glClearColor(Rvalue, Gvalue, Bvalue, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPolygonMode(GL_BACK, GL_FILL);
    
    // ������ ������ �簢���� �׸���
    for (int i = 0; i < rectCount; ++i) {
        ret morphed;
        morph(morphed, showingrect[i]);
        
        // ��� �簢���� �Ϲ� �������� ǥ�� (���� ǥ�� ���� ����)
        glColor3f(showingrect[i].Rvalue, showingrect[i].Gvalue, showingrect[i].Bvalue);
        
        glRectf(morphed.x1, morphed.y1, morphed.x2, morphed.y2);
    }
    
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
    case '1': // �밢�� �̵� �� ƨ��� ���
        // �ٸ� ��� ���� (2��, 3��, 4��, 5�� ��� ����)
        followMode = false;
        selectedRect = -1;
        zigzagMode = false; // 2�� ��� ����
        morphMode = false;  // 3�� ��� ����
        colorMode = false;  // 4�� ��� ����
        randomFollowMode = false;  // 5�� ��� ����
        randomSelectedRect = -1;
        
        // ƨ��� ��� ���
        bounceMode = !bounceMode;
        
        if (bounceMode) {
            // �� �簢���� ������ �밢�� �ӵ� �ο�
            std::uniform_real_distribution<GLdouble> speedDis(-BOUNCE_SPEED, BOUNCE_SPEED);
            for (int i = 0; i < rectCount; ++i) {
                velocityX[i] = speedDis(gen);
                velocityY[i] = speedDis(gen);
                
                // �ӵ��� 0�� �ʹ� �����ٸ� �ּ� �ӵ� ����
                if (abs(velocityX[i]) < 1.0) {
                    velocityX[i] = (velocityX[i] >= 0) ? 1.0 : -1.0;
                }
                if (abs(velocityY[i]) < 1.0) {
                    velocityY[i] = (velocityY[i] >= 0) ? 1.0 : -1.0;
                }
            }
        }
        break;
    case '2': // ������� �̵� ���
        // �ٸ� ��� ���� (1��, 3��, 4��, 5�� ��� ����)
        followMode = false;
        selectedRect = -1;
        bounceMode = false; // 1�� ��� ����
        morphMode = false;  // 3�� ��� ����
        colorMode = false;  // 4�� ��� ����
        randomFollowMode = false;  // 5�� ��� ����
        randomSelectedRect = -1;
        
        // ������� ��� ���
        zigzagMode = !zigzagMode;
        
        if (zigzagMode) {
            // �� �簢���� ������ ������� ���� �ο�
            std::uniform_real_distribution<GLdouble> speedDis(-ZIGZAG_BASE_SPEED, ZIGZAG_BASE_SPEED);
            
            for (int i = 0; i < rectCount; ++i) {
                zigzagSpeedX[i] = speedDis(gen);
                zigzagSpeedY[i] = ZIGZAG_Y_SPEED; // Y �ӵ��� ������ 10.0
                
                // X ���� �ּ� �ӵ� ����
                if (abs(zigzagSpeedX[i]) < 0.5) {
                    zigzagSpeedX[i] = (zigzagSpeedX[i] >= 0) ? 0.5 : -0.5;
                }
            }
        }
        break;
    case '3': // ���� ��ȭ ���
        // �ٸ� ��� ���� (1��, 2��, 4��, 5�� ��� ����)
        followMode = false;
        selectedRect = -1;
        bounceMode = false;  // 1�� ��� ����
        zigzagMode = false;  // 2�� ��� ����
        colorMode = false;   // 4�� ��� ����
        randomFollowMode = false;  // 5�� ��� ����
        randomSelectedRect = -1;
        
        // ���� ��ȭ ��� ���
        morphMode = !morphMode;
        break;
    case '4': // ���� ��ȭ ���
        // �ٸ� ��� ���� (1��, 2��, 3��, 5�� ��� ����)
        followMode = false;
        selectedRect = -1;
        bounceMode = false;  // 1�� ��� ����
        zigzagMode = false;  // 2�� ��� ����
        morphMode = false;   // 3�� ��� ����
        randomFollowMode = false;  // 5�� ��� ����
        randomSelectedRect = -1;
        
        // ���� ��ȭ ��� ���
        colorMode = !colorMode;
        break;
    case '5': // ���� �ִϸ��̼� �����ϱ� ���
        // 5�� ���� 1,2,3,4 �� �ϳ��� �����߸� �۵�
        if (bounceMode || zigzagMode || morphMode || colorMode) {
            followMode = false;
            selectedRect = -1;
            
            randomFollowMode = !randomFollowMode;
            
            if (randomFollowMode && rectCount > 1) {
                // �����ϰ� �簢�� ����
                std::uniform_int_distribution<int> rectDis(0, rectCount - 1);
                randomSelectedRect = rectDis(gen);
            } else {
                randomSelectedRect = -1;
            }
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
            // 1) ���� �簢�� Ŭ�� �˻� (���� �� ���󰡱�)
            for (int i = 0; i < rectCount; ++i) {
                if (ptinrect(x, y, showingrect[i])) {
                    selectedRect = i;
                    followMode = true;
                    // ���� ��� ����
                    randomFollowMode = false;
                    randomSelectedRect = -1;
                    targetX = x;
                    targetY = y;
                    glutPostRedisplay();
                    return;
                }
            }
            
            // 2) �� ���� Ŭ�� �� �� �簢�� ����
            if (rectCount < 5) {
                showingrect[rectCount].x1 = x - rectspace/2;
                showingrect[rectCount].y1 = y - rectspace/2;
                showingrect[rectCount].x2 = x + rectspace/2;
                showingrect[rectCount].y2 = y + rectspace/2;
                showingrect[rectCount].Rvalue = dis(gen) / 256.0f;
                showingrect[rectCount].Gvalue = dis(gen) / 256.0f;
                showingrect[rectCount].Bvalue = dis(gen) / 256.0f;
                
                // �밢�� �̵� ��尡 ���������� �� �簢������ ���� �ӵ� �ο�
                if (bounceMode) {
                    std::uniform_real_distribution<GLdouble> speedDis(-BOUNCE_SPEED, BOUNCE_SPEED);
                    velocityX[rectCount] = speedDis(gen);
                    velocityY[rectCount] = speedDis(gen);
                    
                    // �ӵ��� 0�� �ʹ� �����ٸ� �ּ� �ӵ� ����
                    if (abs(velocityX[rectCount]) < 1.0) {
                        velocityX[rectCount] = (velocityX[rectCount] >= 0) ? 1.0 : -1.0;
                    }
                    if (abs(velocityY[rectCount]) < 1.0) {
                        velocityY[rectCount] = (velocityY[rectCount] >= 0) ? 1.0 : -1.0;
                    }
                }
                // ������� ��尡 ���������� �� �簢������ ������� ���� �ο�
                else if (zigzagMode) {
                    std::uniform_real_distribution<GLdouble> speedDis(-ZIGZAG_BASE_SPEED, ZIGZAG_BASE_SPEED);
                    
                    zigzagSpeedX[rectCount] = speedDis(gen);
                    zigzagSpeedY[rectCount] = ZIGZAG_Y_SPEED; // Y �ӵ��� ������ 10.0
                
                    if (abs(zigzagSpeedX[rectCount]) < 0.5) {
                        zigzagSpeedX[rectCount] = (zigzagSpeedX[rectCount] >= 0) ? 0.5 : -0.5;
                    }
                }
                // ���� ��ȭ ��尡 ���������� �� �簢���� ���� ��ȭ ���� (Ư���� �ʱ�ȭ �ʿ� ����)
                else if (morphMode) {
                    // ���� ��ȭ ���� Ư���� �ʱ�ȭ�� �ʿ����� ����
                    // UpdateMorphAnimation���� �ڵ����� ó����
                }
                // ���� ��ȭ ��尡 ���������� �� �簢���� ���� ��ȭ ���� (Ư���� �ʱ�ȭ �ʿ� ����)
                else if (colorMode) {
                    // ���� ��ȭ ���� Ư���� �ʱ�ȭ�� �ʿ����� ����
                    // UpdateColorAnimation���� �ڵ����� ó����
                } 
                else {
                    // ��� ��尡 ���������� �ӵ��� 0���� ����
                    velocityX[rectCount] = 0.0;
                    velocityY[rectCount] = 0.0;
                    zigzagSpeedX[rectCount] = 0.0;
                    zigzagSpeedY[rectCount] = 0.0;
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
            // ��Ŭ������ ���󰡱� ��� ����
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
    return !(a.x2 <= b.x1 || // A�� B�� ���ʿ� ����
        a.x1 >= b.x2 || // A�� B�� �����ʿ� ����
        a.y2 <= b.y1 || // A�� B�� �Ʒ��� ����
        a.y1 >= b.y2);  // A�� B�� ���� ����
}