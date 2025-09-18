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
std::uniform_int_distribution<int> splitDis(1, 4);      // ���� Ÿ�� (1~4)
std::uniform_int_distribution<int> directionDis(0, 7);  // �̵� ���� (0~7)

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
    
    // ���� �߰��Ǵ� ������
    int splitType = 1;   // ���� ���� (1:�����¿�, 2:�밢��, 3:�ȹ�, 4:�ѹ���)
    int moveDirection = 0; // 4�� ������ �� �̵� ���� (0~7: �ȹ�)
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
        // Ŭ���� ���� ���� ã��
        for (int i = 0; i < 10; i++) {
            rect_group& group = rectGroups[i];
            
            if (!group.isActive) continue;
            
            // 8�� ���� �� �ϳ��� Ŭ���Ǹ� ��ü �׷��� �������� �̵� ����
            for (int j = 0; j < 8; j++) {
                if (!group.pieces[j].isActive) continue;
                
                if (ptinrect(x, y, group.pieces[j])) {
                    // �ֿܼ� Ŭ���� ���� ���� ���
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
                    
                    // �̵� ����
                    Split8Rectangle(i);
                    glutPostRedisplay();
                    return;
                }
            }
        }
        
        // Ŭ�������� ������ ���� ���
        std::cout << "No piece clicked at (" << x << "," << y << ")" << std::endl;
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
        
        // ���� �簢�� ���� (���ؿ�)
        group.mainRect.x1 = numdis(gen);
        group.mainRect.y1 = numdis(gen);
        group.mainRect.x2 = group.mainRect.x1 + rectspace;
        group.mainRect.y2 = group.mainRect.y1 + rectspace;
        group.mainRect.Rvalue = dis(gen) / 256.0f;
        group.mainRect.Gvalue = dis(gen) / 256.0f;
        group.mainRect.Bvalue = dis(gen) / 256.0f;
        group.mainRect.isActive = true;
        
        // �׷� �ʱ� ����
        group.isActive = true;
        group.splitType = splitDis(gen);        // 1~4 �� ����
        group.moveDirection = directionDis(gen); // 0~7 �� ����
        
        // 8�� �������� ���� ��ġ�� 8���� ��Ģ���� ��ġ
        InitializePieces(i);
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

    // ��� �簢�� �׷��� 8�� ������ �׸��� (���� �簢���� �׸��� ����)
    for (int i = 0; i < 10; i++) {
        rect_group& group = rectGroups[i];
        
        if (!group.isActive) continue;
        
        // �׻� 8���� ���� �����鸸 �׸���
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
            
            // ���� �簢�� �缳�� (���ؿ�)
            group.mainRect.x1 = numdis(gen);
            group.mainRect.y1 = numdis(gen);
            group.mainRect.x2 = group.mainRect.x1 + rectspace;
            group.mainRect.y2 = group.mainRect.y1 + rectspace;
            group.mainRect.Rvalue = dis(gen) / 256.0f;
            group.mainRect.Gvalue = dis(gen) / 256.0f;
            group.mainRect.Bvalue = dis(gen) / 256.0f;
            group.mainRect.isActive = true;
            
            // �׷� ���� ����
            group.isActive = true;
            group.splitType = splitDis(gen);
            group.moveDirection = directionDis(gen);
            
            // 8�� �������� �� ��ġ�� 8���� ��Ģ���� ���ġ
            InitializePieces(i);
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

// 8���� �Լ� - splitType�� ���� �ٸ� �������� ����
void Split8Rectangle(int groupIndex) {
    if (groupIndex < 0 || groupIndex >= 10) return;
    if (!rectGroups[groupIndex].isActive) return;
    
    rect_group& group = rectGroups[groupIndex];
    ret& original = group.mainRect;
    
    // �̵� ���� ���� ���
    std::cout << ">>> Starting movement for Rectangle " << groupIndex << " <<<" << std::endl;
    
    // �� ������ �ڽ��� ��ġ���� �ٱ������� �̵��ϴ� ���� ����
    const GLdouble directions[8][2] = {
        {-1.4, -1.4}, // 0: �»� �� �»�밢������
        {-1.4, 1.4},  // 1: ���� �� ���ϴ밢������
        {1.4, -1.4},  // 2: ��� �� ���밢������  
        {1.4, 1.4},   // 3: ���� �� ���ϴ밢������
        {-2, 0},      // 4: ���� �߰� �� ��������
        {0, -2},      // 5: ��� �߰� �� ����
        {0, 2},       // 6: �ϴ� �߰� �� �Ʒ���
        {2, 0}        // 7: ���� �߰� �� ����������
    };
    
    // ��� ������ ������ �����ϰ� ����
    for (int i = 0; i < 8; i++) {
        group.pieces[i].Rvalue = original.Rvalue;
        group.pieces[i].Gvalue = original.Gvalue;
        group.pieces[i].Bvalue = original.Bvalue;
        group.pieces[i].lifeTime = 0;
    }
    
    switch (group.splitType) {
        case 1: { // �����¿� - ��� ���� �̵��ϵ�, 0~3���� ���
            std::cout << "All pieces move, but only show 0~3 (corners)" << std::endl;
            
            // ��� ����(0~7��)�� �ش� ��ġ�� �´� �̵� �ӵ� �ο�
            for (int i = 0; i < 8; i++) {
                group.pieces[i].velocityX = directions[i][0];
                group.pieces[i].velocityY = directions[i][1];
            }
            
            // 4~7�� ������ ������ �ʰ� (�̵��� �ϵ� ȭ�鿡 ǥ�� ����)
            for (int i = 4; i < 8; i++) {
                group.pieces[i].isActive = false;
            }
            break;
        }
        case 2: { // �밢�� - ��� ���� �̵��ϵ�, 4~7���� ���
            std::cout << "All pieces move, but only show 4~7 (middles)" << std::endl;
            
            // ��� ����(0~7��)�� �ش� ��ġ�� �´� �̵� �ӵ� �ο�
            for (int i = 0; i < 8; i++) {
                group.pieces[i].velocityX = directions[i][0];
                group.pieces[i].velocityY = directions[i][1];
            }
            
            // 0~3�� ������ ������ �ʰ� (�̵��� �ϵ� ȭ�鿡 ǥ�� ����)
            for (int i = 0; i < 4; i++) {
                group.pieces[i].isActive = false;
            }
            break;
        }
        case 3: { // �ȹ����� - ��� 8�� �簢�� �̵��ϰ� ���
            std::cout << "All 8 pieces move and show" << std::endl;
            
            // ��� ������ �ش� ��ġ���� �ٱ������� �̵�
            for (int i = 0; i < 8; i++) {
                group.pieces[i].velocityX = directions[i][0];
                group.pieces[i].velocityY = directions[i][1];
            }
            break;
        }
        case 4: { // 0~3�� �簢���� �ѹ������� - ��� ���� �̵��ϵ�, 0~3���� ���
            const char* dirNames[] = {"Up-Left", "Down-Left", "Up-Right", "Down-Right", 
                                    "Left", "Up", "Down", "Right"};
            std::cout << "All pieces move in their respective directions, but only show 0~3" << std::endl;
            
            // Type 4������ ��� ������ ������ �������� �̵�
            GLdouble moveX = directions[group.moveDirection % 8][0];
            GLdouble moveY = directions[group.moveDirection % 8][1];
            
            // ��� ������ ���� �������� �̵�
            for (int i = 0; i < 8; i++) {
                group.pieces[i].velocityX = moveX;
                group.pieces[i].velocityY = moveY;
            }
            
            // 4~7�� ������ ������ �ʰ� (�̵��� �ϵ� ȭ�鿡 ǥ�� ����)
            for (int i = 4; i < 8; i++) {
                group.pieces[i].isActive = false;
            }
            break;
        }
    }
    
    std::cout << "Movement started!" << std::endl << std::endl;
}

// 8���� ���� �������� ���� �簢�� ��ġ�� ��ġ�ϴ� �Լ�
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
    
    // ��� ������ ������ ������ �������� ����
    for (int i = 0; i < 8; i++) {
        group.pieces[i] = original;
        group.pieces[i].isActive = true;
        group.pieces[i].lifeTime = 0;
        group.pieces[i].velocityX = 0.0;
        group.pieces[i].velocityY = 0.0;
        
        // ��� ������ ������ ������ �������� ����
        group.pieces[i].Rvalue = original.Rvalue;
        group.pieces[i].Gvalue = original.Gvalue;
        group.pieces[i].Bvalue = original.Bvalue;
    }
    
    // === ����� 8���� ��Ģ�� ���� ��ġ ===
    
    // 4�� �⺻ ���� (2x2 ����)
    // �»� (0,0) ~ (10,10)
    group.pieces[0].x1 = x1;
    group.pieces[0].y1 = y1;
    group.pieces[0].x2 = x1 + halfWidth;
    group.pieces[0].y2 = y1 + halfHeight;
    
    // ���� (0,10) ~ (10,20)
    group.pieces[1].x1 = x1;
    group.pieces[1].y1 = y1 + halfHeight;
    group.pieces[1].x2 = x1 + halfWidth;
    group.pieces[1].y2 = y2;
    
    // ��� (10,0) ~ (20,10)
    group.pieces[2].x1 = x1 + halfWidth;
    group.pieces[2].y1 = y1;
    group.pieces[2].x2 = x2;
    group.pieces[2].y2 = y1 + halfHeight;
    
    // ���� (10,10) ~ (20,20)
    group.pieces[3].x1 = x1 + halfWidth;
    group.pieces[3].y1 = y1 + halfHeight;
    group.pieces[3].x2 = x2;
    group.pieces[3].y2 = y2;
    
    // === 4�� �߰��� �簢�� (��ġ�� �κ�) ===
    // (0,5) ~ (10,15) - ���� �߰� ������
    group.pieces[4].x1 = x1;
    group.pieces[4].y1 = y1 + quarterHeight;
    group.pieces[4].x2 = x1 + halfWidth;
    group.pieces[4].y2 = y2 - quarterHeight;
    
    // (5,0) ~ (15,10) - ���� �߰� ������
    group.pieces[5].x1 = x1 + quarterWidth;
    group.pieces[5].y1 = y1;
    group.pieces[5].x2 = x2 - quarterWidth;
    group.pieces[5].y2 = y1 + halfHeight;
    
    // (5,10) ~ (15,20) - ���� �߰�, ���� �ϴ�
    group.pieces[6].x1 = x1 + quarterWidth;
    group.pieces[6].y1 = y1 + halfHeight;
    group.pieces[6].x2 = x2 - quarterWidth;
    group.pieces[6].y2 = y2;
    
    // (10,5) ~ (20,15) - ����, ���� �߰�
    group.pieces[7].x1 = x1 + halfWidth;
    group.pieces[7].y1 = y1 + quarterHeight;
    group.pieces[7].x2 = x2;
    group.pieces[7].y2 = y2 - quarterHeight;
    
    // �׷��� "�׻� ���ҵ� ����"�� ����
    group.isSplit = true;
}

// �ִϸ��̼� ������Ʈ
void UpdateAnimation() {
    for (int i = 0; i < 10; i++) {
        rect_group& group = rectGroups[i];
        
        if (!group.isActive) continue;
        
        bool hasActivePiece = false;
        
        // 8���� ������ �ִϸ��̼� ó��
        for (int j = 0; j < 8; j++) {
            ret& piece = group.pieces[j];
            
            if (!piece.isActive) continue;
            
            // �̵� ���� �����鸸 ��ġ ������Ʈ �� ���
            if (abs(piece.velocityX) > 0.01 || abs(piece.velocityY) > 0.01) {
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
                
                GLdouble newWidth = currentWidth * 0.96;  // 4% ���
                GLdouble newHeight = currentHeight * 0.96;
                
                piece.x1 = centerX - newWidth / 2;
                piece.x2 = centerX + newWidth / 2;
                piece.y1 = centerY - newHeight / 2;
                piece.y2 = centerY + newHeight / 2;
                
                piece.lifeTime++;
                
                // ���� ����: ũ�Ⱑ 5 ���� �Ǵ� 300������(5��) ���
                if (abs(piece.x2 - piece.x1) <= 5 || abs(piece.y2 - piece.y1) <= 5 || piece.lifeTime > 300) {
                    piece.isActive = false;
                } else {
                    hasActivePiece = true;
                }
            } else {
                // �̵����� �ʴ� �������� �״�� ����
                hasActivePiece = true;
            }
        }
        
        // ��� ������ ��Ȱ��ȭ�Ǹ� �׷� ��ü�� ��Ȱ��ȭ (rŰ�θ� �����)
        if (!hasActivePiece) {
            std::cout << "All pieces in group " << i << " disappeared" << std::endl;
            group.isActive = false;
        }
    }
}