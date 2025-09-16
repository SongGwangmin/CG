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
    case 'a':
    {
        int i = 0;
        while (rectcount < 30 && i < 10) {
			ret* newrect = new ret;
			newrect->x1 = numdis(gen);
			newrect->y1 = numdis(gen);
			newrect->x2 = newrect->x1 + rectspace;
			newrect->y2 = newrect->y1 + rectspace;
			newrect->Rvalue = dis(gen) / 256.0f;
			newrect->Gvalue = dis(gen) / 256.0f;
			newrect->Bvalue = dis(gen) / 256.0f;
			rectlist.push_back(*newrect);
			rectcount++;
			i++;
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
                        // �� ���� ������ x1, y1 ����
                        rectlist_iter->x1 = std::min(rectlist_iter->x1, it->x1);
                        rectlist_iter->y1 = std::min(rectlist_iter->y1, it->y1);
                        
                        // �� ū ������ x2, y2 ����
                        rectlist_iter->x2 = std::max(rectlist_iter->x2, it->x2);
                        rectlist_iter->y2 = std::max(rectlist_iter->y2, it->y2);
                        
                        // ���ο� ���� ����
                        rectlist_iter->Rvalue = dis(gen) / 256.0f;
                        rectlist_iter->Gvalue = dis(gen) / 256.0f;
                        rectlist_iter->Bvalue = dis(gen) / 256.0f;
                        
                        // �浹�� �簢�� ����
                        rectlist.erase(it);
                        rectcount--;
                        break; // erase �� �ݺ��ڰ� ��ȿȭ�ǹǷ� ���� ����
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
                        rectlist_iter = std::prev(it.base());
                        
                        if (rectcount < 29) { // 2���� �߰��� ���̹Ƿ� 29 �̸��� ��
                            // rectlist_iter�� �簢�� ���� ����
                            GLdouble minX = rectlist_iter->x1;
                            GLdouble minY = rectlist_iter->y1;
                            GLdouble maxX = rectlist_iter->x2;
                            GLdouble maxY = rectlist_iter->y2;
                            GLdouble width = maxX - minX;
                            GLdouble height = maxY - minY;
                            
                            // ù ��° ���ο� �簢�� ����
                            std::uniform_real_distribution<GLdouble> xDis(minX, maxX);
                            std::uniform_real_distribution<GLdouble> yDis(minY, maxY);
                            
                            ret newrect1;
                            newrect1.x1 = xDis(gen);
                            newrect1.y1 = yDis(gen);
                            newrect1.x2 = xDis(gen);
                            newrect1.y2 = yDis(gen);
                            
                            // newrect1 ����ȭ: x2 > x1, y2 > y1�� �ǵ��� ����
                            if (newrect1.x2 < newrect1.x1) {
                                std::swap(newrect1.x1, newrect1.x2);
                            }
                            if (newrect1.y2 < newrect1.y1) {
                                std::swap(newrect1.y1, newrect1.y2);
                            }
                            
                            newrect1.Rvalue = dis(gen) / 256.0f;
                            newrect1.Gvalue = dis(gen) / 256.0f;
                            newrect1.Bvalue = dis(gen) / 256.0f;

                            
                            
                            // �� ��° ���ο� �簢�� ����
                            ret newrect2;
                            newrect2.x1 = xDis(gen);
                            newrect2.y1 = yDis(gen);
                            newrect2.x2 = xDis(gen);
                            newrect2.y2 = yDis(gen);
                            
                            // newrect2 ����ȭ: x2 > x1, y2 > y1�� �ǵ��� ����
                            if (newrect2.x2 < newrect2.x1) {
                                std::swap(newrect2.x1, newrect2.x2);
                            }
                            if (newrect2.y2 < newrect2.y1) {
                                std::swap(newrect2.y1, newrect2.y2);
                            }
                            
                            newrect2.Rvalue = dis(gen) / 256.0f;
                            newrect2.Gvalue = dis(gen) / 256.0f;
                            newrect2.Bvalue = dis(gen) / 256.0f;
                            
                            // �� ���� ���ο� �簢���� ����Ʈ�� �߰�
                            rectlist.push_back(newrect1);
                            rectlist.push_back(newrect2);
                            rectcount += 2;
                            
                            // ���� �簢�� ����
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

bool isColliding(const ret& a, const ret& b) {
    return !(a.x2 <= b.x1 || // A�� B�� ���ʿ� ����
        a.x1 >= b.x2 || // A�� B�� �����ʿ� ����
        a.y2 <= b.y1 || // A�� B�� �Ʒ��� ����
        a.y1 >= b.y2);  // A�� B�� ���� ����
}