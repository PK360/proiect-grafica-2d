//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - 04_02_Shader.vert |
// ======================================
// 
//  Shaderul de varfuri / Vertex shader - afecteaza geometria scenei; 
//

#version 330

//  Variabile de intrare (dinspre programul principal);
layout (location = 0) in vec4 in_Position;     //  Se preia din buffer de pe prima pozitie (0) atributul care contine coordonatele;
layout (location = 1) in vec4 in_Color;        //  Se preia din buffer de pe a doua pozitie (1) atributul care contine culoarea;
layout (location = 2) in vec2 texCoord;        //  Se preia din buffer de pe a treia pozitie (2) atributul care contine textura;

//  Variabile de iesire;
out vec4 ex_Color;      //  Transmite culoarea (de modificat in Shader.frag); 
out vec2 tex_Coord;     //  Transmite textura (de modificat in Shader.frag); 
out vec2 local_Coord;   //  for circle

//  Variabile uniforme;
uniform mat4 matFinal;

void main(void) {
  gl_Position = matFinal * in_Position;
  ex_Color = in_Color;
  
  local_Coord = texCoord;
  tex_Coord = vec2(texCoord.x, (1.0 - texCoord.y));
}