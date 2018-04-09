#include <bv/BVRenderer.h>

#include <iostream>
#include <algorithm>
#include <experimental/filesystem>
#include <sstream>
#include <AntTweakBar.h>


namespace cgbv
{
	void TW_CALL handleScreenshot(void *data)
	{
		BVRenderer *renderer = reinterpret_cast<BVRenderer*>(data);
		renderer->capture();
	}


    BVRenderer::BVRenderer(GLFWwindow *window) : Renderer(window)
    {

    }


    BVRenderer::~BVRenderer()
    {

    }


	void BVRenderer::capture()
	{
		screenshot.set();
	}


    void BVRenderer::destroy()
    {
        glDeleteBuffers(1, &canvas.vbo);
        glDeleteVertexArrays(1, &canvas.vao);
        glDeleteSamplers(1, &sampler);
        glDeleteFramebuffers(2, fbo);
    }


    void BVRenderer::resize(int width, int height)
    {
        window_width = width;
        window_height = height;

        glViewport(0, 0, width, height);

        projection = glm::ortho(-1.f, 1.f, -1.f, 1.f, -10.f, 10.f);

        TwWindowSize(width, height > 0 ? height : 1);
    }


    void BVRenderer::input(int key, int scancode, int action, int modifiers)
    {
        switch(key)
        {
            case GLFW_KEY_UP:
                if(action == GLFW_PRESS)
                    model *= glm::rotate(glm::mat4(1.f), (float) M_PI / 6.f, glm::vec3(1.f, 0.f, 0.f));
                break;

            case GLFW_KEY_DOWN:
                if(action == GLFW_PRESS)
                    model *= glm::rotate(glm::mat4(1.f), -(float) M_PI / 6.f, glm::vec3(1.f, 0.f, 0.f));
                break;

            case GLFW_KEY_LEFT:
                if(action == GLFW_PRESS)
                    model *= glm::rotate(glm::mat4(1.f), (float) M_PI / 6.f, glm::vec3(0.f, 1.f, 0.f));
                break;

            case GLFW_KEY_RIGHT:
                if(action == GLFW_PRESS)
                    model *= glm::rotate(glm::mat4(1.f), -(float) M_PI / 6.f, glm::vec3(0.f, 1.f, 0.f));
                break;

            default:
                break;
        }
    }


    bool BVRenderer::setup()
    {
        glfwGetFramebufferSize(window, &window_width, &window_height);

//        if(!gl::sys::LoadFunctions())
//            return false;

        if(!gladLoadGL())
            return false;


        glClearColor(0.6f, 0.6f, 0.6f, 1.f);


        projection = glm::ortho(-1.f, 1.f, -1.f, 1.f, -10.f, 10.f);
		model = glm::mat4(1.f);


        shader = std::make_unique<cgbv::shader::GLSLShaderprogram>("../shader/bv/VertexShader.glsl", "../shader/bv/FragmentShader.glsl");
        locs.vertex = shader->getAttribLocation("vertex");
        locs.uv = shader->getAttribLocation("uv");
        locs.mvp = shader->getUniformLocation("matrices.mvp");
        locs.tex = shader->getUniformLocation("textures.tex");
        locs.parameterA = shader->getUniformLocation("parameter.paramA");
        locs.parameterB = shader->getUniformLocation("parameter.paramB");
        locs.subVertex = shader->getSubroutineIndex(GL_VERTEX_SHADER, "passthrough");
        locs.subFragment = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "passthrough");


        glGenVertexArrays(1, &canvas.vao);
        glBindVertexArray(canvas.vao);

        std::vector<float> verts =
                {
                        -1.f, -1.f, 0.f,
                        1.f, -1.f, 0.f,
                        -1.f, 1.f, 0.f,
                        1.f, 1.f, 0.f
                };
        std::vector<float> uv =
                {
                        0.f, 0.f,
                        1280.f, 0.f,
                        0.f, 720.f,
                        1280.f, 720.f
                };

        glGenBuffers(1, &canvas.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, canvas.vbo);
        glBufferData(GL_ARRAY_BUFFER, (verts.size() + uv.size()) * sizeof(float), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(float), verts.data());
        glBufferSubData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), uv.size() * sizeof(float), uv.data());

        glEnableVertexAttribArray(locs.vertex);
        glVertexAttribPointer(locs.vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(locs.uv);
        glVertexAttribPointer(locs.uv, 2, GL_FLOAT, GL_FALSE, 0, (const void *) (size_t)(verts.size() * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);




        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        std::vector<std::string> images;
        std::string texturePath = "../textures/bv/";
		for (auto &p : std::experimental::filesystem::directory_iterator(texturePath))
			images.push_back(p.path().generic_string());

        texture = std::make_unique<cgbv::textures::Texture2DRect[]>(images.size());

        for(unsigned int i = 0; i < images.size(); ++i)
            texture[i].Generate(images[i]);




        // Render Target
        framebufferTexture = std::make_unique<cgbv::textures::Texture2DRect>();

        glBindTexture(GL_TEXTURE_RECTANGLE, framebufferTexture->getTextureID());
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		secondframebufferTexture = std::make_unique<cgbv::textures::Texture2DRect>();

		glBindTexture(GL_TEXTURE_RECTANGLE, secondframebufferTexture->getTextureID());
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



        glGenFramebuffers(2, fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebufferTexture->getTextureID(), 0);
        GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, drawBuffers);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer 1 kaputt!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, secondframebufferTexture->getTextureID(), 0);
		glDrawBuffers(1, drawBuffers);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer 2 kaputt!" << std::endl;




        // GUI
        TwInit(TW_OPENGL_CORE, nullptr);
        TwWindowSize(1280, 720);
        TwBar *tweakbar = TwNewBar("TweakBar");
        TwDefine(" TweakBar size='300 600'");
		TwDefine(" GLOBAL fontsize=3 ");

        std::vector<TwEnumVal> imgTypeData;
        for(auto &p : images)
        {
            TwEnumVal value;

            auto index = std::find(images.begin(), images.end(), p) - images.begin();

            value.Value = index;
            value.Label = p.c_str();
            imgTypeData.push_back(value);
        }
        TwType imgType = TwDefineEnum("ImageType", imgTypeData.data(), imgTypeData.size());
		TwAddVarRW(tweakbar, "Image", imgType, &uiParams.activeTexture, nullptr);

		TwAddButton(tweakbar, "Take Screenshot", handleScreenshot, this, nullptr);

		TwAddVarRW(tweakbar, "Passes", TW_TYPE_INT32, &uiParams.numberPasses, "min='1' max='4' group='Render Stages'");

		const TwEnumVal bvoperations[] = { { 0, "Passthrough" },{ 1, "Bewegter Mittelwert" },{ 2, "Helligkeit/Kontrast" },{ 3, "Sharpen" } ,{ 4, "Dilatation" },{ 5, "Erosion" },{ 6, "Gauss 3x3" },{ 7, "Gauss 5x5" },{ 8, "Gauss 7x7" } ,{ 9, "Gauss 7x7 Vertikal" } ,{ 10, "Gauss 7x7 Horizontal" },{ 11, "Sobel" } ,{ 12, "Laplace" }, {13, "Median"} };
		TwType optype = TwDefineEnum("Operator", bvoperations, 14);
		TwAddVarRW(tweakbar, "Operation Pass #1", optype, &uiParams.passOps[0], "group='Render Stages'");
		TwAddVarRW(tweakbar, "Operation Pass #2", optype, &uiParams.passOps[1], "group='Render Stages'");
		TwAddVarRW(tweakbar, "Operation Pass #3", optype, &uiParams.passOps[2], "group='Render Stages'");
		TwAddVarRW(tweakbar, "Operation Pass #4", optype, &uiParams.passOps[3], "group='Render Stages'");

		TwAddVarRW(tweakbar, "Param #1 X", TW_TYPE_FLOAT, &uiParams.one.x, "group='Op Parameter'");
		TwAddVarRW(tweakbar, "Param #1 Y", TW_TYPE_FLOAT, &uiParams.one.y, "group='Op Parameter'");
		TwAddVarRW(tweakbar, "Param #1 Z", TW_TYPE_FLOAT, &uiParams.one.z, "group='Op Parameter'");
		TwAddVarRW(tweakbar, "Param #1 W", TW_TYPE_FLOAT, &uiParams.one.w, "group='Op Parameter'");
		TwAddVarRW(tweakbar, "Param #2 X", TW_TYPE_FLOAT, &uiParams.two.x, "group='Op Parameter'");
		TwAddVarRW(tweakbar, "Param #2 Y", TW_TYPE_FLOAT, &uiParams.two.y, "group='Op Parameter'");
		TwAddVarRW(tweakbar, "Param #2 Z", TW_TYPE_FLOAT, &uiParams.two.z, "group='Op Parameter'");
		TwAddVarRW(tweakbar, "Param #2 W", TW_TYPE_FLOAT, &uiParams.two.w, "group='Op Parameter'");


        framecounter = std::make_unique<statistics::FrameCounter>();

        return true;
    }


    void BVRenderer::render()
    {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        shader->use();
        glUniformMatrix4fv(locs.mvp, 1, GL_FALSE, glm::value_ptr(projection * model));

        glUniform4fv(locs.parameterA, 1, glm::value_ptr(uiParams.one));
        glUniform4fv(locs.parameterB, 1, glm::value_ptr(uiParams.two));

        glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &locs.subVertex);


        auto renderpass = [&](unsigned int tex, unsigned int pass)
        {
            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &uiParams.passOps[pass]);

			glActiveTexture(GL_TEXTURE0);
            glBindSampler(0, sampler);
            glBindTexture(GL_TEXTURE_RECTANGLE, tex);
            glUniform1i(locs.tex, 0);

            glBindVertexArray(canvas.vao);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        };


        switch (uiParams.numberPasses)
        {
            case 1:
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, window_width, window_height);
                renderpass(texture[uiParams.activeTexture].getTextureID(), 0);
                break;
            case 2:
                glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
                glViewport(0, 0, 1280, 720);
                renderpass(texture[uiParams.activeTexture].getTextureID(), 0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, window_width, window_height);
                renderpass(framebufferTexture->getTextureID(), 1);
                break;
            case 3:
                glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
                glViewport(0, 0, 1280, 720);
                renderpass(texture[uiParams.activeTexture].getTextureID(), 0);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
                renderpass(framebufferTexture->getTextureID(), 1);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, window_width, window_height);
                renderpass(secondframebufferTexture->getTextureID(), 2);
                break;
            case 4:
                glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
                glViewport(0, 0, 1280, 720);
                renderpass(texture[uiParams.activeTexture].getTextureID(), 0);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
                renderpass(framebufferTexture->getTextureID(), 1);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
                renderpass(secondframebufferTexture->getTextureID(), 2);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, window_width, window_height);
                renderpass(framebufferTexture->getTextureID(), 3);
                break;
            default:
                break;
        }



		if (screenshot[0])
		{
			std::cout << "Take Screenshot" << std::endl;
			std::unique_ptr<GLubyte[]> pixel = std::make_unique<GLubyte[]>(window_width * window_height * 3);
			glReadPixels(0, 0, window_width, window_height, GL_BGR, GL_UNSIGNED_BYTE, pixel.get());
			textures::Texture2DStorage::Store("../textures/Screenshot.png", pixel.get(), window_width, window_height, 0);
			screenshot.reset();
		}



        TwDraw();
        framecounter->tick(window);
    }


    void BVRenderer::update()
    {

    }
}