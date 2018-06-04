#version 410



// Structures
// =============================================================================================================
struct Matrices
{
	mat4 mvp;
	mat4 mv;
	mat3 normal;
};


struct Light
{
    vec3 lightPos;
	vec4 ambient;
	vec4 diffus;
	vec4 specular;
};


struct VertexOutput
{
    vec3 normal;
	vec3 lightDir;
	vec3 viewDir;
	vec2 uv;
};
// =============================================================================================================




// Subroutines
// =============================================================================================================
subroutine void VertexProgram();
// =============================================================================================================




// Variables
// =============================================================================================================
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvs;

uniform Matrices matrices;

uniform Light light;

uniform float animStage;

subroutine uniform VertexProgram vertexprogram;

layout(location = 0) out VertexOutput Output;
// =============================================================================================================




// Methods
// =============================================================================================================
void main()
{
	vertexprogram();
}
// =============================================================================================================




// Subroutine Implementation
// =============================================================================================================
subroutine (VertexProgram) void verts_and_normals()
{
    vec4 h = matrices.mv * vertex;
    vec3 mvPos = h.xyz / h.w;

	gl_Position = matrices.mvp * vertex;

    Output.normal = matrices.normal * normal;
    Output.lightDir = light.lightPos - mvPos;
    Output.viewDir = -mvPos;
	Output.uv = uvs + vec2(int(animStage) * 0.04f, 0.f);
}

// Subroutine Implementation
// =============================================================================================================
subroutine (VertexProgram) void simpleTrans()
{
    gl_Position = matrices.mvp * vertex;

	Output.normal = matrices.normal * normal;

	vec4 h = matrices.mv * vertex;
    vec3 mvPos = h.xyz / h.w;

    Output.lightDir = light.lightPos - mvPos;

}
// =============================================================================================================