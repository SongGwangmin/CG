#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>
#include <list>
#include <algorithm>

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

GLdouble Rvalue = 0.0;
GLdouble Gvalue = 0.0;
GLdouble Bvalue = 1.0;

int width;
int height;

std::list<ret> rectlist;
std::list<ret>::iterator rectlist_iter;
int rectcount = 0;

bool isLeftMousePressed = false; // ���� ���콺 ��ư�� ���� �������� Ȯ���ϴ� ����

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


// Ÿ�̸� �ݹ� (glutTimerFunc ��� ȣ��� �Լ� ����)
void TimerFunc(int value) {
    if (!timerRunning) return;

    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        glutPostRedisplay();
        glutTimerFunc(1000, TimerFunc, 0); // 1�ʸ��� ����
    }
}

void Motion(int x, int y);

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

    // ó���� 20���� �簢�� ����
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

    // ���ο� Ư���� �簢�� ���� (ũ��� ������ 2��)
    ret specialRect;
    specialRect.x1 = 250; // ȭ�� �߾� ��ó
    specialRect.y1 = 250;
    specialRect.x2 = specialRect.x1 + (rectspace * 2); // ���� 2��
    specialRect.y2 = specialRect.y1 + (rectspace * 2); // ���� 2��
    specialRect.Rvalue = 0.0f; // ���������� ����
    specialRect.Gvalue = 0.0f;
    specialRect.Bvalue = 0.0f;
    
    rectlist.push_back(specialRect);
    rectlist_iter = std::prev(rectlist.end()); // ������ ��Ҹ� ����Ŵ
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
        // Ư���� �簢���� ���� ���콺 ��ư�� ���� ���¿����� ǥ��
        if (it == rectlist_iter && !isLeftMousePressed) {
            continue; // Ư���� �簢���� �ǳʶٱ�
        }
        
        ret morphed;
        morph(morphed, *(it));
        glColor3f(it->Rvalue, it->Gvalue, it->Bvalue);
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
    case 'r': // ����: ��� ���� �ʱ� ���·� �ǵ�����
        {
            // ����Ʈ ������ ����
            rectlist.clear();
            rectcount = 0;
            isLeftMousePressed = false;
            
            // ó���� 20���� �簢�� ����
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

            // ���ο� Ư���� �簢�� ���� (���� ũ��� ����)
            ret specialRect;
            specialRect.x1 = 250; // ȭ�� �߾� ��ó
            specialRect.y1 = 250;
            specialRect.x2 = specialRect.x1 + (rectspace * 2); // ���� 2��
            specialRect.y2 = specialRect.y1 + (rectspace * 2); // ���� 2��
            specialRect.Rvalue = 0.0f; // ���������� ����
            specialRect.Gvalue = 0.0f;
            specialRect.Bvalue = 0.0f;
            
            rectlist.push_back(specialRect);
            rectlist_iter = std::prev(rectlist.end()); // ������ ��Ҹ� ����Ŵ
            rectcount++; // Ư���� �簢���� ī��Ʈ�� ����
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
            // Ư���� �簢���� ���콺 ��ġ�� �̵�
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
            // ���콺�� ������ Ư���� �簢���� ������ �ٽ� ���������� ����
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
            // rectcount�� 20 �̸��̸� ���ο� �簢�� �߰�
            if (rectcount < 20) {
                ret newrect;
                // Ŭ���� ��ġ�� �߽����� �簢�� ����
                newrect.x1 = x - rectspace / 2;
                newrect.y1 = y - rectspace / 2;
                newrect.x2 = x + rectspace / 2;
                newrect.y2 = y + rectspace / 2;
                
                // ȭ�� ��踦 ����� �ʵ��� ����
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
                
                // Ư���� �簢�� �տ� �߰� (Ư���� �簢���� �������� �����ǵ���)
                rectlist.insert(rectlist_iter, newrect);
                rectcount++;
                
                // Ư���� �簢�� ũ�� ���� (x1, y1 5�� ����, x2, y2 5�� ����)
                if (rectlist_iter != rectlist.end()) {
                    rectlist_iter->x1 += 5;
                    rectlist_iter->y1 += 5;
                    rectlist_iter->x2 -= 5;
                    rectlist_iter->y2 -= 5;
                    
                    // �簢�� ũ�Ⱑ �ʹ� �۾����� �ʵ��� �ּ� ũ�� ����
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
    // ���� ���콺 ��ư�� ���� ���¿��� Ư���� �簢���� ���콺�� ���� �̵�
    if (isLeftMousePressed && rectlist_iter != rectlist.end()) {
        GLdouble rectWidth = rectlist_iter->x2 - rectlist_iter->x1;
        GLdouble rectHeight = rectlist_iter->y2 - rectlist_iter->y1;
        rectlist_iter->x1 = x - rectWidth / 2;
        rectlist_iter->y1 = y - rectHeight / 2;
        rectlist_iter->x2 = rectlist_iter->x1 + rectWidth;
        rectlist_iter->y2 = rectlist_iter->y1 + rectHeight;
        
        // �ٸ� �簢������� �浹 �˻�
        for (auto it = rectlist.begin(); it != rectlist.end(); ) {
            if (it != rectlist_iter && isColliding(*rectlist_iter, *it)) {
                // �浹�� �簢���� ������ Ư���� �簢���� ����
                rectlist_iter->Rvalue = it->Rvalue;
                rectlist_iter->Gvalue = it->Gvalue;
                rectlist_iter->Bvalue = it->Bvalue;
                
                // Ư���� �簢�� ũ�� ���� (x1, y1 5�� ����, x2, y2 5�� ����)
                rectlist_iter->x1 -= 5;
                rectlist_iter->y1 -= 5;
                rectlist_iter->x2 += 5;
                rectlist_iter->y2 += 5;
                
                // �浹�� �簢�� ����
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
    return !(a.x2 <= b.x1 || // A�� B�� ���ʿ� ����
        a.x1 >= b.x2 || // A�� B�� �����ʿ� ����
        a.y2 <= b.y1 || // A�� B�� �Ʒ��� ����
        a.y1 >= b.y2);  // A�� B�� ���� ����
}