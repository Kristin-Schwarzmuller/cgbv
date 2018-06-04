#pragma once

#include <Renderer.h>
#include <memory>
#include <vector>
#include <GLSLShader.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <Texture.h>
#include <FrameCounter.h>
#include <Camera.h>
#include <chrono>


namespace cgbv
{
    struct ShaderLocations
    {
		unsigned int vertex, normal, uv;

        unsigned int modelViewProjection, normalmatrix, modelview;

        unsigned int subVertex, subFragment;

		unsigned int lightPos;

		unsigned int ambientLight;

		unsigned int diffusLight;

		unsigned int spekularLight;

		unsigned int ambientMaterial;

		unsigned int diffusMaterial;

		unsigned int spekularMaterial;

		unsigned int emissivMaterial;

		unsigned int shininessMaterial;

		unsigned int texture;
		
		unsigned int animationUVs;
    };

    struct BufferCombo
    {
        unsigned int vao, vbo, vertsToDraw;
    };

	struct UIParameter
	{
		glm::quat globalRotation;
        glm::vec4 lightPos = glm::vec4(0.f, 0.f, 15.f, 1.f);
		glm::vec4 ambientLight = glm::vec4(0.3f, 0.1f, 0.2f, 1.f);
		glm::vec4 diffusLight = glm::vec4(1.f, 1.f, 1.f, 1.f);
		glm::vec4 specularLight = glm::vec4(1.f, 1.f, 1.f, 1.f);
		glm::vec4 ambientMaterial = glm::vec4(0.7f, 0.4f, 0.8f, 0.8f);
		glm::vec4 diffusMaterial = glm::vec4(0.7f, 0.4f, 0.8f, 0.7f);
		glm::vec4 spekularMaterial = glm::vec4(1.f, 1.f, 1.f, 1.f);
		glm::vec4 emissivMaterial = glm::vec4(0.f, 0.f, 0.f, 0.f);
		float shininessMaterial = 16.f;


        float f;
	};

	


	class CGRenderer : public Renderer
	{
		std::unique_ptr<cgbv::textures::Texture2D> texture;
		unsigned int sampler;

		float animStage = 0.f;
		std::chrono::high_resolution_clock::time_point last;
        std::unique_ptr<cgbv::shader::GLSLShaderprogram> shader;

        ShaderLocations locs;

		BufferCombo cone, disk;

        glm::mat4 projection, model;

        glm::mat3 normal;

		UIParameter parameter;

        cgbv::Camera camera;

	public:
        CGRenderer(GLFWwindow *window);
		~CGRenderer(void);

		virtual void destroy();
		virtual void resize(int width, int height);
		virtual void input(int key, int scancode, int action, int modifiers);
		std::vector<glm::vec3> tessellate(std::vector<glm::vec3>, int depth);
		virtual bool setup();
		virtual void render();
		virtual void update();
	};
}