# version 460 core
layout ( location = 0) in vec3 aPos ; // entra desde la config del VAO
layout ( location = 1) in vec3 aColor ;
out vec3 vColor ; // sale hacia el fragment shader
const mat3 kTransform = mat3 ( // ojo: GLSL recibe COLUMNAS
vec3 ( 0.3686 , -0.1454 , -0.3119) ,
vec3 ( 0.0000 , 0.5438 , -0.2536) ,
vec3 ( -0.2581 , -0.2077 , -0.4454) );
void main ()
{
vColor = aColor ;
gl_Position = vec4 ( kTransform * aPos , 1.0) ;
}