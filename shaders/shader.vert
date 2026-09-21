# version 460 core
layout ( location = 0) in vec3 aPos ;
layout ( location = 1) in vec3 aNormal ;
layout ( location = 2) in vec2 aTexCoords ;
out vec3 vNormal;
uniform mat4 uModel; // la matriz del modelo ACTUAL
uniform mat4 uAjuste; // caja negra hasta ver la Unidad VII
void main()
{
vNormal = aNormal;
gl_Position = uAjuste * uModel * vec4(aPos, 1.0);
}