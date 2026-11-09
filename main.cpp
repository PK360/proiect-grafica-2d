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

    matr_resize = glm::ortho(global::range_x.x, global::range_x.y, global::range_y.x, global::range_y.y);
    last_t = glutGet(GLUT_ELAPSED_TIME);
}

// Funcție ajutătoare pentru desenarea unei găuri (pochet)
void drawPocket(float x, float y, float size) {
    glm::mat4 matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    glm::mat4 matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(size, size, 1.0f));
    glm::mat4 matr_final = matr_resize * matr_transl * matr_scale;
    
    glUniformMatrix4fv(loc_myMatrix, 1, GL_FALSE, &matr_final[0][0]);
    
    // Presupunem că în shader codCol = 2 sau un cod dedicat înseamnă culoarea neagră,
    // sau poți folosi 0 dacă tratezi manual culoarea. Să zicem codCol = 2 pentru negru:
    glUniform1i(loc_codCol, 2); 
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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



// 1. Masa de biliard (suprafața verde)
    glUniform2f(loc_uvOffset, 0.0f, 0.0f);
    float table_w = global::tableMaxX - global::tableMinX, table_h = global::tableMaxY - global::tableMinY;
    float table_center_x = (global::tableMinX + global::tableMaxX) / 2.0f;
    float table_center_y = (global::tableMinY + global::tableMaxY) / 2.0f;
    
    matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(table_center_x, table_center_y, 0.0f)); 
    matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(table_w, table_h, 1.0f));
    matr_final = matr_resize * matr_transl * matr_scale;
    
    glUniformMatrix4fv(loc_myMatrix, 1, GL_FALSE, &matr_final[0][0]);
    glUniform1i(loc_codCol, 0); // 0 = verde / postav
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

<<<<<<< Updated upstream
    // 2. Găurile negre (Pocheturile - cele 6 poziții corespunzătoare coliziunilor)
    float pocketDiameter = global::ballSize * 3.5f; // Dimensiunea vizuală a găurii negre
    float min_x = global::tableMinX;
    float max_x = global::tableMaxX;
    float min_y = global::tableMinY;
    float max_y = global::tableMaxY;
    float mid_x = table_center_x;

    // Cele 4 găuri de colț + 2 găuri de la mijlocul laturilor lungi
    drawPocket(min_x, min_y, pocketDiameter); // Colț stânga-jos
    drawPocket(max_x, min_y, pocketDiameter); // Colț dreapta-jos
    drawPocket(min_x, max_y, pocketDiameter); // Colț stânga-sus
    drawPocket(max_x, max_y, pocketDiameter); // Colț dreapta-sus
    drawPocket(mid_x, min_y, pocketDiameter); // Mijloc jos
    drawPocket(mid_x, max_y, pocketDiameter); // Mijloc sus
=======
    // gaurile mesei din colturi
    float pocketDiameter = global::pocketSize * 2.0f;
    float gaura_min_x = global::tableMinX, gaura_max_x = global::tableMaxX;
    float gaura_min_y = global::tableMinY, gaura_max_y = global::tableMaxY;
    float gaura_mid_x = table_center_x;

    std::vector<glm::vec2> pockets = {
        {gaura_min_x, gaura_max_y}, // stanga sus
        {gaura_min_x, gaura_min_y}, // stanga jos
        {gaura_mid_x, gaura_max_y}, // mijloc sus
        {gaura_mid_x, gaura_min_y}, // mijloc jos
        {gaura_max_x, gaura_max_y}, // dreapta sus
        {gaura_max_x, gaura_min_y}  // dreapta jos
    };

    drawPocket(pockets[0].x, pockets[0].y, pocketDiameter); // stanga  sus
    drawPocket(pockets[1].x, pockets[1].y, pocketDiameter); // stanga  jos
    drawPocket(pockets[2].x, pockets[2].y, pocketDiameter); // mijloc  sus
    drawPocket(pockets[3].x, pockets[3].y, pocketDiameter); // mijloc  jos
    drawPocket(pockets[4].x, pockets[4].y, pocketDiameter); // dreapta sus
    drawPocket(pockets[5].x, pockets[5].y, pocketDiameter); // dreapta jos



    // aim line
    if ((aiming || cueStriking) && !physics.getShapes().empty()) {
        Vec2f ballPos = physics.getShapes()[0]->getPosition();
        glm::vec2 bPos(ballPos.x, ballPos.y);

        glm::vec2 dir = aiming ? (bPos - cur_mousePos) : (strikeDir * (strikePower / 3.5f));
        float dist = glm::length(dir);

        if (dist > 5.0f || cueStriking) {
            glm::vec2 normDir = aiming ? glm::normalize(dir) : strikeDir;
            float angle = atan2(normDir.y, normDir.x);

            // Cat timp tintim, retragerea tacului este proportionala cu distanta mouse-ului
            if (aiming) {
                cuePullOffset = std::min(dist * 0.08f, 45.0f); // se retrage pana la max 120 (no 45!!!) unitati
            }

            // 1. LINIA DE TINTIRE (doar cat timp tinem click apasat)
            if (aiming) {
                float lineDist = dist * 0.5f;
                glm::vec2 midPoint = bPos + normDir * (lineDist / 2.0f);

                matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(midPoint.x, midPoint.y, 0.0f));
                glm::mat4 matr_rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
                matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(lineDist, 2.0f, 1.0f));
                
                matr_final = matr_resize * matr_transl * matr_rot * matr_scale;
                glUniform1i(loc_codCol, 2);
                glUniform1f(loc_lungLinie, lineDist);
                glUniform4f(loc_matCol, 0.85f, 0.85f, 0.85f, 1.0f);
                glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            //tacul
            float cueLength = 220.0f; 
            float cueWidth = 4.5f; 
            float ballRadius = global::ballSize;

            //centrul bilei fata de centrul tacului (raportez rotatia la centrul tacului, nu la varful lui)
            float distFromBallCenter = ballRadius + cuePullOffset + (cueLength / 2.0f);
            glm::vec2 cueCenter = bPos - normDir * distFromBallCenter;

            glm::mat4 cueTransl = glm::translate(glm::mat4(1.0f), glm::vec3(cueCenter.x, cueCenter.y, 0.0f));
            glm::mat4 cueRot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 cueScale = glm::scale(glm::mat4(1.0f), glm::vec3(cueLength, cueWidth, 1.0f));

            matr_final = matr_resize * cueTransl * cueRot * cueScale;
            glUniform1i(loc_codCol, -1); // codCol 2 = culoare solida din matCol
            glUniform4f(loc_matCol, 0.76f, 0.48f, 0.22f, 1.0f); // Culoare lemn tac
            glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
    }

>>>>>>> Stashed changes

    //Verificare daca bilele au intrat in buzunare
    {
        float pocketRadius = global::pocketSize; // raza găurii

        auto& shapes = physics.getShapes();
        for (int i = (int)shapes.size() - 1; i >= 0; i--) {
            Vec2f bPos = shapes[i]->getPosition();
            glm::vec2 ballPos(bPos.x, bPos.y);

            bool inPocket = false;
            for (const auto& p : pockets) {
                float dist = glm::distance(ballPos, p);
                if (dist < pocketRadius) {
                    inPocket = true;
                    break;
                }
            }

            if (inPocket) {
                //Bila alba are indexul 0
                if (i == 0) {
                    // Exemplu de resetare pentru bila albă:
                    shapes[i]->setPosition({global::tableMinX + table_w * 0.25f, table_center_y});
                    shapes[i]->setSpeed({0.0f, 0.0f});
                    aiming = false;
                    cueStriking = false;
                    continue; //Nu stergem bila alba
                }

                shapes[i]->setPosition({-9999.0f, -9999.0f});
                shapes[i]->setSpeed({0.0f, 0.0f});
                }
            }
        }





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

    glutMouseFunc(mouseClick);     
    glutMotionFunc(mouseMotion);
    glutCloseFunc(Cleanup);				//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();
	return 0;
}