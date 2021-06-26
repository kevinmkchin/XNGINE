#version 330
// fragment shader is not necessary
void main()
{
    gl_FragDepth = gl_FragCoord.z;
}