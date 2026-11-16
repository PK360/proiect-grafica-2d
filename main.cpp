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
const char* shdr_vert = "vert_shader.vert", * shdr_frag = "frag_shader.frag";

// ids
GLuint id_vao, id_vbo, id_program;

// textures
std::vector<GLuint> text_balls(16);

// uniform locations
GLuint loc_codCol, loc_matFinal, loc_matCol, loc_uvOffset, loc_lungLinie;

// transform matrices
glm::mat4 matr_final, matr_resize, matr_scale, matr_transl, matr_rot;

// Physics Engine
PhysicsOOP physics;

// other variables
int last_t; bool active_turn = false, ballsMoving = false;
float cuePullOffset = 0.0f, strikePower = 0.0f, pocketDiameter = 2 * global::pocketSize;
glm::vec2 strikeDir{ 0.0f, 0.0f }, cur_mousePos{ 0.0f, 0.0f };

GameManager gameManager;



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

    //  Se creeaza / se leaga un VAO
    glGenVertexArrays(1, &id_vao);
    glBindVertexArray(id_vao);

    //  Se creeaza un buffer pentru VARFURI - COORDONATE, CULORI si TEXTURARE;
    glGenBuffers(1, &id_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, id_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // texturi bile
    for (int i = 0; i < 16; i++) {
        std::string path = "textures/balls/ball-" + std::to_string(i) + ".png";
        text_balls[i] = LoadTexture(path.c_str());
    }

    float D = 2 * global::ballSize;    //diametrul bilei
    float rowSpacing = D * 0.866025f; // sqrt(3) / 2, h triunghiului
    int ballIdx = 0;

    //bila alba
    physics.addCircle(new Ball(
        ballIdx, true,
        { global::tableMinX + global::table_w * 0.25f, global::table_center_y },
        { 0.0f, 0.0f }
    ));
    ballIdx++;

    float vX = global::tableMinX + global::table_w * 0.70f;     //coordonatele varfului de unde incepe triunghiul
    float vY = global::table_center_y;

    for (int i = 0; i < 5; i++) {
        float x = vX + i * rowSpacing;

        for (int j = 0; j <= i; j++) {
            float y = vY + (j - i / 2.0f) * D;

            physics.addCircle(new Ball(
                ballIdx, true,
                { x, y },
                { 0.0f, 0.0f }
            ));

            ballIdx++;
        }
    }

    id_program = LoadShaders(shdr_vert, shdr_frag);
    glUseProgram(id_program);

    loc_codCol = glGetUniformLocation(id_program, "codCol");
    loc_matCol = glGetUniformLocation(id_program, "matCol");
    loc_matFinal = glGetUniformLocation(id_program, "matFinal");
    loc_uvOffset = glGetUniformLocation(id_program, "uvOffset");
    loc_lungLinie = glGetUniformLocation(id_program, "lungLinie");

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

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &id_vao);

    text_balls.clear();
}



// draw table rectangle
void drawTable(float padding, glm::vec4 color) {
    glm::mat4 matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(global::table_center_x, global::table_center_y, 0.0f));
    glm::mat4 matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(global::table_w + padding, global::table_h + padding, 1.0f));
    glm::mat4 matr_final = matr_resize * matr_transl * matr_scale;

    glUniform1i(loc_codCol, -1);
    glUniform4fv(loc_matCol, 1, &color[0]);
    glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

// draw pocket circle
void drawPocket(float x, float y, float size) {
    glm::mat4 matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    glm::mat4 matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(size, size, 1.0f));
    glm::mat4 matr_final = matr_resize * matr_transl * matr_scale;

    glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
    glUniform1i(loc_codCol, 1);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

// draw ball from ui
void drawUIBall(int ballId, float ballSize, float x, float y) {
    glBindTexture(GL_TEXTURE_2D, text_balls[ballId]);
    glUniform2f(loc_uvOffset, 0.0f, 0.0f);
    
    glm::mat4 bt = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    glm::mat4 bs = glm::scale(glm::mat4(1.0f), glm::vec3(ballSize * 2.0f, ballSize * 2.0f, 1.0f));
    glm::mat4 bFin = matr_resize * bt * bs;
    
    glUniform1i(loc_codCol, 0); 
    glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &bFin[0][0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
};

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
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawUI() {
    float ui_Y = global::tableMaxY + 150.0f, ballSize = global::ballSize * 0.8f; 
    float p1_startX = global::tableMinX + 50.0f, p2_startX = global::tableMaxX - 50.0f;

    // Helper lambda to draw a cue stick
    auto drawCue = [&](float x, float y) {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
        glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(80.0f, 6.0f, 1.0f));
        glm::mat4 fin = matr_resize * t * s;
        glUniform1i(loc_codCol, -1);
        glUniform4f(loc_matCol, 0.76f, 0.48f, 0.22f, 1.0f);
        glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &fin[0][0]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    };

    // 1. Draw Cue Sticks for BOTH players permanently
    drawCue(p1_startX, ui_Y - 30.0f);
    drawCue(p2_startX, ui_Y - 30.0f);

    // 2. Draw Active Turn Marker (A red diamond pointing down)
    float markerX = (gameManager.cur_player == 1) ? p1_startX : p2_startX;
    glm::mat4 t_mark = glm::translate(glm::mat4(1.0f), glm::vec3(markerX, ui_Y - 15.0f, 0.0f));
    glm::mat4 r_mark = glm::rotate(glm::mat4(1.0f), 0.785398f, glm::vec3(0.0f, 0.0f, 1.0f)); // 45 degrees
    glm::mat4 s_mark = glm::scale(glm::mat4(1.0f), glm::vec3(12.0f, 12.0f, 1.0f));
    glm::mat4 fin_mark = matr_resize * t_mark * r_mark * s_mark;
    
    glUniform1i(loc_codCol, -1);
    glUniform4f(loc_matCol, 1.0f, 0.0f, 0.0f, 1.0f); // Red
    glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &fin_mark[0][0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // 3. Draw Assigned Ball Types
    if (gameManager.p1_type != -1) {
        // Draw them much smaller (ballSize * 0.6f) and further out so they don't look like captured balls
        drawUIBall((gameManager.p1_type == 1) ? 2 : 10, ballSize * 0.6f, p1_startX - 65.0f, ui_Y);
        drawUIBall((gameManager.p2_type == 1) ? 2 : 10, ballSize * 0.6f, p2_startX + 65.0f, ui_Y);
    }

    // 4. Draw Captured Balls
    for (size_t i = 0; i < gameManager.p1_balls.size(); i++) {
        drawUIBall(gameManager.p1_balls[i], ballSize, p1_startX + (i * 30.0f), ui_Y);
    }
    for (size_t i = 0; i < gameManager.p2_balls.size(); i++) {
        drawUIBall(gameManager.p2_balls[i], ballSize, p2_startX - (i * 30.0f), ui_Y);
    }
}



//  Functia de desenare
void render(void) {
    int now_t = glutGet(GLUT_ELAPSED_TIME);
    float frameDt = (now_t - last_t) / 1000.0f;
    float deltatime = frameDt;
    last_t = now_t;
    
    // 1. Pass the game manager to the sub-step loop
    const float step = 0.005f;
    while (deltatime > 0.0f) {
        physics.update(std::min(deltatime, step), gameManager);
        deltatime -= step;
    }

    // 2. Check if balls have completely stopped to end the turn
    ballsMoving = false;
    for (Ball* b : physics.getShapes()) {
        if (b->isActive() && b->getSpeed().sqrMagnitude() > 0.1f) {
            ballsMoving = true;
            break;
        }
    }

    if (active_turn && !ballsMoving && !global::cueStriking) {
        gameManager.endTurn();
        active_turn = false; 
    }

    // daca tacul este in miscare inspre bila (calculeaza pozitia)
    if (global::cueStriking && !physics.getShapes().empty()) {
        cuePullOffset -= (std::max(600.0f, strikePower * 1.5f)) * frameDt;
        
        //cand offset<=0, adica vf tacului atinge bila, el o va lovi
        if (cuePullOffset <= 0.0f) {
            cuePullOffset = 0.0f;
            global::cueStriking = false;
            physics.getShapes()[0]->setSpeed({ strikeDir.x * strikePower, strikeDir.y * strikePower });
        }
    }
    
    // ---------------------------------------------------------
    // CALCULATE AND DISPLAY FPS IN WINDOW TITLE
    // ---------------------------------------------------------
    // static int frameCount = 0;
    // static int lastFpsTime = 0;
    
    // frameCount++;
    // if (now_t - lastFpsTime >= 1000) { // Update once every second (1000 ms)
    //     // Calculate the exact FPS
    //     float fps = frameCount * 1000.0f / (now_t - lastFpsTime);
    
    //     // Build the new title string
    //     std::string title = "2D Billiards - FPS: " + std::to_string((int)fps);
    
    //     // Update the freeglut window
    //     glutSetWindowTitle(title.c_str());
    
    //     // Reset counters for the next second
    //     frameCount = 0;
    //     lastFpsTime = now_t;
    // }
    // ---------------------------------------------------------
    
    
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(id_program);
    glBindVertexArray(id_vao);
    
    
    
    // masa de biliard
    drawTable(67.5f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));    // border negru la margine
    drawTable(65.0f, glm::vec4(0.5f, 0.35f, 0.035f, 1.0f)); // margine de "lemn"
    drawTable(2.5f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));     // border la masa principala
    drawTable(0.0f, glm::vec4(0.1f, 0.4f, 0.2f, 1.0f));     // masa principala




    // gaurile mesei din colturi
    drawPocket(global::pockets[0].x, global::pockets[0].y, pocketDiameter); // stanga sus
    drawPocket(global::pockets[1].x, global::pockets[1].y, pocketDiameter); // stanga jos
    drawPocket(global::pockets[2].x, global::pockets[2].y, pocketDiameter); // centru sus
    drawPocket(global::pockets[3].x, global::pockets[3].y, pocketDiameter); // centru jos
    drawPocket(global::pockets[4].x, global::pockets[4].y, pocketDiameter); // dreapta sus
    drawPocket(global::pockets[5].x, global::pockets[5].y, pocketDiameter); // dreapta jos



    // aim line
    if ((global::aiming || global::cueStriking) && !physics.getShapes().empty()) {
        Vec2f ballPos = physics.getShapes()[0]->getPosition();
        glm::vec2 bPos(ballPos.x, ballPos.y);

        glm::vec2 dir = (global::aiming) ? (bPos - cur_mousePos) : (strikeDir * (strikePower / 3.5f));
        float dist = glm::length(dir);

        if (dist > 5.0f || global::cueStriking) {
            glm::vec2 normDir = (global::aiming) ? glm::normalize(dir) : strikeDir;
            float angle = atan2(normDir.y, normDir.x);

            if (global::aiming) {
                float lineDist = dist * 0.5f;
                cuePullOffset = std::min(dist * 0.08f, 45.0f);
                glm::vec2 midPoint = bPos + normDir * (lineDist / 2.0f);

                matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(midPoint.x, midPoint.y, 0.0f));
                matr_rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
                matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(lineDist, 2.0f, 1.0f)); 
                matr_final = matr_resize * matr_transl * matr_rot * matr_scale;

                glUniform1i(loc_codCol, 2);
                glUniform1f(loc_lungLinie, lineDist);
                glUniform4f(loc_matCol, 0.85f, 0.85f, 0.85f, 1.0f);
                glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        }
    }



    // bilele de biliard
    glUniform1i(loc_codCol, 0);
    glActiveTexture(GL_TEXTURE0);

    float ball_diam = global::ballSize * 2.0f;
    matr_scale = glm::scale(glm::mat4(1.0f), glm::vec3(ball_diam, ball_diam, 1.0f));

    const std::vector<Ball*>& shapes = physics.getShapes();
    for (unsigned int i = 0; i < shapes.size(); i++) {
        // UNCOMMENT THIS: Only draw active balls on the main table
        if (shapes[i]->isActive()) { 
            glBindTexture(GL_TEXTURE_2D, text_balls[i]);

            glUniform2f(loc_uvOffset, shapes[i]->getUVOffset().x, shapes[i]->getUVOffset().y);

            matr_transl = glm::translate(glm::mat4(1.0f), glm::vec3(shapes[i]->getPosition().x, shapes[i]->getPosition().y, 0.0f));
            matr_final = matr_resize * matr_transl * matr_scale;

            glUniformMatrix4fv(loc_matFinal, 1, GL_FALSE, &matr_final[0][0]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }
    
    drawUI();
    


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
    if (global::aiming) {
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
        if (state == GLUT_DOWN && !global::cueStriking) {
            //putem lovi doar daca bila sta pe loc
            if (!ballsMoving) {
                global::aiming = true;
                mouseMotion(x, y);
            }
        }
        else if (state == GLUT_UP && global::aiming) {
            // loveste mingea
            global::aiming = false;
            if (!physics.getShapes().empty()) {
                Vec2f ballPos = physics.getShapes()[0]->getPosition();
                glm::vec2 bPos(ballPos.x, ballPos.y);

                // calculam forta + directia
                glm::vec2 dir = bPos - cur_mousePos;
                float dist = glm::length(dir);
                if (dist > 5.0f) {
                    strikeDir = glm::normalize(dir);
                    float powerFactor = 3.5f;
                    strikePower = dist * powerFactor;

                    global::cueStriking = true; active_turn = true;
                }
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