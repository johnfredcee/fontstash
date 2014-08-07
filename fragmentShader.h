#ifndef __FONTSTASH_FRAGMENTSHADER_H
#define __FONTSTASH_FRAGMENTSHADER_H

const char fragmentShader[] = "#version 330 core\n\
// Interpolated values from the vertex shaders\n\
in vec2 UV;\n\
// Ouput data\n\
out vec3 color;\n\
// Values that stay constant for the whole mesh.\n\
uniform sampler2D myTextureSampler;\n\
void main() {\n\
    // Output color = color of the texture at the specified UV\n\
    color = texture2D( myTextureSampler, UV ).rgb;\n\
}";

#endif /* __FONTSTASH_FRAGMENTSHADER_H */
