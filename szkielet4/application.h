#ifndef APPLICATION_H
#define APPLICATION_H

#include "shaderprogram.h"
#include "model.h"
#include "board.h"
#include "piece.h"
#include "pgn.h"

class Application {
public:
	//ShaderProgram *shader;					//Wskaźnik na obiekt reprezentujący program cieniujący.

	GLuint textures[4];						//array of texture handles

	Model* models[6];						//array of pointers to models
	Chessboard* chessBoard;
	Piece* pieces[35];						//0-31 all pieces | 32-33 pieces to move | 34 piece to beat
	Match* match;

	Application(ShaderProgram*);
};

#endif