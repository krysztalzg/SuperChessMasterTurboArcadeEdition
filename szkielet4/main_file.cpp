#include <stdio.h>
#include <vector>
#include <string>
#include <iterator>

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "application.h"
#include "tga.h"
#include "shaderprogram.h"
#include "board.h"
#include "piece.h"
#include "model.h"
#include "pgn.h"


using namespace std;
using namespace glm;

//Macierze
mat4  matP;																										//rzutowania
mat4  matV;																										//widoku
vec3 camera = vec3(0.0f, 5.0f, -5.0f);																			//camera (vector)

//Ustawienia okna i rzutowania
int windowWidth=1280;
int windowHeight=720;
float cameraAngle=45.0f;

//Zmienne do animacji
float speedX = 0, speedY = 0;
float angleX = 0, angleY = 0;
int lastTime = 0;

//Uchwyty na shadery
ShaderProgram *shader;																					//Wska�nik na obiekt reprezentuj�cy program cieniuj�cy.
/*
GLuint textures[4];																								//array of texture handles

Model* models[6];																								//array of pointers to models
Chessboard* chessBoard;																							
Piece* pieces[35];																								//0-31 all pieces | 32-33 pieces to move | 34 piece to beat
Match* match;*/

Application *App;



//Procedura rysuj�ca jaki� obiekt. Ustawia odpowiednie parametry dla vertex shadera i rysuje.
void drawObjects() {
	//W��czenie programu cieniuj�cego, kt�ry ma zosta� u�yty do rysowania
	//W tym programie wystarczy�oby wywo�a� to raz, w setupShaders, ale chodzi o pokazanie, 
	//�e mozna zmienia� program cieniuj�cy podczas rysowania jednej sceny
	shader->use();

	//Przeka� do shadera macierze P i V.
	glUniformMatrix4fv(shader->getUniformLocation("P"), 1, false, value_ptr(matP));
	glUniformMatrix4fv(shader->getUniformLocation("V"), 1, false, value_ptr(matV));

	glUniform1i(shader->getUniformLocation("textureMap0"), 0);
	glUniform1i(shader->getUniformLocation("textureMap1"), 1);

	glUniform4f(shader->getUniformLocation("lightPosition1"), 0, 1, 4, 1);
	glUniform4f(shader->getUniformLocation("lightPosition2"), 0, 4, -4, 1);

	for (int i = 0; i < 32; ++i)																				//drawing all pieces
		if (App->pieces[i]->getOnBoard())
			App->pieces[i]->draw(shader);
	//there to add drawing chessboard from model or primitive
}

//Procedura rysuj�ca
void displayFrame() {
	//Wyczy�� bufor kolor�w i bufor g��boko�ci
	glClearColor(0.5,0.5,0.5,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Wylicz macierz rzutowania
	matP=glm::perspective(cameraAngle, (float)windowWidth/(float)windowHeight, 1.0f, 100.0f);
	
	//Wylicz macierz widoku
	matV = lookAt(vec3(0.0f, 5.0f, -5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	
	//Narysuj obiekty
	drawObjects();
	
	//Tylny bufor na przedni
	glutSwapBuffers();
}


//Procedura uruchamiana okresowo. Robi animacj�.
void nextFrame(void) {
	int actTime=glutGet(GLUT_ELAPSED_TIME);
	int interval=actTime-lastTime;
	lastTime=actTime;

	angleX += speedX*interval / 1000.0;
	angleY += speedY*interval / 1000.0;

	if (angleX>360) angleX -= 360;
	if (angleX<0) angleX += 360;
	if (angleY>360) angleY -= 360;
	if (angleY<0) angleY += 360;
	/*
	for (int i = 0; i < 2; ++i)
		if (App->pieces[32 + i] != null && App->pieces[32 + i]->getMoving())
			App->pieces[32 + i]->move();
			*/
	glutPostRedisplay();
}

//Procedura wywo�ywana przy zmianie rozmiaru okna
void changeSize(int w, int h) {
	//Ustawienie wymiarow przestrzeni okna
	glViewport(0,0,w,h);
	//Zapami�tanie nowych wymiar�w okna dla poprawnego wyliczania macierzy rzutowania
	windowWidth=w;
	windowHeight=h;
}

//Procedura inicjuj�ca biblotek� glut
void initGLUT(int *argc, char** argv) {
	glutInit(argc,argv); //Zainicjuj bibliotek� GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);													//Alokuj bufory kolor�w (podw�jne buforowanie) i bufor kolor�w
	
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - windowWidth) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - windowHeight) / 2);	//Wska� pocz�tkow� pozycj� okna
	glutInitWindowSize(windowWidth,windowHeight); //Wska� pocz�tkowy rozmiar okna
	glutCreateWindow("SuperChessMasterTurboArcadeEdition"); //Utw�rz okno i nadaj mu tytu�
	
	glutReshapeFunc(changeSize);																				//Zarejestruj procedur� changeSize jako procedur� obs�uguj�ca zmian� rozmiaru okna
	glutDisplayFunc(displayFrame);																				//Zarejestruj procedur� displayFrame jako procedur� obs�uguj�ca od�wierzanie okna
	glutIdleFunc(nextFrame);																					//Zarejestruj procedur� nextFrame jako procedur� wywo�ywan� najcz�ci�j jak si� da (animacja)
}


//Procedura inicjuj�ca bibliotek� glew
void initGLEW() {
	GLenum err=glewInit();
	if (GLEW_OK!=err) {
		fprintf(stderr,"%s\n",glewGetErrorString(err));
		exit(1);
	}
	
}


//Wczytuje vertex shader i fragment shader i ��czy je w program cieniuj�cy
void setupShaders() {
	shader = new ShaderProgram("vshader.txt", NULL, "fshader.txt");
}

//procedura inicjuj�ca r�ne sprawy zwi�zane z rysowaniem w OpenGL
void initOpenGL() {
	setupShaders();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE_ARB);
}

//Zwolnij pami�� karty graficznej z shader�w i programu cieniuj�cego
void cleanShaders() {
	delete shader;
}


void keyDown(int c, int x, int y){
	switch (c) {
	case GLUT_KEY_UP:
		speedX = 100;
		break;
	case GLUT_KEY_DOWN:
		speedX = -100;
		break;
	case GLUT_KEY_LEFT:
		speedY = 100;
		break;
	case GLUT_KEY_RIGHT:
		speedY = -100;
		break;
	case GLUT_KEY_DELETE:
	case 13:
		break;
	}
}


void keyUp(int c, int x, int y){
	if (c == GLUT_KEY_UP || c == GLUT_KEY_DOWN)
		speedX = 0;
	if (c == GLUT_KEY_RIGHT || c == GLUT_KEY_LEFT)
		speedY = 0;
}

int main(int argc, char** argv) {

	initGLUT(&argc,argv);
	initGLEW();
	initOpenGL(); 
	
	glutSpecialFunc(keyDown);
	glutSpecialUpFunc(keyUp);

	App = new Application(shader);

	glutMainLoop();
	
	cleanShaders();
	return 0;
}
