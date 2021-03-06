#include <time.h>
#include "Punto.h"
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <list>	
#include <GL/glut.h>


using namespace std;	//para utilizar los cin y los cout 

float red = 1.0, green = 0.0, blue = 0.0;
int tmpx, tmpy; // Almacena el primer punto
int brushSize = 4;
int eraserSize = 1;
bool isSecond = false;
bool isRandom = false;
bool isEraser = false;
bool isRadial = false;
float window_w = 500;
float window_h = 500;

int shape = 1; // 1:Punto, 2:Linea, 3:Rectangulo, 4:Circulo, 5:Pincel

 vector<Punto> Puntos;		// Almacena todos los puntos para retroceder una accion
 list<int> undoHistory; // Lista todos las acciones del retroceso maximo 20.
 list<int> redoHistory; // record for redo, maximum 20 shapes in history
 vector<Punto> redoPuntos;  // store the Puntos after undo temporaly

void display(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(2);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < Puntos.size(); i++)
	{
		glColor3f(Puntos[i].getR(), Puntos[i].getG(), Puntos[i].getB());
		glVertex2i(Puntos[i].getX(), Puntos[i].getY());
	}
	glEnd();
	glutSwapBuffers();
}

void clear()
{
	Puntos.clear();
	undoHistory.clear();
	redoPuntos.clear();
	redoHistory.clear();
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	 cout << asctime(timeinfo);
			  
}

void quit()
{
	 cout << "Thank you for using this Paint tool! Goodbye!" <<  endl;
	exit(0);
}
void undo()
{
	if (undoHistory.size() > 0)
	{
		if (undoHistory.back() != Puntos.size() && redoHistory.back() != Puntos.size())
		{
			redoHistory.push_back(Puntos.size());
		}
		int numRemove = Puntos.size() - undoHistory.back();
		for (int i = 0; i < numRemove; i++)
		{
			redoPuntos.push_back(Puntos.back());
			Puntos.pop_back();
		}
		redoHistory.push_back(undoHistory.back());
		undoHistory.pop_back();
	}
	else
	{
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		 cout << asctime(timeinfo)
				  << "[Warning] Cannot undo. This is the first record in the history.\n";
	}
}

void redo()
{
	if (redoHistory.size() > 1)
	{
		undoHistory.push_back(redoHistory.back());
		redoHistory.pop_back();
		int numRemove = redoHistory.back() - Puntos.size();
		for (int i = 0; i < numRemove; i++)
		{
			Puntos.push_back(redoPuntos.back());
			redoPuntos.pop_back();
		}
	}
	else
	{
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		 cout << asctime(timeinfo)
				  << "[Warning] Cannot redo. This is the last record in the history.\n";
	}
}

void drawPunto(int mousex, int mousey)
{
	Punto newPunto(mousex, window_h - mousey, isEraser ? 1.0 : red, isEraser ? 1.0 : green, isEraser ? 1.0 : blue);
	Puntos.push_back(newPunto);
}

void drawBrush(int x, int y)
{
	for (int i = 0; i < brushSize; i++)
	{
		int randX = rand() % (brushSize + 1) - brushSize / 2 + x;
		int randY = rand() % (brushSize + 1) - brushSize / 2 + y;
		drawPunto(randX, randY);
	}
}

void drawLine(int x1, int y1, int x2, int y2)
{
	bool changed = false;
	// Bresenham's line algorithm is only good when abs(dx) >= abs(dy)
	// So when abs(dx) < abs(dy), change axis x and y
	if (abs(x2 - x1) < abs(y2 - y1))
	{
		int tmp1 = x1;
		x1 = y1;
		y1 = tmp1;
		int tmp2 = x2;
		x2 = y2;
		y2 = tmp2;
		changed = true;
	}
	int dx = x2 - x1;
	int dy = y2 - y1;
	int yi = 1;
	int xi = 1;
	if (dy < 0)
	{
		yi = -1;
		dy = -dy;
	}
	if (dx < 0)
	{
		xi = -1;
		dx = -dx;
	}
	int d = 2 * dy - dx;
	int incrE = dy * 2;
	int incrNE = 2 * dy - 2 * dx;

	int x = x1, y = y1;
	if (changed)
		drawPunto(y, x);
	else
		drawPunto(x, y);
	while (x != x2)
	{
		if (d <= 0)
			d += incrE;
		else
		{
			d += incrNE;
			y += yi;
		}
		x += xi;
		if (changed)
			drawPunto(y, x);
		else
			drawPunto(x, y);
	}
}

/**
 * We can use drawLine function to draw the rectangle
 * 
 * Top-left corner specified by the first click,
 * and the bottom-right corner specified by a second click
 */
void drawRectangle(int x1, int y1, int x2, int y2)
{
	if (x1 < x2 && y1 < y2)
	{
		drawLine(x1, y1, x2, y1);
		drawLine(x2, y1, x2, y2);
		drawLine(x2, y2, x1, y2);
		drawLine(x1, y2, x1, y1);
	}
	else
	{
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		 cout << asctime(timeinfo)
				  << "[Warning] The first click should be the top-left corner, the second click should be bottom-right corner.\n";
	}
}

/**
 * Midpoint circle algorithm
 */
void drawCircle(int x1, int y1, int x2, int y2)
{
	int r = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
	double d;
	int x, y;

	x = 0;
	y = r;
	d = 1.25 - r;

	while (x <= y)
	{
		drawPunto(x1 + x, y1 + y);
		drawPunto(x1 - x, y1 + y);
		drawPunto(x1 + x, y1 - y);
		drawPunto(x1 - x, y1 - y);
		drawPunto(x1 + y, y1 + x);
		drawPunto(x1 - y, y1 + x);
		drawPunto(x1 + y, y1 - x);
		drawPunto(x1 - y, y1 - x);
		x++;
		if (d < 0)
		{
			d += 2 * x + 3;
		}
		else
		{
			y--;
			d += 2 * (x - y) + 5;
		}
	}
}

void drawRadialBrush(int x, int y)
{
	int xc = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int yc = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	int dx, dy;

	dx = xc - x;
	dy = yc - y;

	drawPunto(xc + dx, yc + dy);
	drawPunto(xc - dx, yc + dy);
	drawPunto(xc + dx, yc - dy);
	drawPunto(xc - dx, yc - dy);
	drawPunto(xc + dy, yc + dx);
	drawPunto(xc - dy, yc + dx);
	drawPunto(xc + dy, yc - dx);
	drawPunto(xc - dy, yc - dx);
}

void erase(int x, int y)
{
	for (int i = -eraserSize; i <= eraserSize; i++)
	{
		for (int j = -eraserSize; j <= eraserSize; j++)
		{
			drawPunto(x + i, y + j);
		}
	}
}

void keyboard(unsigned char key, int xIn, int yIn)
{
	isSecond = false;
	switch (key)
	{
	case 'q':
	case 27: // 27 is the esc key
		quit();
		break;
	case 'c':
		clear();
		break;
	case '+':
		if (shape == 5 && !isEraser)
		{
			if (brushSize < 16)
				brushSize += 4;
			else
			{
				time_t rawtime;
				struct tm *timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				 cout << asctime(timeinfo)
						  << "[Warning] El tama�o del pincel esta al maximo. No puede ser mas grande.\n";
			}
		}
		else if (isEraser)
		{
			if (eraserSize < 10)
				eraserSize += 4;
			else
			{
				time_t rawtime;
				struct tm *timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				 cout << asctime(timeinfo)
						  << "[Warning] El tama�o del borrador esta al maximo. No puede ser mas grande.\n";
			}
		}
		break;
	case '-':
		if (shape == 5 && !isEraser)
		{
			if (brushSize > 4)
				brushSize -= 4;
			else
			{
				time_t rawtime;
				struct tm *timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				 cout << asctime(timeinfo)
						  << "[Warning] El tama�o del pincel esta al minimo. No puede ser mas peque�o.\n";
			}
		}
		else if (isEraser)
		{
			if (eraserSize > 2)
				eraserSize -= 4;
			else
			{
				time_t rawtime;
				struct tm *timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				 cout << asctime(timeinfo)
						  << "[Warning] El tama�o del borrador esta al minimo. No puede ser mas peque�o.\n";
			}
		}
		break;
	case 'u':
		undo();
		break;
	case 'r':
		redo();
		break;
	}
}

void mouse(int bin, int state, int x, int y)
{
	if (bin == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (isRandom)
		{
			srand(time(NULL));
			red = float(rand()) / float(RAND_MAX);
			green = float(rand()) / float(RAND_MAX);
			blue = float(rand()) / float(RAND_MAX);
		}
		if (isEraser)
		{
			undoHistory.push_back(Puntos.size());
			erase(x, y);
		}
		else
		{
			if (shape == 1)
			{
				undoHistory.push_back(Puntos.size());
				if (isRadial)
					drawRadialBrush(x, y);
				else
					drawPunto(x, y);
			}
			else if (shape == 5)
			{
				undoHistory.push_back(Puntos.size());
				drawBrush(x, y);
			}
			else
			{
				if (!isSecond)
				{
					tmpx = x;
					tmpy = y;
					isSecond = true;
				}
				else
				{
					if (undoHistory.back() != Puntos.size())
						undoHistory.push_back(Puntos.size());
					if (shape == 2)
						drawLine(tmpx, tmpy, x, y);
					else if (shape == 3)
						drawRectangle(tmpx, tmpy, x, y);
					else if (shape == 4)
						drawCircle(tmpx, tmpy, x, y);
					isSecond = false;
				}
			}
		}
		if (undoHistory.size() > 20)
		{
			undoHistory.pop_front();
		}
	}
}

void motion(int x, int y)
{
	if (isEraser)
		erase(x, y);
	else
	{
		if (shape == 1)
		{
			if (isRadial)
				drawRadialBrush(x, y);
			else
				drawPunto(x, y);
		}
		if (shape == 5)
			drawBrush(x, y);
	}
}

void reshape(int w, int h)
{
	window_w = w;
	window_h = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void processMainMenu(int value)
{
	switch (value)
	{
	case 0:
		quit();
		break;
	case 1:
		clear();
		break;
	case 2:
		undo();
		break;
	case 3:
		redo();
		break;
	}
}

void processBrushSizeMenu(int value)
{
	shape = 5;
	isEraser = false;
	brushSize = value;
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
}

void processColourMenu(int value)
{
	isSecond = false;
	isEraser = false;
	isRandom = false;

	switch (value)
	{
	case 1: // red
		red = 1.0;
		green = 0.0;
		blue = 0.0;
		break;
	case 2: // green
		red = 0.0;
		green = 1.0;
		blue = 0.0;
		break;
	case 3: // blue
		red = 0.0;
		green = 0.0;
		blue = 1.0;
		break;
	case 4: // purple
		red = 0.5;
		green = 0.0;
		blue = 0.5;
		break;
	case 5: // yellow
		red = 1.0;
		green = 1.0;
		blue = 0.0;
		break;
	case 6: // random
		isRandom = true;
		break;
	}
}

void processShapeMenu(int value)
{
	shape = value;
	isEraser = false;
	isSecond = false;
	isRadial = false;

	switch (shape)
	{
	case 1:
		glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
		break;
	case 2:
	case 3:
	case 4:
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		break;
	}
}

void processEraserSizeMenu(int value)
{
	glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
	eraserSize = value;
	isEraser = true;
}

void processRadicalBrushMenu(int value)
{
	isRadial = value == 1 ? true : false;
}

void createOurMenu()
{
	int colourMenu = glutCreateMenu(processColourMenu);
	glutAddMenuEntry("Rojo", 1);
	glutAddMenuEntry("Verde", 2);
	glutAddMenuEntry("Azul", 3);
	glutAddMenuEntry("Purpura", 4);
	glutAddMenuEntry("Amarillo", 5);
	glutAddMenuEntry("Aleatorio", 6);

	int sizeMenu = glutCreateMenu(processBrushSizeMenu);
	glutAddMenuEntry("4px", 4);
	glutAddMenuEntry("8px", 8);
	glutAddMenuEntry("12px", 12);
	glutAddMenuEntry("16px", 16);

	int shapeMenu = glutCreateMenu(processShapeMenu);
	glutAddMenuEntry("Punto", 1);
	glutAddMenuEntry("Linea", 2);
	glutAddMenuEntry("Rectangulo", 3);
	glutAddMenuEntry("Ciculo", 4);
	glutAddSubMenu("Pincel", sizeMenu);

	int eraserSizeMenu = glutCreateMenu(processEraserSizeMenu);
	glutAddMenuEntry("Peque�o", 2);
	glutAddMenuEntry("Mediano", 6);
	glutAddMenuEntry("Grande", 10);

	int radicalBrushMenu = glutCreateMenu(processRadicalBrushMenu);
	glutAddMenuEntry("True", 1);
	glutAddMenuEntry("False", 2);

	int main_id = glutCreateMenu(processMainMenu);
	glutAddSubMenu("Color", colourMenu);
	glutAddSubMenu("Formas", shapeMenu);
	glutAddSubMenu("Radical Paint Brush", radicalBrushMenu);
	glutAddSubMenu("Borrador", eraserSizeMenu);
	glutAddMenuEntry("Retroceder", 2);
	glutAddMenuEntry("Adelantar", 3);
	glutAddMenuEntry("Limpiar", 1);
	glutAddMenuEntry("Salir", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void init(void)
{
	/* background color */
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor3f(red, green, blue);

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, window_w, 0.0, window_h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void FPS(int val)
{
	glutPostRedisplay();
	glutTimerFunc(0, FPS, 0);
}

void callbackInit()
{
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(17, FPS, 0);
}

void printGuide()
{
	
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(window_w, window_h);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Peint");
	callbackInit();
	init();
	printGuide();
	createOurMenu();
	glutMainLoop();
	return (0);
}
