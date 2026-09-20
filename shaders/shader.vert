# version 460 core
layout ( location = 0) in vec3 aPos ;
layout ( location = 1) in vec3 aNormal ;
layout ( location = 2) in vec2 aTexCoords ;
uniform mat4 uModel ; // <- la matriz de este modelo
uniform mat4 uAjuste ; // <- ajuste de vista y proyecci ón ( caja negra por ahora )
void main ()
{
gl_Position = uAjuste * uModel * vec4 ( aPos , 1.0) ;
}