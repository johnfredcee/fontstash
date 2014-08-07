#ifndef __FONTSTASH_VERTEXSHADER_H
#define __FONTSTASH_VERTEXSHADER_H

const char vertexShader[] = "#version 330 core\n\
\n\
// Input vertex data, different for all executions of this shader.\n\
layout(location = 0) in vec3 vertexPosition_modelspace;\n\
layout(location = 1) in vec2 vertexUV;\n\
\n\
// Output data ; will be interpolated for each fragment.\n\
out vec2 UV;\n\
\n\
// Values that stay constant for the whole mesh.\n\
uniform mat4 MVP;\n\
\n\
void main() {\n\
    // Output position of the vertex, in clip space : MVP * position\n\
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n\
    \n\
    // UV of the vertex. No special space for this one.\n\
    UV = vertexUV;\n\
}";

#endif /* __FONTSTASH_VERTEXSHADER_H */
