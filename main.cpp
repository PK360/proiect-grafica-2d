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
GLuint loc_codCol, loc_matFinal, loc_matCol, loc_uvOffset;

// transform matrices
glm::mat4 matr_final, matr_resize, matr_scale, matr_transl;

// Physics Engine
PhysicsOOP physics;

// other variables
int last_t; bool aiming = false;
glm::vec2 cur_mousePos{0.0f, 0.0f};

float table_w = global::tableMaxX - global::tableMinX, table_h = global::tableMaxY - global::tableMinY;
float table_center_x = (global::tableMinX + global::tableMaxX) / 2.0f;
float table_center_y = (global::tableMinY + global::tableMaxY) / 2.0f;



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
    loc_matCol = glGetUniformLocation(id_program, "matCol");
    loc_matFinal = glGetUniformLocation(id_program, "matFinal");
    loc_uvOffset = glGetUniformLocation(id_program, "uvOffset");

    matr_resize = glm::ortho(global::range_x.x, global::range_x.y, global::range_y.x, global::range_y.y);
    last_t = glutGet(GLUT_ELAPSED_TIME);
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



// draw table rectangle
void drawTable(float padding, glm::vec4 color) {
    glm::mat4 matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(table_center_x, table_center_y, 0.0f)); 
    glm::mat4 matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(table_w + padding, table_h + padding, 1.0f));
    glm::mat4 matr_final = matr_resize * matr_transl * matr_scale;
    
    glUniform1i(loc_codCol, 2);
    glUniform4fv(loc_matCol, 1, &color[0]);
    glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// draw pocket circle
void drawPocket(float x, float y, float size) {
    glm::mat4 matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    glm::mat4 matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(size, size, 1.0f));
    glm::mat4 matr_final = matr_resize * matr_transl * matr_scale;
    
    glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
    glUniform1i(loc_codCol, 1); 
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// draw line between two points
void drawLine(float x1, float y1, float x2, float y2) {
    float cx = (x1 + x2) / 2.0f, cy = (y1 + y2) / 2.0f;
    float len = std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    float angle = atan2(y2 - y1, x2 - x1);
    
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(cx, cy, 0.0f));
    glm::mat4 r = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(len, 2.0f, 1.0f)); 
    
    glm::mat4 fin = matr_resize * t * r * s;
    glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &fin[0][0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

//  Functia de desenare
void render(void) {
    int now_t = glutGet(GLUT_ELAPSED_TIME);
    float deltatime = (now_t - last_t) / 1000.0f;
    last_t = now_t;

    const float step = 0.005f;
    while (deltatime > 0.0f) {
        float deltastep = std::min(deltatime, step);
        physics.update(deltastep);
        deltatime -= step;
    }



    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(id_program);
    glBindVertexArray(id_vao);



    // masa de biliard
    drawTable(67.5f, glm::vec4(0.0f, 0.0f,  0.0f,   1.0f)); // border negru la margine
    drawTable(65.0f, glm::vec4(0.5f, 0.35f, 0.035f, 1.0f)); // margine de "lemn"
    drawTable(2.5f,  glm::vec4(0.0f, 0.0f,  0.0f,   1.0f)); // border la masa principala
    drawTable(0.0f,  glm::vec4(0.1f, 0.4f,  0.2f,   1.0f)); // masa principala
    



    // gaurile mesei din colturi
    float pocketDiameter = global::pocketSize * 2.0f;
    float min_x = global::tableMinX, max_x = global::tableMaxX;
    float min_y = global::tableMinY, max_y = global::tableMaxY;
    float mid_x = table_center_x;

    drawPocket(min_x, max_y, pocketDiameter); // stanga  sus
    drawPocket(min_x, min_y, pocketDiameter); // stanga  jos
    drawPocket(mid_x, max_y, pocketDiameter); // mijloc  sus
    drawPocket(mid_x, min_y, pocketDiameter); // mijloc  jos
    drawPocket(max_x, max_y, pocketDiameter); // dreapta sus
    drawPocket(max_x, min_y, pocketDiameter); // dreapta jos



    // bilele de biliard
    glUniform1i(loc_codCol, 0);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(id_program, "text"), 0);

    float ball_diam = global::ballSize * 2.0f;
    matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(ball_diam, ball_diam, 1.0f));

    const auto& shapes = physics.getShapes();
    for (size_t i = 0; i < shapes.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, text_balls[i]);
        
        glUniform2f(loc_uvOffset, shapes[i]->getUVOffset().x, shapes[i]->getUVOffset().y);

        matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(shapes[i]->getPosition().x, shapes[i]->getPosition().y, 0.0f));
        matr_final = matr_resize * matr_transl * matr_scale;
        
        glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }



    // aim line
    if (aiming && !physics.getShapes().empty()) {
        glUniform1i(loc_codCol, 4);
        glUniform4f(loc_matCol, 1.0, 0.5, 0.0, 1.0);
        
        Vec2f ballPos = physics.getShapes()[0]->getPosition();
        glm::vec2 bPos(ballPos.x, ballPos.y);
        
        glm::vec2 dir = cur_mousePos - bPos;
        float dist = glm::length(dir);
        
        if (dist > 5.0f) { 
            float lineDist = dist * 0.75f;            
            float angle = atan2(dir.y, dir.x); 
            
            glm::vec2 midPoint = bPos + glm::normalize(dir) * (lineDist / 2.0f);
            
            matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(midPoint.x, midPoint.y, 0.0f));
            glm::mat4 matr_rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
            matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(lineDist, 2.0f, 1.0f)); 
            
            matr_final = matr_resize * matr_transl * matr_rot * matr_scale;
            glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
            
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }



    // debug lines for the collision
    // glUniform1i(loc_codCol, 4);
    // glUniform4f(loc_matCol, 1.0, 0.5, 0.0, 1.0);

    // float pocketSize = global::pocketSize; 
    // min_x = global::tableMinX; max_x = global::tableMaxX;
    // min_y = global::tableMinY; max_y = global::tableMaxY;
    // mid_x = (min_x + max_x) / 2.0f;

    // drawLine(min_x, min_y + pocketSize, min_x, max_y - pocketSize); // bottom left
    // drawLine(max_x, min_y + pocketSize, max_x, max_y - pocketSize); // bottom right
    // drawLine(min_x + pocketSize, min_y, mid_x - pocketSize, min_y); // bottom middle
    // drawLine(mid_x + pocketSize, min_y, max_x - pocketSize, min_y); // top middle
    // drawLine(mid_x + pocketSize, max_y, max_x - pocketSize, max_y); // top right
    // drawLine(min_x + pocketSize, max_y, mid_x - pocketSize, max_y); // top left



    //glFlush();                           
    glutSwapBuffers();
    glutPostRedisplay();
}




void mouseMotion(int x, int y) {
    if (aiming) {
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);
        
        // Dynamically calculate based on your actual camera ranges
        float width = global::range_x.y - global::range_x.x;
        float height = global::range_y.y - global::range_y.x;
        
        // This perfectly syncs the mouse exactly to where your camera is looking
        cur_mousePos.x = global::range_x.x + ((float)x / (float)w) * width;
        cur_mousePos.y = global::range_y.x + (1.0f - ((float)y / (float)h)) * height;
    }
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // Start aiming (only if ball is completely stopped)
            //if (!physics.getShapes().empty() && physics.getShapes()[0]->getSpeed().magnitude() < 0.1f) {
                aiming = true;
                mouseMotion(x, y);
            //}
        } else if (state == GLUT_UP && aiming) {
            // Shoot the ball!
            aiming = false;
            if (!physics.getShapes().empty()) {
                Vec2f ballPos = physics.getShapes()[0]->getPosition();
                
                // Direction from Ball to Mouse
                float dirX = cur_mousePos.x - ballPos.x;
                float dirY = cur_mousePos.y - ballPos.y;
                
                // Multiply the drag distance by a power factor to get the speed
                float power = 3.5f; 
                physics.getShapes()[0]->setSpeed({dirX * power, dirY * power});
            }
        }
    }
}





//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);					//	Modul de afisare al ferestrei, se foloseste un singur buffer de afisare si culori RGB;
	glutInitWindowSize(global::win_w, global::win_h);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(global::winpos_x, global::winpos_y);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Indexarea varfurilor");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

    glewInit();
    
    Initialize();						//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
    glutDisplayFunc(render);	//  Desenarea scenei in fereastra;
    glutMouseFunc(mouseClick);      // Add this
    glutMotionFunc(mouseMotion);
    glutCloseFunc(Cleanup);				//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();
	return 0;
}