//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - 04_04_Shader.frag |
// ======================================
// 
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
//

#version 330

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;
in vec2 tex_Coord;		//	Coordonata de texturare;
in vec2 local_Coord;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

//  Variabile uniforme;
uniform sampler2D text;
uniform int codCol;
uniform vec4 matCol;
uniform vec2 uvOffset;
uniform float lungLinie;

void main(void) {
  if (codCol == -1) { out_Color = matCol; } // color from uniform
  else if (codCol == 0) { // bilele de biliard
    // circle mask
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(local_Coord, center);

    if (dist > 0.5) { discard; }

    // texture scaling
    float scaleFactor = 0.65; 
    vec2 scaled_uv = (tex_Coord - vec2(0.5)) * scaleFactor + vec2(0.5);
    scaled_uv += uvOffset;

    vec4 texColor = texture(text, scaled_uv);
    
    // edge darkening
    float shadow = smoothstep(0.3, 0.5, dist); 
    texColor.rgb *= (1.0 - shadow * 0.6); // 60%

    out_Color = texColor;
  }
  else if (codCol == 1) { // gaurile mesei
    //vec4 texColor = texture(text, tex_Coord);
    //if (texColor.a < 0.1) { discard; }

    vec2 center = vec2(0.5, 0.5);
    float dist = distance(local_Coord, center);

    if (dist > 0.5) { discard; }

    out_Color = vec4(0.0, 0.0, 0.0, 1.0);
  }
  else if (codCol == 2) { // aim line
    float dist_along = tex_Coord.x * lungLinie;
    
    float visible_size = 20.0, gap_size = 20.0;
    if (mod(dist_along, visible_size + gap_size) > visible_size) { discard; }

    out_Color = matCol;
    out_Color.a *= 0.5; // transparency of the line
  }
} 