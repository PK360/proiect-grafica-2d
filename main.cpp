//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - 04_02_Indexare.cpp          |
// ================================================
// 

#include <GL/glew.h>        
#include <GLFW/glfw3.h>     
#include <GL/freeglut.h>

#include "loadShaders.h"
#include "glm/glm.hpp"      
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SOIL.h"

#include <string>

#include "classes/PhysicsOOP.h"



// -------------------------- variables --------------------------
// shader names
const char* shdr_vert = "vert_shader.vert", *shdr_frag = "frag_shader.frag";

// ids
GLuint id_vao, id_vbo, id_ebo, id_program;

// textures
std::vector<GLuint> text_balls(16);

// uniform locations
GLuint loc_codCol, loc_myMatrix, loc_uvOffset;

// transform matrices
glm::mat4 matr_final, matr_resize, matr_scale, matr_transl;

// Physics Engine
PhysicsOOP physics;

// other variables
int last_t;
glm::vec2 range_x{-400.0f, 400.0f}, range_y{-300.0f, 300.0f};
glm::vec2 delta{range_x.y - range_x.x, range_y.y - range_y.x};




// ---------------------- functions ----------------------

//  Se initializeaza un Vertex Buffer Object (VBO) pentru tranferul datelor
void createVBO(void) {
    GLfloat Vertices[] = {
        // Coordonate (x, y, z, w)     Culori (r, g, b)    Texturare (u, v)
        -0.5f, -0.5f, 0.0f, 1.0f,      1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // Stanga jos
         0.5f, -0.5f, 0.0f, 1.0f,      1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // Dreapta jos
         0.5f,  0.5f, 0.0f, 1.0f,      1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // Dreapta sus
        -0.5f,  0.5f, 0.0f, 1.0f,      1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // Stanga sus
    };

    //  Indicii care determina ordinea de parcurgere a varfurilor;
    GLuint Indices[] = {
            0, 1, 2,  //  Primul triunghi;
            0, 2, 3   //  Al doilea triunghi;
    };

    //  Se creeaza / se leaga un VAO
    glGenVertexArrays(1, &id_vao);                                                   
    glBindVertexArray(id_vao);

    //  Se creeaza un buffer pentru VARFURI - COORDONATE, CULORI si TEXTURARE;
    glGenBuffers(1, &id_vbo);                                                   
    glBindBuffer(GL_ARRAY_BUFFER, id_vbo);                                      
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    //  Se creeaza un buffer pentru INDICI;
    glGenBuffers(1, &id_ebo);                                                       
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_ebo);                                  
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    //  Se activeaza lucrul cu atribute;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(7 * sizeof(GLfloat)));
}

//  Functia de incarcare a texturilor in program;
GLuint LoadTexture(const char* texturePath) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height;
    unsigned char* image = SOIL_load_image(texturePath, &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);       
    createVBO();                                

    physics.init();

    // texturi bile
    for(int i = 0; i < 16; i++) {
        std::string path = "textures/balls/ball-" + std::to_string(i) + ".png";
        text_balls[i] = LoadTexture(path.c_str());
        
        // momentan sunt doar randomly spawned pe masa
        physics.addCircle(new Ball(
            i + 1, 
            {ourMath::randomFloat(global::tableMinX + 20, global::tableMaxX - 20), 
             ourMath::randomFloat(global::tableMinY + 20, global::tableMaxY - 20)}, 
            {0.0f, 0.0f}
        ));
    }

    id_program = LoadShaders(shdr_vert, shdr_frag);
    glUseProgram(id_program);

    loc_codCol = glGetUniformLocation(id_program, "codCol");
    loc_myMatrix = glGetUniformLocation(id_program, "myMatrix");
    loc_uvOffset = glGetUniformLocation(id_program, "uvOffset");

    matr_resize = glm::ortho(range_x.x, range_x.y, range_y.x, range_y.y);
    last_t = glutGet(GLUT_ELAPSED_TIME);
}

//  Functia de desenare
void render(void) {
    int now_t = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (now_t - last_t) / 1000.0f;
    last_t = now_t;

    physics.update(deltaTime);

    glClear(GL_COLOR_BUFFER_BIT);           
    glUseProgram(id_program);
    glBindVertexArray(id_vao);



    // masa de billiard
    glUniform2f(loc_uvOffset, 0.0f, 0.0f);
    float table_w = global::tableMaxX - global::tableMinX, table_h = global::tableMaxY - global::tableMinY;
    float table_center_x = (global::tableMinX + global::tableMaxX) / 2.0f, table_center_y = (global::tableMinY + global::tableMaxY) / 2.0f;
    
    matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(table_center_x, table_center_y, 0.0f)); 
    matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(table_w, table_h, 1.0f));
    matr_final = matr_resize * matr_transl * matr_scale;
    
    glUniformMatrix4fv(loc_myMatrix, 1, GL_FALSE, &matr_final[0][0]);
    glUniform1i(loc_codCol, 0);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);



    // bilele
    glUniform1i(loc_codCol, 1);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(id_program, "myTexture"), 0);

    float ball_diam = global::ballSize * 2.0f;
    matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(ball_diam, ball_diam, 1.0f));

    const auto& shapes = physics.getShapes();
    for (size_t i = 0; i < shapes.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, text_balls[i]);
        
        glUniform2f(loc_uvOffset, shapes[i]->getUVOffset().x, shapes[i]->getUVOffset().y);        
        matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(shapes[i]->getPosition().x, shapes[i]->getPosition().y, 0.0f));
        matr_final = matr_resize * matr_transl * matr_scale;
        
        glUniformMatrix4fv(loc_myMatrix, 1, GL_FALSE, &matr_final[0][0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    //glutSwapBuffers();
    glFlush();                           
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    // Press Spacebar to shoot ball #1
    if (key == ' ') {
        if (!physics.getShapes().empty()) {
            physics.getShapes()[0]->setSpeed({ourMath::randomFloat(-600.0f, 600.0f), ourMath::randomFloat(-600.0f, 600.0f)});
        }
    }
}

// Eliberarea resurselor alocate de program
void Cleanup(void) {
    glDeleteProgram(id_program);

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &id_vbo);
    glDeleteBuffers(1, &id_ebo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &id_vao);

    text_balls.clear();
}

//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);					//	Modul de afisare al ferestrei, se foloseste un singur buffer de afisare si culori RGB;
	glutInitWindowSize(global::win_w, global::win_h);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(global::winpos_x, global::winpos_y);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Indexarea varfurilor");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

    glewInit();
    
    Initialize();						//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
    glutDisplayFunc(render);	//  Desenarea scenei in fereastra;
    glutKeyboardFunc(keyboard);
    glutCloseFunc(Cleanup);				//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();
	return 0;
}