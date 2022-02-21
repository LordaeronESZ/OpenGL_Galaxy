# version 430

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 vertNorm;

out vec2 tc;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
layout (binding = 0) uniform sampler2D samp;

void main() {
	gl_Position = proj_matrix * mv_matrix * vec4(vertPos, 1.0);
	tc = texCoord;
}