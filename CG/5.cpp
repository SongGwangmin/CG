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
    
    // �ִϸ��̼� �ʵ��
    GLdouble velocityX = 0.0;
    GLdouble velocityY = 0.0;
    bool isActive = true;
    int lifeTime = 0;
} ret;

// 8���� ret�� ��� ���ο� ����ü
typedef struct RECT_GROUP {
    ret mainRect;        // ���� �簢��
    ret pieces[8];       // 8���� ���ҵ� ������
    bool isSplit = false; // ���ҵǾ����� ����
    bool isActive = true; // ��ü �׷��� Ȱ�� ��������
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

// 10���� �簢�� �׷� ����
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
        // Ŭ���� �簢�� �׷� ã�� (���ҵ��� ���� ������)
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

// Ÿ�̸� �ݹ�
void TimerFunc(int value) {
    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
    }
    
    // �ִϸ��̼� ������Ʈ
    UpdateAnimation();
    
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunc, 0); // 60 FPS
}

void main(int argc, char** argv)
{
    //--- ������ �����ϱ�
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(500, 500);
    glutCreateWindow("8-Split Rectangle System");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "GLEW Initialized\n";
    
    // 10���� �簢�� �׷� �ʱ�ȭ
    for (int i = 0; i < 10; ++i) {
        rect_group& group = rectGroups[i];
        
        // ���� �簢�� ����
        group.mainRect.x1 = numdis(gen);
        group.mainRect.y1 = numdis(gen);
        group.mainRect.x2 = group.mainRect.x1 + rectspace;
        group.mainRect.y2 = group.mainRect.y1 + rectspace;
        group.mainRect.Rvalue = dis(gen) / 256.0f;
        group.mainRect.Gvalue = dis(gen) / 256.0f;
        group.mainRect.Bvalue = dis(gen) / 256.0f;
        group.mainRect.isActive = true;
        
        // �׷� �ʱ� ����
        group.isSplit = false;
        group.isActive = true;
        
        // 8�� ������ ��Ȱ��ȭ ���·� �ʱ�ȭ
        for (int j = 0; j < 8; j++) {
            group.pieces[j].isActive = false;
        }
    }
    
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    
    // �ִϸ��̼� Ÿ�̸� ����
    glutTimerFunc(16, TimerFunc, 0);
    
    glutMainLoop();
}

GLvoid drawScene()
{
    glClearColor(Rvalue, Gvalue, Bvalue, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ��� �簢�� �׷� �׸���
    for (int i = 0; i < 10; i++) {
        rect_group& group = rectGroups[i];
        
        if (!group.isActive) continue;
        
        if (!group.isSplit) {
            // ���ҵ��� ���� ���: ���� �簢�� �׸���
            ret drawsupport;
            drawsupport = morph(drawsupport, group.mainRect);
            glColor3f(drawsupport.Rvalue, drawsupport.Gvalue, drawsupport.Bvalue);
            glRectf(drawsupport.x1, drawsupport.y1, drawsupport.x2, drawsupport.y2);
        } else {
            // ���ҵ� ���: 8���� ������ �׸���
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

    case 't': // Ÿ�̸� ����/���� (���� ����)
        timerRunning = !timerRunning;
        break;

    case 'r': // ���� - ���� 10�� �簢������ ����
        for (int i = 0; i < 10; ++i) {
            rect_group& group = rectGroups[i];
            
            // ���� �簢�� �缳��
            group.mainRect.x1 = numdis(gen);
            group.mainRect.y1 = numdis(gen);
            group.mainRect.x2 = group.mainRect.x1 + rectspace;
            group.mainRect.y2 = group.mainRect.y1 + rectspace;
            group.mainRect.Rvalue = dis(gen) / 256.0f;
            group.mainRect.Gvalue = dis(gen) / 256.0f;
            group.mainRect.Bvalue = dis(gen) / 256.0f;
            group.mainRect.isActive = true;
            
            // �׷� ���� ����
            group.isSplit = false;
            group.isActive = true;
            
            // 8�� ������ ��Ȱ��ȭ
            for (int j = 0; j < 8; j++) {
                group.pieces[j].isActive = false;
            }
        }
        break;

    case 'q': // ���α׷� ����
        glutLeaveMainLoop();
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

// 8���� �Լ� - ����� ��Ģ��� ��Ȯ�� ����!
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
    
    // �׷��� ���� ���·� ����
    group.isSplit = true;
    
    // ��� ������ ���� ���� ����
    for (int i = 0; i < 8; i++) {
        group.pieces[i] = original; // ���� ���� ����
        group.pieces[i].isActive = true;
        group.pieces[i].lifeTime = 0;
    }
    
    // === 4�� �⺻ ���� (2x2 ����) ===
    // �»� (0,0) ~ (10,10)
    group.pieces[0].x1 = x1;
    group.pieces[0].y1 = y1;
    group.pieces[0].x2 = x1 + halfWidth;
    group.pieces[0].y2 = y1 + halfHeight;
    group.pieces[0].velocityX = -1.5; group.pieces[0].velocityY = -1.5;
    
    // ���� (0,10) ~ (10,20)
    group.pieces[1].x1 = x1;
    group.pieces[1].y1 = y1 + halfHeight;
    group.pieces[1].x2 = x1 + halfWidth;
    group.pieces[1].y2 = y2;
    group.pieces[1].velocityX = -1.5; group.pieces[1].velocityY = 1.5;
    
    // ��� (10,0) ~ (20,10)
    group.pieces[2].x1 = x1 + halfWidth;
    group.pieces[2].y1 = y1;
    group.pieces[2].x2 = x2;
    group.pieces[2].y2 = y1 + halfHeight;
    group.pieces[2].velocityX = 1.5; group.pieces[2].velocityY = -1.5;
    
    // ���� (10,10) ~ (20,20)
    group.pieces[3].x1 = x1 + halfWidth;
    group.pieces[3].y1 = y1 + halfHeight;
    group.pieces[3].x2 = x2;
    group.pieces[3].y2 = y2;
    group.pieces[3].velocityX = 1.5; group.pieces[3].velocityY = 1.5;
    
    // === 4�� �߰��� �簢�� ===
    // (0,5) ~ (10,15) - ���� �߰� ������
    group.pieces[4].x1 = x1;
    group.pieces[4].y1 = y1 + quarterHeight;
    group.pieces[4].x2 = x1 + halfWidth;
    group.pieces[4].y2 = y2 - quarterHeight;
    group.pieces[4].velocityX = -2.0; group.pieces[4].velocityY = 0;
    
    // (5,0) ~ (15,10) - ���� �߰� ������
    group.pieces[5].x1 = x1 + quarterWidth;
    group.pieces[5].y1 = y1;
    group.pieces[5].x2 = x2 - quarterWidth;
    group.pieces[5].y2 = y1 + halfHeight;
    group.pieces[5].velocityX = 0; group.pieces[5].velocityY = -2.0;
    
    // (5,10) ~ (15,20) - ���� �߰�, ���� �ϴ�
    group.pieces[6].x1 = x1 + quarterWidth;
    group.pieces[6].y1 = y1 + halfHeight;
    group.pieces[6].x2 = x2 - quarterWidth;
    group.pieces[6].y2 = y2;
    group.pieces[6].velocityX = 0; group.pieces[6].velocityY = 2.0;
    
    // (10,5) ~ (20,15) - ����, ���� �߰�
    group.pieces[7].x1 = x1 + halfWidth;
    group.pieces[7].y1 = y1 + quarterHeight;
    group.pieces[7].x2 = x2;
    group.pieces[7].y2 = y2 - quarterHeight;
    group.pieces[7].velocityX = 2.0; group.pieces[7].velocityY = 0;
}

// �ִϸ��̼� ������Ʈ
void UpdateAnimation() {
    for (int i = 0; i < 10; i++) {
        rect_group& group = rectGroups[i];
        
        if (!group.isActive || !group.isSplit) continue;
        
        bool hasActivePiece = false;
        
        // 8���� ������ �ִϸ��̼� ó��
        for (int j = 0; j < 8; j++) {
            ret& piece = group.pieces[j];
            
            if (!piece.isActive) continue;
            
            // ��ġ ������Ʈ
            piece.x1 += piece.velocityX;
            piece.y1 += piece.velocityY;
            piece.x2 += piece.velocityX;
            piece.y2 += piece.velocityY;
            
            // ũ�� ��� (�߽��� ����)
            GLdouble centerX = (piece.x1 + piece.x2) / 2;
            GLdouble centerY = (piece.y1 + piece.y2) / 2;
            GLdouble currentWidth = piece.x2 - piece.x1;
            GLdouble currentHeight = piece.y2 - piece.y1;
            
            GLdouble newWidth = currentWidth * 0.995;  // 0.5% ���
            GLdouble newHeight = currentHeight * 0.995;
            
            piece.x1 = centerX - newWidth / 2;
            piece.x2 = centerX + newWidth / 2;
            piece.y1 = centerY - newHeight / 2;
            piece.y2 = centerY + newHeight / 2;
            
            piece.lifeTime++;
            
            // ���� ����: ũ�Ⱑ 10 ���� �Ǵ� 600������(10��) ���
            if (abs(piece.x2 - piece.x1) <= 10 || abs(piece.y2 - piece.y1) <= 10 || piece.lifeTime > 600) {
                piece.isActive = false;
            } else {
                hasActivePiece = true;
            }
        }
        
        // ��� ������ ��Ȱ��ȭ�Ǹ� �׷� ��ü�� ��Ȱ��ȭ
        if (!hasActivePiece) {
            group.isActive = false;
        }
    }
    
    // ��Ȱ�� �簢�� ����
    for (auto& group : rectGroups) {
        if (!group.isActive) continue;
        
        // ���� �簢���� üũ
        if (!group.isSplit && !group.mainRect.isActive) {
            group.isActive = false;
        }
    }
    
    // ���������� ��Ȱ��ȭ�� �׷��� ����
    std::vector<rect_group> activeGroups;
    for (auto& group : rectGroups) {
        if (group.isActive) {
            activeGroups.push_back(group);
        }
    }
    std::copy(activeGroups.begin(), activeGroups.end(), rectGroups);
}