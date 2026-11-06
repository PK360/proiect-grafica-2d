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
uniform sampler2D myTexture;
uniform int codCol;
uniform vec2 uvOffset;

void main(void) {
  if (codCol == 0) { out_Color = vec4(0.1, 0.4, 0.2, 1.0); }
  else if (codCol == 1) {
    // 1. Math-based circle mask (discards pixels outside the 0.5 radius)
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(local_Coord, center);

    if (dist > 0.5) { discard; }
    

    // 2. SCALE THE TEXTURE
    // Change this value to adjust the texture size! 
    // 0.5 = Zooms IN (takes a smaller half-diameter circle from the texture)
    // 2.0 = Zooms OUT (makes the texture look smaller on the ball)
    float scaleFactor = 0.65; 
    
    // Shift to origin, scale, then shift back to center
    vec2 scaled_uv = (tex_Coord - vec2(0.5)) * scaleFactor + vec2(0.5);
    
    scaled_uv += uvOffset;

    // Sample using the new scaled coordinates
    vec4 texColor = texture(myTexture, scaled_uv);
    
    // 3. Make it "ball-y" by darkening the outer edges (fake 3D volume)
    float shadow = smoothstep(0.3, 0.5, dist); 
    texColor.rgb *= (1.0 - shadow * 0.6); // Darkens up to 60% at the edge

    out_Color = texColor;
  }
  else {
    vec4 texColor = texture(myTexture, tex_Coord);
    if (texColor.a < 0.1) { discard; }
    
    out_Color = texColor;
  }
} 