#pragma once

#include <Renderer.h>
#include <memory>
#include <vector>
#include <GLSLShader.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <Texture.h>
#include <FrameCounter.h>
#include <bitset>

namespace cgbv
{
    struct ShaderLocations
    {
        unsigned int vertex, uv;

        unsigned int mvp;

        unsigned int subVertex, subFragment;

        unsigned int tex;

        unsigned int parameterA, parameterB;
    };

    struct Canvas
    {
        unsigned int vao, vbo;
    };

	struct ProgramParameter
	{
		unsigned int activeTexture = 0;
		int numberPasses = 1;
		unsigned int passOps[4] = { 0, 0, 0, 0 };
		glm::vec4 one = glm::vec4(0.f, 0.f, 0.f, 0.f);
		glm::vec4 two = glm::vec4(0.f, 0.f, 0.f, 0.f);
	};


	class BVRenderer : public Renderer
	{
        std::unique_ptr<cgbv::shader::GLSLShaderprogram> shader;

        ShaderLocations locs;

        Canvas canvas;

		glm::mat4 projection, model;

        std::vector<cgbv::textures::Texture2DRect> texSet;
        std::unique_ptr<cgbv::textures::Texture2DRect[]> texture;
        std::unique_ptr<cgbv::textures::Texture2DRect> framebufferTexture;
        std::unique_ptr<cgbv::textures::Texture2DRect> secondframebufferTexture;

        unsigned int sampler;

        unsigned int fbo[2];

		ProgramParameter uiParams;

        std::unique_ptr<statistics::FrameCounter> framecounter;

		std::bitset<1> screenshot;

	public:
		BVRenderer(GLFWwindow *window);
		~BVRenderer(void);

		virtual void destroy();
		virtual void resize(int width, int height);
		virtual void input(int key, int scancode, int action, int modifiers);
		virtual bool setup();
		virtual void render();
		virtual void update();
		void capture();
	};
}
