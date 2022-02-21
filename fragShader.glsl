# version 430

in vec2 tc;

out vec4 fragColor;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
layout (binding = 0) uniform sampler2D samp;

void main() {
	vec4 textureColor = texture(samp, tc);
	fragColor = textureColor;
}