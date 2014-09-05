#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "GL/glew.h"
#include "stdio.h"

class ShaderProgram {
private:
	GLuint shaderProgram; //Uchwyt reprezentuj�cy program cieniujacy
	GLuint vertexShader; //Uchwyt reprezentuj�cy vertex shader
	GLuint geometryShader; //Uchwyt reprezentuj�cy geometry shader
	GLuint fragmentShader; //Uchwyt reprezentuj�cy fragment shader
	char* readFile(char*); //metoda wczytuj�ca plik tekstowy do tablicy znak�w
	GLuint loadShader(GLenum, char*); //Metoda wczytuje i kompiluje shader, a nast�pnie zwraca jego uchwyt
public:
	ShaderProgram(char*, char*, char*);
	~ShaderProgram();
	void use(); //W��cza wykorzystywanie programu cieniuj�cego
	GLuint getUniformLocation(char*); //Pobiera numer slotu zwi�zanego z dan� zmienn� jednorodn�
	GLuint getAttribLocation(char*); //Pobiera numer slotu zwi�zanego z danym atrybutem
};

#endif