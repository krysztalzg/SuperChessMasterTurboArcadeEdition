#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "GL/glew.h"
#include "stdio.h"

class ShaderProgram {
private:
	GLuint shaderProgram; //Uchwyt reprezentujący program cieniujacy
	GLuint vertexShader; //Uchwyt reprezentujący vertex shader
	GLuint geometryShader; //Uchwyt reprezentujący geometry shader
	GLuint fragmentShader; //Uchwyt reprezentujący fragment shader
	char* readFile(char*); //metoda wczytująca plik tekstowy do tablicy znaków
	GLuint loadShader(GLenum, char*); //Metoda wczytuje i kompiluje shader, a następnie zwraca jego uchwyt
public:
	ShaderProgram(char*, char*, char*);
	~ShaderProgram();
	void use(); //Włącza wykorzystywanie programu cieniującego
	GLuint getUniformLocation(char*); //Pobiera numer slotu związanego z daną zmienną jednorodną
	GLuint getAttribLocation(char*); //Pobiera numer slotu związanego z danym atrybutem
};

#endif