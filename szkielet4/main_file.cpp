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

Application* App;
/*
//Uchwyty na shadery
ShaderProgram *shader;																					//Wska�nik na obiekt reprezentuj�cy program cieniuj�cy.

GLuint textures[4];																								//array of texture handles

Model* models[6];																								//array of pointers to models
Chessboard* chessBoard;																							
Piece* pieces[35];																								//0-31 all pieces | 32-33 pieces to move | 34 piece to beat
Match* match;
*/

GLuint readTexture(char* filename) {
	GLuint tex;
	TGAImg img;
	glActiveTexture(GL_TEXTURE0);
	if (img.Load(filename) == IMG_OK) {
		glGenTextures(1, &tex); //Zainicjuj uchwyt tex
		glBindTexture(GL_TEXTURE_2D, tex); //Przetwarzaj uchwyt tex
		if (img.GetBPP() == 24) //Obrazek 24bit
			glTexImage2D(GL_TEXTURE_2D, 0, 3, img.GetWidth(), img.GetHeight(), 0,
			GL_RGB, GL_UNSIGNED_BYTE, img.GetImg());
		else if (img.GetBPP() == 32) //Obrazek 32bit
			glTexImage2D(GL_TEXTURE_2D, 0, 4, img.GetWidth(), img.GetHeight(), 0,
			GL_RGBA, GL_UNSIGNED_BYTE, img.GetImg());
		else {
			printf("Nieobs�ugiwany format obrazka w pliku: %s \n", filename);
		}
	}
	else {
		printf("B��d przy wczytywaniu pliku: %s\n", filename);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	return tex;
}

//Procedura rysuj�ca jaki� obiekt. Ustawia odpowiednie parametry dla vertex shadera i rysuje.
void drawObjects() {
	//W��czenie programu cieniuj�cego, kt�ry ma zosta� u�yty do rysowania
	//W tym programie wystarczy�oby wywo�a� to raz, w setupShaders, ale chodzi o pokazanie, 
	//�e mozna zmienia� program cieniuj�cy podczas rysowania jednej sceny
	App->shader->use();

	//Przeka� do shadera macierze P i V.
	glUniformMatrix4fv(App->shader->getUniformLocation("P"), 1, false, value_ptr(matP));
	glUniformMatrix4fv(App->shader->getUniformLocation("V"), 1, false, value_ptr(matV));

	glUniform1i(App->shader->getUniformLocation("textureMap0"), 0);
	glUniform1i(App->shader->getUniformLocation("textureMap1"), 1);

	glUniform4f(App->shader->getUniformLocation("lightPosition1"), 0, 1, 4, 1);
	glUniform4f(App->shader->getUniformLocation("lightPosition2"), 0, 4, -4, 1);

	for (int i = 0; i < 32; ++i)																				//drawing all pieces
		if (App->pieces[i]->getOnBoard())
			App->pieces[i]->draw(App->shader);

	//there to add drawing chessboard from model or primitive
}

//Procedura rysuj�ca
void displayFrame() {
	//Wyczy�� bufor kolor�w i bufor g��boko�ci
	glClearColor(0,0,0,1);
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

	for (int i = 0; i < 2; ++i)
		if (App->pieces[32 + i]->getMoving())
			App->pieces[32 + i]->move();

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
	glutCreateWindow("OpenGL 3.3"); //Utw�rz okno i nadaj mu tytu�
	
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
	App->shader=new ShaderProgram("vshader.txt",NULL,"fshader.txt");
}

//procedura inicjuj�ca r�ne sprawy zwi�zane z rysowaniem w OpenGL
void initOpenGL() {
	tex0 = readTexture("metal.tga");
	tex1 = readTexture("sky.tga");
	setupShaders();
	setupVBO();
	setupVAO();
	glEnable(GL_DEPTH_TEST);
}

//Zwolnij pami�� karty graficznej z shader�w i programu cieniuj�cego
void cleanShaders() {
	delete shaderProgram;
}

/**/void freeVBO() {
	glDeleteBuffers(1,&bufVertices);
	glDeleteBuffers(1,&bufColors);
	glDeleteBuffers(1,&bufNormals);
}

/**/void freeVAO() {
	glDeleteVertexArrays(1,&vao);
}

void keyDown(int c, int x, int y){
	if (c == GLUT_KEY_UP)
		speed = 50;
	if (c == GLUT_KEY_DOWN)
		speed = -50;
}

void keyUp(int c, int x, int y){
	if (c == GLUT_KEY_UP || c == GLUT_KEY_DOWN)
		speed = 0;
}

int main(int argc, char** argv) {
	initGLUT(&argc,argv);
	initGLEW();
	initOpenGL(); 

	App = new Application();

	
	glutSpecialFunc(keyDown);
	glutSpecialUpFunc(keyUp);
	
	glutMainLoop();
	
	freeVAO();
	freeVBO();
	cleanShaders();
	return 0;
}
