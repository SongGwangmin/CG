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
                // �ֿܼ� Ŭ���� �簢�� ���� ���
                std::cout << "=== Rectangle Clicked ===" << std::endl;
                std::cout << "Index: " << i << std::endl;
                std::cout << "Split Type: " << group.splitType;
                
                switch(group.splitType) {
                    case 1: std::cout << " (8-Split: 4 corner + 4 middle)"; break;
                    case 2: std::cout << " (4-Split: Diagonal)"; break; 
                    case 3: std::cout << " (8-Split: All directions)"; break;
                    case 4: std::cout << " (4-Split: One direction)"; break;
                }
                std::cout << std::endl;
                
                if (group.splitType == 4) {
                    std::cout << "Move Direction: " << group.moveDirection;
                    const char* dirNames[] = {"Up", "Up-Right", "Right", "Down-Right", 
                                            "Down", "Down-Left", "Left", "Up-Left"};
                    std::cout << " (" << dirNames[group.moveDirection] << ")" << std::endl;
                }
                
                std::cout << "Position: (" << group.mainRect.x1 << "," << group.mainRect.y1 
                         << ") to (" << group.mainRect.x2 << "," << group.mainRect.y2 << ")" << std::endl;
                std::cout << "Color: RGB(" << group.mainRect.Rvalue << "," 
                         << group.mainRect.Gvalue << "," << group.mainRect.Bvalue << ")" << std::endl;
                std::cout << "=========================" << std::endl;
                
                Split8Rectangle(i);
                glutPostRedisplay();
                return;
            }
        }
        
        // Ŭ�������� �簢���� ���� ���
        std::cout << "No rectangle clicked at (" << x << "," << y << ")" << std::endl;
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
        
        // ���� ���� Ÿ�԰� �̵� ���� ����
        group.splitType = splitDis(gen);        // 1~4 �� ����
        group.moveDirection = directionDis(gen); // 0~7 �� ����
        
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
            
            // ���ο� ���� ���� Ÿ�԰� �̵� ���� ����
            group.splitType = splitDis(gen);
            group.moveDirection = directionDis(gen);
            
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

// 8���� �Լ� - splitType�� ���� �ٸ� �������� ����
void Split8Rectangle(int groupIndex) {
    if (groupIndex < 0 || groupIndex >= 10) return;
    if (!rectGroups[groupIndex].isActive || rectGroups[groupIndex].isSplit) return;
    
    rect_group& group = rectGroups[groupIndex];
    ret& original = group.mainRect;
    
    // ���� ���� ���� ���
    std::cout << ">>> Starting split for Rectangle " << groupIndex << " <<<" << std::endl;
    
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
    
    // �ȹ� �̵� ���� ���� [��, ���, ��, ����, ��, ����, ��, �»�]
    const GLdouble directions[8][2] = {
        {0, -2},    // ��
        {1.4, -1.4}, // ���  
        {2, 0},     // ��
        {1.4, 1.4}, // ����
        {0, 2},     // ��
        {-1.4, 1.4}, // ����
        {-2, 0},    // ��
        {-1.4, -1.4} // �»�
    };
    
    // ��� ������ ���� ���� ����
    for (int i = 0; i < 8; i++) {
        group.pieces[i] = original;
        group.pieces[i].isActive = true;
        group.pieces[i].lifeTime = 0;
    }
    
    switch (group.splitType) {
        case 1: { // �����¿� 4���� + �߰��� 4��
            std::cout << "Splitting into 8 pieces: 4 corners + 4 middle pieces" << std::endl;
            
            // �»�, ����, ���, ����
            group.pieces[0].x1 = x1; group.pieces[0].y1 = y1;
            group.pieces[0].x2 = x1 + halfWidth; group.pieces[0].y2 = y1 + halfHeight;
            group.pieces[0].velocityX = -1.5; group.pieces[0].velocityY = -1.5;
            
            group.pieces[1].x1 = x1; group.pieces[1].y1 = y1 + halfHeight;
            group.pieces[1].x2 = x1 + halfWidth; group.pieces[1].y2 = y2;
            group.pieces[1].velocityX = -1.5; group.pieces[1].velocityY = 1.5;
            
            group.pieces[2].x1 = x1 + halfWidth; group.pieces[2].y1 = y1;
            group.pieces[2].x2 = x2; group.pieces[2].y2 = y1 + halfHeight;
            group.pieces[2].velocityX = 1.5; group.pieces[2].velocityY = -1.5;
            
            group.pieces[3].x1 = x1 + halfWidth; group.pieces[3].y1 = y1 + halfHeight;
            group.pieces[3].x2 = x2; group.pieces[3].y2 = y2;
            group.pieces[3].velocityX = 1.5; group.pieces[3].velocityY = 1.5;
            
            // �߰��� 4��
            group.pieces[4].x1 = x1; group.pieces[4].y1 = y1 + quarterHeight;
            group.pieces[4].x2 = x1 + halfWidth; group.pieces[4].y2 = y2 - quarterHeight;
            group.pieces[4].velocityX = -2.0; group.pieces[4].velocityY = 0;
            
            group.pieces[5].x1 = x1 + quarterWidth; group.pieces[5].y1 = y1;
            group.pieces[5].x2 = x2 - quarterWidth; group.pieces[5].y2 = y1 + halfHeight;
            group.pieces[5].velocityX = 0; group.pieces[5].velocityY = -2.0;
            
            group.pieces[6].x1 = x1 + quarterWidth; group.pieces[6].y1 = y1 + halfHeight;
            group.pieces[6].x2 = x2 - quarterWidth; group.pieces[6].y2 = y2;
            group.pieces[6].velocityX = 0; group.pieces[6].velocityY = 2.0;
            
            group.pieces[7].x1 = x1 + halfWidth; group.pieces[7].y1 = y1 + quarterHeight;
            group.pieces[7].x2 = x2; group.pieces[7].y2 = y2 - quarterHeight;
            group.pieces[7].velocityX = 2.0; group.pieces[7].velocityY = 0;
            break;
        }
        case 2: { // �밢�� 4����
            std::cout << "Splitting into 4 diagonal pieces" << std::endl;
            
            // 4���� Ȱ��ȭ, �������� ��Ȱ��ȭ
            for (int i = 4; i < 8; i++) {
                group.pieces[i].isActive = false;
            }
            
            group.pieces[0].x1 = x1; group.pieces[0].y1 = y1;
            group.pieces[0].x2 = x1 + halfWidth; group.pieces[0].y2 = y1 + halfHeight;
            group.pieces[0].velocityX = -2.0; group.pieces[0].velocityY = -2.0;
            
            group.pieces[1].x1 = x1 + halfWidth; group.pieces[1].y1 = y1;
            group.pieces[1].x2 = x2; group.pieces[1].y2 = y1 + halfHeight;
            group.pieces[1].velocityX = 2.0; group.pieces[1].velocityY = -2.0;
            
            group.pieces[2].x1 = x1; group.pieces[2].y1 = y1 + halfHeight;
            group.pieces[2].x2 = x1 + halfWidth; group.pieces[2].y2 = y2;
            group.pieces[2].velocityX = -2.0; group.pieces[2].velocityY = 2.0;
            
            group.pieces[3].x1 = x1 + halfWidth; group.pieces[3].y1 = y1 + halfHeight;
            group.pieces[3].x2 = x2; group.pieces[3].y2 = y2;
            group.pieces[3].velocityX = 2.0; group.pieces[3].velocityY = 2.0;
            break;
        }
        case 3: { // �ȹ� 8����
            std::cout << "Splitting into 8 pieces in all directions" << std::endl;
            
            GLdouble centerX = x1 + halfWidth;
            GLdouble centerY = y1 + halfHeight;
            GLdouble pieceWidth = width / 3;
            GLdouble pieceHeight = height / 3;
            
            for (int i = 0; i < 8; i++) {
                group.pieces[i].x1 = centerX - pieceWidth/2;
                group.pieces[i].y1 = centerY - pieceHeight/2;
                group.pieces[i].x2 = centerX + pieceWidth/2;
                group.pieces[i].y2 = centerY + pieceHeight/2;
                group.pieces[i].velocityX = directions[i][0];
                group.pieces[i].velocityY = directions[i][1];
            }
            break;
        }
        case 4: { // �ѹ������� 4�� ����
            const char* dirNames[] = {"Up", "Up-Right", "Right", "Down-Right", 
                                    "Down", "Down-Left", "Left", "Up-Left"};
            std::cout << "Splitting into 4 pieces moving " << dirNames[group.moveDirection] << std::endl;
            
            // 4���� Ȱ��ȭ
            for (int i = 4; i < 8; i++) {
                group.pieces[i].isActive = false;
            }
            
            GLdouble pieceWidth = width / 2;
            GLdouble pieceHeight = height / 2;
            GLdouble moveX = directions[group.moveDirection][0];
            GLdouble moveY = directions[group.moveDirection][1];
            
            // 4����Ͽ� ��� ���� ��������
            group.pieces[0].x1 = x1; group.pieces[0].y1 = y1;
            group.pieces[0].x2 = x1 + pieceWidth; group.pieces[0].y2 = y1 + pieceHeight;
            group.pieces[0].velocityX = moveX; group.pieces[0].velocityY = moveY;
            
            group.pieces[1].x1 = x1 + pieceWidth; group.pieces[1].y1 = y1;
            group.pieces[1].x2 = x2; group.pieces[1].y2 = y1 + pieceHeight;
            group.pieces[1].velocityX = moveX; group.pieces[1].velocityY = moveY;
            
            group.pieces[2].x1 = x1; group.pieces[2].y1 = y1 + pieceHeight;
            group.pieces[2].x2 = x1 + pieceWidth; group.pieces[2].y2 = y2;
            group.pieces[2].velocityX = moveX; group.pieces[2].velocityY = moveY;
            
            group.pieces[3].x1 = x1 + pieceWidth; group.pieces[3].y1 = y1 + pieceHeight;
            group.pieces[3].x2 = x2; group.pieces[3].y2 = y2;
            group.pieces[3].velocityX = moveX; group.pieces[3].velocityY = moveY;
            break;
        }
    }
    
    std::cout << "Split completed!" << std::endl << std::endl;
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
            
            // ũ�� ��� (�߽��� ����) - �ſ� ���� ��� �ӵ�
            GLdouble centerX = (piece.x1 + piece.x2) / 2;
            GLdouble centerY = (piece.y1 + piece.y2) / 2;
            GLdouble currentWidth = piece.x2 - piece.x1;
            GLdouble currentHeight = piece.y2 - piece.y1;
            
            GLdouble newWidth = currentWidth * 0.96;  // 4% ��� (�ſ� ���� ���)
            GLdouble newHeight = currentHeight * 0.96;
            
            piece.x1 = centerX - newWidth / 2;
            piece.x2 = centerX + newWidth / 2;
            piece.y1 = centerY - newHeight / 2;
            piece.y2 = centerY + newHeight / 2;
            
            piece.lifeTime++;
            
            // ���� ����: ũ�Ⱑ 10 ���� �Ǵ� 300������(5��) ���
            if (abs(piece.x2 - piece.x1) <= 5 || abs(piece.y2 - piece.y1) <= 5 || piece.lifeTime > 300) {
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