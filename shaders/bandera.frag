#version 460 core
in vec2 vUV;
uniform float uCuadros; // cuadros por lado
uniform float uAlfa;    // 0 = invisible, 1 = opaco
out vec4 fragColor;

void main()
{
    vec2 celda = floor(vUV * uCuadros);          // en que cuadro estoy (entero)
    float blanco = mod(celda.x + celda.y, 2.0);  // alterna 0/1 como un tablero
    fragColor = vec4(vec3(blanco), uAlfa);
}