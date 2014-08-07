#ifndef __FONTSTASH_FRAGMENTSHADER_H
#define __FONTSTASH_FRAGMENTSHADER_H

const char fragmentShader[] = "#version 330 core\n\
\n\
// Interpolated values from the vertex shaders\n\
in vec2 UV;\n\
\n\
// Ouput data\n\
out vec3 color;\n\
\n\
// Values that stay constant for the whole mesh.\n\
uniform sampler2D myTextureSampler;\n\
\n\
void main() {\n\
    // Output color = color of the texture at the specified UV\n\
    color = texture(myTextureSampler, UV).rrr;\n\
}";

#endif /* __FONTSTASH_FRAGMENTSHADER_H */
