//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - 04_02_Indexare.cpp |
// ======================================
// 
//	Program ce deseneaza un patrat, folosindu-se tehnicile MODERN OpenGL;
//	ELEMENTE DE NOUTATE:
//	 - folosirea indexarii varfurilor: elemente asociate (matrice, buffer);
//	 - desenarea se face folosind functia glDrawElements();
//
//
// 
//	Biblioteci

#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GLFW/glfw3.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <GL/freeglut.h>    //	Include functii pentru: 
#include <vector>
							//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
							//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
							//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SOIL.h"			//	Biblioteca pentru texturare;

using namespace std;

constexpr float PI = 3.141592;

class OpenGLRenderer {
	// -------------------------- variables --------------------------
	// window variables
	GLfloat win_w = 800.0f, win_h = 600.0f, winpos_x = 100.0f, winpos_y = 100.0f;

	// ids
	GLuint id_vao, id_vbo, id_ebo, id_program;

	// uniform locations
	GLuint loc_codCol, loc_myMatrix;
	std::vector<GLuint> id_textures;

	// transform matrices
	glm::mat4 matr_final, matr_resize, matr_scale, matr_transl;
	

	// other variables
	glm::vec2 range_x{-300.0f, 300.0f}, range_y{-300.0f, 300.0f};
	glm::vec2 delta{range_x.y - range_x.x, range_y.y - range_y.x};


	// ---------------------- private functions ----------------------
	//  Setarea parametrilor necesari pentru fereastra de vizualizare;
	void Initialize(void) {
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);		//  Culoarea de fond a ecranului;
		createVBO();								//  Trecerea datelor de randare spre bufferul folosit de shadere;

		//	Incarcarea texturii;
		LoadTexture("text_smiley_face.png");

		// create shaders
		this->id_program = LoadShaders("ex.3-04_02_Shader.vert", "ex.3-04_02_Shader.frag");
		glUseProgram(id_program);


		//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
		loc_codCol = glGetUniformLocation(id_program, "codCol");
		loc_myMatrix = glGetUniformLocation(id_program, "myMatrix");
		//	Dreptunghiul "decupat"; 

		matr_resize = glm::ortho(range_x.x, range_x.y, range_y.x, range_y.y);
		matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 0.0));
		matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.5f, 1.0f));
	}

public:
	OpenGLRenderer(int& argc, char* argv[]) {
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);					//	Modul de afisare al ferestrei, se foloseste un singur buffer de afisare si culori RGB;
		glutInitWindowSize(win_w, win_h);						//  Dimensiunea ferestrei;
		glutInitWindowPosition(winpos_x, winpos_y);								//  Pozitia initiala a ferestrei;
		glutCreateWindow("Indexarea varfurilor");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

		//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
		//  Trebuie initializat inainte de desenare;

		glewInit();

		Initialize();						//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
		glutDisplayFunc(render);	//  Desenarea scenei in fereastra;
		glutCloseFunc(~OpenGLRenderer);				//  Eliberarea resurselor alocate de program;
	}

	~OpenGLRenderer() {
		// destroy shader
		glDeleteProgram(id_program);

		
		// destroy VBO
		//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);

		//  Stergerea bufferelor pentru VARFURI (Coordonate, Culori, Textura), INDICI;
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &id_vbo);
		glDeleteBuffers(1, &id_ebo);

		//  Eliberaea obiectelor de tip VAO;
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &id_vao);

		id_textures.clear();
	}

	void render(void) {
		glClear(GL_COLOR_BUFFER_BIT);			//  Se curata ecranul OpenGL pentru a fi desenat noul continut;


		//	Transmiterea variabilei uniforme pentru MATRICEA DE TRANSFORMARE spre shadere;
		matr_final = matr_resize * matr_transl * matr_scale;
		glUniformMatrix4fv(loc_myMatrix, 1, GL_FALSE, &matr_final[0][0]);
		glUniform1i(loc_codCol, 0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		
		// TEXTURARE 
		// Activarea / legarea texturii active
		matr_final = matr_resize * matr_scale * matr_transl;
		glUniformMatrix4fv(loc_myMatrix, 1, GL_FALSE, &matr_final[0][0]);
		glUniform1i(loc_codCol, 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id_textures.back());
		//	Transmiterea variabilei uniforme pentru texturare spre shaderul de fragmente;
		glUniform1i(glGetUniformLocation(id_program, "myTexture"), 0);

		//	Desenarea primitivelor;
		//	Functia glDrawElements primeste 4 argumente:
		//	 - arg1 = modul de desenare;
		//	 - arg2 = numarul de varfuri;
		//	 - arg3 = tipul de date al indicilor;
		//	 - arg4 = pointer spre indici (EBO): pozitia de start a indicilor;
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//	EXERCITIU: De realizat desenul folosind segmente de dreapta;


		glFlush();								//  Asigura rularea tuturor comenzilor OpenGL apelate anterior;
	}

	//  Se initializeaza un Vertex Buffer Object (VBO) pentru tranferul datelor spre memoria placii grafice (spre shadere);
	//  In acesta se stocheaza date despre varfuri (coordonate, culori, indici, texturare etc.);
	void createVBO(void) {
		GLfloat Vertices[] = {
			//	Coordonate;					Culori;				Coordonate de texturare;
				-15.0f, -15.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
			   15.0f, -15.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
			   15.0f,  15.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 	1.0f, 1.0f,
			  -15.0f,  15.0f, 0.0f, 1.0f, 	1.0f, 0.0f, 1.0f,	0.0f, 1.0f,
	
			  -50.0f, -50.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f,	// Stanga jos;
			   50.0f, -50.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,	1.0f, 0.0f, // Dreapta jos;
			   50.0f,  50.0f, 0.0f, 1.0f,   1.0f, 1.0f, 0.0f,	1.0f, 1.0f,	// Dreapta sus;
			  -50.0f,  50.0f, 0.0f, 1.0f,   0.0f, 1.0f, 1.0f,	0.0f, 1.0f  // Stanga sus;
		};
	
		//	Indicii care determina ordinea de parcurgere a varfurilor;
		GLuint Indices[] = {
			  0, 1, 2,  //	Primul triunghi;
			  0, 2, 3	//  Al doilea triunghi;
		};
	
		//  Transmiterea datelor prin buffere;
	
		//  Se creeaza / se leaga un VAO (Vertex Array Object);
		glGenVertexArrays(1, &id_vao);                                                   //  Generarea VAO si indexarea acestuia catre variabila id_vao;
		glBindVertexArray(id_vao);
	
		//  Se creeaza un buffer pentru VARFURI - COORDONATE, CULORI si TEXTURARE;
		glGenBuffers(1, &id_vbo);													//  Generarea bufferului si indexarea acestuia catre variabila id_vbo;
		glBindBuffer(GL_ARRAY_BUFFER, id_vbo);										//  Setarea tipului de buffer - atributele varfurilor;
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	
		//	Se creeaza un buffer pentru INDICI;
		glGenBuffers(1, &id_ebo);														//  Generarea bufferului si indexarea acestuia catre variabila id_ebo;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_ebo);									//  Setarea tipului de buffer - atributele varfurilor;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
	
		//	Se activeaza lucrul cu atribute;
		//  Se asociaza atributul (0 = coordonate) pentru shader;
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0);
		//  Se asociaza atributul (1 =  culoare) pentru shader;
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
		//  Se asociaza atributul (2 =  texturare) pentru shader;
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(7 * sizeof(GLfloat)));
	}

	//	Functia de incarcare a texturilor in program;
	void LoadTexture(const char* texturePath) {
		// create new texture id in vector
		id_textures.push_back(0);
		GLuint& id_texture = id_textures.back();


		// Generarea unui obiect textura si legarea acestuia;
		glGenTextures(1, &id_texture);
		glBindTexture(GL_TEXTURE_2D, id_texture);

		//	Desfasurarea imaginii pe orizontala/verticala in functie de parametrii de texturare;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Modul in care structura de texeli este aplicata pe cea de pixeli;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Incarcarea texturii si transferul datelor in obiectul textura; 
		int width, height;
		unsigned char* image = SOIL_load_image(texturePath, &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		
		// Eliberarea resurselor
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};