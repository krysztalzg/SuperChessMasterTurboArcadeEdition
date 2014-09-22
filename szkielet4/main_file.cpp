#include <stdio.h>
#include <vector>
#include <string>
#include <iterator>
#include <ctime>

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

int lock = 0;

//Macierze
mat4  matP;																										//rzutowania
mat4  matV;																										//widoku

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

	glUniform4f(shader->getUniformLocation("lightPosition1"), 4, 2, 4, 1);
	glUniform4f(shader->getUniformLocation("lightPosition2"), -4, 2, -4, 1);

	App->chessBoard->draw(shader);																				//drawing chessboard
	for (int i = 0; i < 32; i++)																				//drawing all pieces
		if (App->pieces[i]->getOnBoard())
			App->pieces[i]->draw(shader);
	
	
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

	matV = rotate(matV, angleX, vec3(1.0f, 0.0f, 0.0f));
	matV = rotate(matV, angleY, vec3(0.0f, 1.0f, 0.0f));
	
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

	angleX += speedX*interval / 1000.0f;
	angleY += speedY*interval / 1000.0f;

	if (angleX>360) angleX -= 360;
	if (angleX<0) angleX += 360;
	if (angleY>360) angleY -= 360;
	if (angleY<0) angleY += 360;
	
	for (int i = 0; i < 32; ++i)
		if (App->pieces[i]->getMoving())
			App->pieces[i]->move(App->chessBoard);

	for (int i = 0; i < 32; ++i)
		App->pieces[i]->setPossibleMoves(App->chessBoard);
			
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
	glutCreateWindow("SuperChessMasterTurboArcadeEdition X"); //Utw�rz okno i nadaj mu tytu�
	
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

void nextMove() {
	Move move = App->match->postMoves.back();
	App->match->postMoves.pop_back();
	int piece = -1, piece2 = -1;

	if (!move.castling) {							//normal move
		cout << (char)(move.col + 97) << (char)(move.row + 49) << endl;
		for (int i = 0; i < 32; ++i) {				//piece to moveto the target
			if (App->pieces[i]->getOnBoard()) {
				for (unsigned int j = 0; j < App->pieces[i]->possibleMoves.size(); ++j) {
					if (move.pcol == -1) {
						if (App->pieces[i]->possibleMoves[j]->col == move.col && App->pieces[i]->possibleMoves[j]->row == move.row && App->pieces[i]->type == move.piece && App->pieces[i]->color == move.color) {
							piece = i;
							break;
						}
					}
					else {
						if (App->pieces[i]->possibleMoves[j]->col == move.col && App->pieces[i]->possibleMoves[j]->row == move.row && App->pieces[i]->type == move.piece && App->pieces[i]->current->col == move.pcol && App->pieces[i]->color == move.color) {
							piece = i;
							break;
						}
					}
				}
			}
			if (piece != -1) {
				App->pieces[piece]->setTarget(&App->chessBoard->squares[move.col][move.row]);
				move.prow = App->pieces[piece]->current->row;
				move.pcolumn = App->pieces[piece]->current->col;
				move.pieceno = piece;
				break;
			}
		}
		if (move.capture) {							//piece to beat
			for (int i = 0; i < 32; ++i) {
				if (App->pieces[i]->getOnBoard()) {
					if (App->pieces[i]->current->col == move.col && App->pieces[i]->current->row == move.row)
						piece2 = i;
					if (piece2 != -1) {
						App->pieces[piece2]->setOnBoard(false);
						move.pieceno2 = piece2;
						break;
					}
				}
			}
		}
	}
	else {											//castling
		cout << "castling" << endl;
		switch (move.color) {
		case -1:
			if (move.castling == 1) {				//short
				App->pieces[31]->setTarget(&App->chessBoard->squares[6][7]);
				App->pieces[30]->setTarget(&App->chessBoard->squares[5][7]);
			}
			else {									//queen
				App->pieces[31]->setTarget(&App->chessBoard->squares[1][7]);
				App->pieces[29]->setTarget(&App->chessBoard->squares[2][7]);
			}
			break;
		case 1:
			if (move.castling == 1) {				//short
				App->pieces[15]->setTarget(&App->chessBoard->squares[6][0]);
				App->pieces[14]->setTarget(&App->chessBoard->squares[5][0]);
			}
			else {									//queen
				App->pieces[15]->setTarget(&App->chessBoard->squares[1][0]);
				App->pieces[13]->setTarget(&App->chessBoard->squares[2][0]);
			}
		}
	}
	App->match->reMoves.push_back(move);
}


void prevMove() {
	Move move = App->match->reMoves.back();
	App->match->reMoves.pop_back();

	if (!move.castling) {
		App->pieces[move.pieceno]->setTarget(&App->chessBoard->squares[move.pcolumn][move.prow]);
		if (move.pieceno2 != -1)
			App->pieces[move.pieceno]->setOnBoard(true);
	}
	else {
		switch (move.color) {
		case -1:
			if (move.castling == 1) {				//short
				App->pieces[31]->setTarget(&App->chessBoard->squares[4][7]);
				App->pieces[30]->setTarget(&App->chessBoard->squares[7][7]);
			}
			else {									//queen
				App->pieces[31]->setTarget(&App->chessBoard->squares[4][7]);
				App->pieces[29]->setTarget(&App->chessBoard->squares[0][7]);
			}
			break;
		case 1:
			if (move.castling == 1) {				//short
				App->pieces[15]->setTarget(&App->chessBoard->squares[4][0]);
				App->pieces[14]->setTarget(&App->chessBoard->squares[7][0]);
			}
			else {									//queen
				App->pieces[15]->setTarget(&App->chessBoard->squares[4][0]);
				App->pieces[13]->setTarget(&App->chessBoard->squares[0][0]);
			}
		}
	}
	App->match->postMoves.push_back(move);

}


void keyDown(int c, int x, int y) {
	int temp, temp2;
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
	case GLUT_KEY_F1:
		break;
	case GLUT_KEY_F2:
		temp2 = rand() % 32;
		if (App->pieces[temp2]->possibleMoves.size() > 0 && App->pieces[temp2]->getOnBoard()) {
			temp = rand() % App->pieces[temp2]->possibleMoves.size();
			App->pieces[temp2]->setTarget(App->pieces[temp2]->possibleMoves[temp]);
		if (App->pieces[temp2]->possibleMoves[temp]->piece != 0)
			for (int i = 0; i < 32; i++)
				if (App->pieces[i]->current->col == App->pieces[temp2]->possibleMoves[temp]->col && App->pieces[i]->current->row == App->pieces[temp2]->possibleMoves[temp]->row)
					App->pieces[i]->setOnBoard(false);
		}
		break;
	case GLUT_KEY_F3:	
		for (int i = 0; i < 32; ++i)
			if (App->pieces[i]->getMoving())
				lock++;
		
		if (!App->match->postMoves.empty() && lock==0)
			nextMove();

		if (App->match->postMoves.empty())
			cout << "End of game - " << App->match->result;
		lock = 0;
		break;
	case GLUT_KEY_F4:
		for (int i = 0; i < 32; ++i)
			if (App->pieces[i]->getMoving())
				lock++;
		if (!App->match->reMoves.empty() && lock == 0)
		prevMove();

		lock = 0;
		break;
	case GLUT_KEY_F5:
		break;
	case GLUT_KEY_F6:
		system("cls");
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++)
				cout << App->chessBoard->squares[i][j].piece << "\t";
			cout << endl;
		}
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
	srand((unsigned int)time(NULL));
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
