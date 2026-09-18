# version 460 core
in vec3 vColor ; // atributo que entra , se lee en vertex shader
out vec4 FragColor ;
void main ()
{
FragColor = vec4 ( vColor , 1.0) ;
}