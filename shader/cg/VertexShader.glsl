#version 410



// Structures
// =============================================================================================================
struct Matrices
{
	mat4 mvp;
	mat4 mv;
	mat3 normal;

	mat2 uv;
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
	vec2 rotation = uvs + vec2(cos(float(animStage)* 0.1f) , sin(float(animStage)* 0.1f));
	Output.uv =   (matrices.uv * (uvs - 0.5f)) + 0.5f; //rotation; // dot(uvs, uvs); // uvs + vec2(int(animStage) * 0.04f, 0.f);
}

// Subroutine Implementation
// =============================================================================================================
subroutine (VertexProgram) void rotation()
{
    gl_Position = matrices.mvp * vertex;

	vec4 h = matrices.mv * vertex;
    vec3 mvPos = h.xyz / h.w;

	Output.normal = matrices.normal * normal;
    Output.lightDir = light.lightPos - mvPos;
	Output.viewDir = -mvPos; //+ vec3(cos(float(animStage)* 0.1f) , sin(float(animStage)* 0.1f), 0);
	Output.uv = uvs;// (matrices.uv * (uvs - 0.5f)) + 0.5f; // + vec2(cos(float(animStage)* 0.1f) , sin(float(animStage)* 0.1f)); 
}
// =============================================================================================================