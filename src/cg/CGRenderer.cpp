#include <cg/CGRenderer.h>

#include <iostream>
#include <AntTweakBar.h>



namespace cgbv
{
	CGRenderer::CGRenderer(GLFWwindow *window) : Renderer(window)
	{

	}


	CGRenderer::~CGRenderer()
	{

	}


	void CGRenderer::destroy()
	{
		glDeleteVertexArrays(1, &disk.vao);
		glDeleteBuffers(1, &disk.vbo);
		glDeleteVertexArrays(1, &cone.vao);
		glDeleteBuffers(1, &cone.vbo);
		glDeleteVertexArrays(1, &moon.vao);
		glDeleteBuffers(1, &moon.vbo);
		glDeleteSamplers(1, &sampler);
	}


	void CGRenderer::resize(int width, int height)
	{
		window_width = width;
		window_height = height;

		glViewport(0, 0, width, height);

		projection = glm::perspective(float(M_PI) / 5.f, float(window_width) / float(window_height), .1f, 200.f);

		TwWindowSize(width, height > 0 ? height : 1);
	}


	void CGRenderer::input(int key, int scancode, int action, int modifiers)
	{
		TwEventCharGLFW(key, action);
		TwEventKeyGLFW(key, action);

		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_W:
				camera.moveForward(.1f);
				break;
			case GLFW_KEY_S:
				camera.moveForward(-.1f);
				break;
			case GLFW_KEY_A:
				camera.moveRight(.1f);
				break;
			case GLFW_KEY_D:
				camera.moveRight(-.1f);
				break;
			default:
				break;
			}
		}
	}

	/** Rekursive Funtion zum tesselieren von Dreiecken
	* input: vertices: Vektor mit 3dim Vertices,
	* input: depth: Anzahl der Tesselierungstiefen 0 = keine --> Abbruchbedingung
	* output: vector, indem jedes Dreieck druch 4 neue Dreiecke ersetzt wird
	*/
	std::vector<glm::vec3> CGRenderer::tessellate(std::vector<glm::vec3> vertices, int depth)
	{
		// Ende der Tesselierungstiefe erreicht? 
		if (depth == 0) return vertices;

		// Unterteilung jedes Dreiecks in vier neue Dreiecke: Aus je 3 Vertices werden 12
		std::vector<glm::vec3> verticesExtended;

		/* So lange min 3 Vertices sich noch im Eingabevector befinden,
		* werden aus diesen drei Punken, drei Punkte zwischen drin berechnet.
		* Daraus resultierend, werden die neuen 4 Dreiecke berechnet und in korreter Reihenfolge ins ausgabe Array geschrieben*/
		for (unsigned int i = 0; i <= vertices.size() - 3; i += 3) {
			// Auslesen der nächsten drei Punkte
			glm::vec3 a = vertices[i];
			glm::vec3 b = vertices[i + 1];
			glm::vec3 c = vertices[i + 2];

			// Berechnen der Zwischenpunkte
			glm::vec3 b2 = (a + (c - a) * 0.5f);
			b2 = b2 / glm::length(b2)*(glm::length(glm::vec3(1, 1, 1)));
			glm::vec3 c2 = (a + (b - a) * 0.5f);
			c2 = c2 / glm::length(c2)*(glm::length(glm::vec3(1, 1, 1)));
			glm::vec3 a2 = (c + (b - c) * 0.5f);
			a2 = a2 / glm::length(a2)*(glm::length(glm::vec3(1, 1, 1)));

			// Zurück schreiben ins Ausgabearray
			verticesExtended.push_back(a);
			verticesExtended.push_back(b2);
			verticesExtended.push_back(c2);

			verticesExtended.push_back(b2);
			verticesExtended.push_back(c);
			verticesExtended.push_back(a2);

			verticesExtended.push_back(a2);
			verticesExtended.push_back(b);
			verticesExtended.push_back(c2);

			verticesExtended.push_back(a2);
			verticesExtended.push_back(c2);
			verticesExtended.push_back(b2);
		}
		// Rekursiver Aufruf --> wenn depth dann noch großer 0 --> weiter
		return tessellate(verticesExtended, --depth);
	}


	bool CGRenderer::setup()
	{
		glfwGetFramebufferSize(window, &window_width, &window_height);


		if (!gladLoadGL())
			return false;


		// GL States
		glClearColor(0.4f, 0.4f, 0.4f, 1.f);

		glEnable(GL_ALPHA_TEST);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		projection = glm::perspective(float(M_PI) / 5.f, float(window_width) / float(window_height), .1f, 200.f);
		camera.setTarget(glm::vec3(0.f, 0.f, 0.f));
		camera.moveTo(0.f, -.5f, 5.f);

		// Shader
		shader = std::make_unique<cgbv::shader::GLSLShaderprogram>("../shader/cg/VertexShader.glsl", "../shader/cg/FragmentShader.glsl");
		locs.vertex = shader->getAttribLocation("vertex");
		locs.normal = shader->getAttribLocation("normal");
		locs.uv = shader->getAttribLocation("uvs");
		locs.modelViewProjection = shader->getUniformLocation("matrices.mvp");
		locs.normalmatrix = shader->getUniformLocation("matrices.normal");
		locs.modelview = shader->getUniformLocation("matrices.mv");
		locs.uvMatrix = shader->getUniformLocation("matrices.uv");
		locs.lightPos = shader->getUniformLocation("light.lightPos");
		locs.ambientLight = shader->getUniformLocation("light.ambient");
		locs.ambientMaterial = shader->getUniformLocation("material.ambient");
		locs.diffusLight = shader->getUniformLocation("light.diffus");
		locs.diffusMaterial = shader->getUniformLocation("material.diffus");
		locs.emissivMaterial = shader->getUniformLocation("material.emissiv");
		locs.spekularLight = shader->getUniformLocation("light.specular");
		locs.spekularMaterial = shader->getUniformLocation("material.spekular");
		locs.shininessMaterial = shader->getUniformLocation("material.shininess");
		locs.texture = shader->getUniformLocation("textures.tex");
		//locs.animationUVs = shader->getUniformLocation("animStage");


		// Geometrie
		//-------------------------------------------------------------------------------------------------------------------------------
		// Kegel
		// locs.subFragment = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "toon");
		// locs.subVertex = shader->getSubroutineIndex(GL_VERTEX_SHADER, "verts_and_normals");
		//std::vector<glm::vec3> basevertices;
		//std::vector<glm::vec3> basenormals;

		//float step = 2.f * float(M_PI) / 32.f;
		//for(float f = 0; f <= 2.f * float(M_PI) + step; f += step)
		//{
		//    float x = std::sin(f);
		//    float z = std::cos(f);
		//    basevertices.push_back(glm::vec3(x, 0.f, z));
		//    basenormals.push_back(glm::vec3(x, std::sin(float(M_PI) / 12.6), z));
		//}

		//std::vector<float> data;
		//glm::vec3 top(0.f, 2.f, 0.f);
		//for(unsigned int i = 0; i < basevertices.size(); ++i)
		//{
		//    int next = (i + 1 == basevertices.size()) ? 0 : i + 1;

		//    glm::vec3 topnormal = glm::normalize(basenormals[i] + basenormals[next]);

		//    data.insert(std::end(data), glm::value_ptr(basevertices[i]), glm::value_ptr(basevertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//    data.insert(std::end(data), glm::value_ptr(basenormals[i]), glm::value_ptr(basenormals[i]) + sizeof(glm::vec3) / sizeof(float));
		//    cone.vertsToDraw++;

		//    data.insert(std::end(data), glm::value_ptr(top), glm::value_ptr(top) + sizeof(glm::vec3) / sizeof(float));
		//    data.insert(std::end(data), glm::value_ptr(topnormal), glm::value_ptr(topnormal) + sizeof(glm::vec3) / sizeof(float));
		//    cone.vertsToDraw++;

		//    data.insert(std::end(data), glm::value_ptr(basevertices[next]), glm::value_ptr(basevertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//    data.insert(std::end(data), glm::value_ptr(basenormals[next]), glm::value_ptr(basenormals[next]) + sizeof(glm::vec3) / sizeof(float));
		//    cone.vertsToDraw++;
		//}

		//glGenVertexArrays(1, &cone.vao);
		//glBindVertexArray(cone.vao);

		//glGenBuffers(1, &cone.vbo);
		//glBindBuffer(GL_ARRAY_BUFFER, cone.vbo);
		//glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

		//glEnableVertexAttribArray(locs.vertex);
		//glVertexAttribPointer(locs.vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		//glEnableVertexAttribArray(locs.normal);
		//glVertexAttribPointer(locs.normal, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*) size_t(3 * sizeof(float)));

		//-------------------------------------------------------------------------------------------------------------------------------
		
	// Code Elisa
	/*	locs.subFragment = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "changeByParam");
		locs.subVertex = shader->getSubroutineIndex(GL_VERTEX_SHADER, "simpleTrans");

		std::vector<glm::vec3> basevertices2;
		std::vector<glm::vec3> basenormals2;

		basevertices2.push_back(glm::vec3(1.f, 0.f, 0.f));
		basevertices2.push_back(glm::vec3(0.f, 1.f, 0.f));
		basevertices2.push_back(glm::vec3(1.f, 1.f, 0.f));

		basenormals2.push_back(glm::vec3(0.f, 0.f, 1.f));
		basenormals2.push_back(glm::vec3(0.f, 0.f, 1.f));
		basenormals2.push_back(glm::vec3(0.f, 0.f, 1.f));


		std::vector<float> data;

		data.insert(std::end(data), glm::value_ptr(basevertices2[0]), glm::value_ptr(basevertices2[0]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals2[0]), glm::value_ptr(basenormals2[0]) + sizeof(glm::vec3) / sizeof(float));
		cone.vertsToDraw++;

		data.insert(std::end(data), glm::value_ptr(basevertices2[1]), glm::value_ptr(basevertices2[1]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals2[1]), glm::value_ptr(basenormals2[1]) + sizeof(glm::vec3) / sizeof(float));
		cone.vertsToDraw++;

		data.insert(std::end(data), glm::value_ptr(basevertices2[2]), glm::value_ptr(basevertices2[2]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals2[2]), glm::value_ptr(basenormals2[2]) + sizeof(glm::vec3) / sizeof(float));
		cone.vertsToDraw++;

		glGenVertexArrays(1, &cone.vao);
		glBindVertexArray(cone.vao);

		glGenBuffers(1, &cone.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, cone.vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(locs.vertex);
		glVertexAttribPointer(locs.vertex, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr);
		glEnableVertexAttribArray(locs.normal);
		glVertexAttribPointer(locs.normal, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)size_t(3 * sizeof(float)));*/

		//-------------------------------------------------------------------------------------------------------------------------------
		//Zylinder
		//locs.subFragment = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "changeByParam");
		//locs.subVertex = shader->getSubroutineIndex(GL_VERTEX_SHADER, "simpleTrans");
		//std::vector<glm::vec3> basevertices;
		//std::vector<glm::vec3> topvertices;
		//std::vector<glm::vec3> basenormals;

		//float step = 2.f * float(M_PI) / 32.f;
		//for(float f = 0; f <= 2.f * float(M_PI) + step; f += step)
		//{
		//    float x = std::sin(f);
		//    float z = std::cos(f);
		//    basevertices.push_back(glm::vec3(x, 0.f, z));
		//	topvertices.push_back(glm::vec3(x, 2.f, z));
		//}

		//glm::vec3 baseCenter(0.f, 0.f, 0.f);
		//glm::vec3 topCenter(0.f, 2.f, 0.f);

		//// Fill in the Data
		//std::vector<float> data;
		//for(unsigned int i = 0; i < basevertices.size(); ++i)
		//{
		//    int next = (i + 1 == basevertices.size()) ? 0 : i + 1;
		//	// ---------- Boden ---------- 
		//	glm::vec3 topnormal = glm::vec3(0, 1, 0);
		//	glm::vec3 basenormal = glm::vec3(0, -1, 0);


		//    data.insert(std::end(data), glm::value_ptr(basevertices[i]), glm::value_ptr(basevertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//    data.insert(std::end(data), glm::value_ptr(basenormal), glm::value_ptr(basenormal) + sizeof(glm::vec3) / sizeof(float));
		//    cone.vertsToDraw++;

		//    data.insert(std::end(data), glm::value_ptr(baseCenter), glm::value_ptr(baseCenter) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(basenormal), glm::value_ptr(basenormal) + sizeof(glm::vec3) / sizeof(float));
		//    cone.vertsToDraw++;

		//    data.insert(std::end(data), glm::value_ptr(basevertices[next]), glm::value_ptr(basevertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(basenormal), glm::value_ptr(basenormal) + sizeof(glm::vec3) / sizeof(float));
		//    cone.vertsToDraw++;

		//	// ---------- Deckel ---------- 
		//	data.insert(std::end(data), glm::value_ptr(topvertices[i]), glm::value_ptr(topvertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(topnormal), glm::value_ptr(topnormal) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//	data.insert(std::end(data), glm::value_ptr(topCenter), glm::value_ptr(topCenter) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(topnormal), glm::value_ptr(topnormal) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//	data.insert(std::end(data), glm::value_ptr(topvertices[next]), glm::value_ptr(topvertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(topnormal), glm::value_ptr(topnormal) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//	// ---------- Seitendreiecke: Top - Base - Top ---------- 
		//	data.insert(std::end(data), glm::value_ptr(topvertices[i]), glm::value_ptr(topvertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(basevertices[i]), glm::value_ptr(basevertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//	data.insert(std::end(data), glm::value_ptr(basevertices[i]), glm::value_ptr(basevertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(basevertices[i]), glm::value_ptr(basevertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//	data.insert(std::end(data), glm::value_ptr(topvertices[next]), glm::value_ptr(topvertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(basevertices[next]), glm::value_ptr(basevertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//	// ---------- Seitendreiecke: Base - Top - Base ---------- 
		//	data.insert(std::end(data), glm::value_ptr(basevertices[i]), glm::value_ptr(basevertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(basevertices[i]), glm::value_ptr(basevertices[i]) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//	data.insert(std::end(data), glm::value_ptr(topvertices[next]), glm::value_ptr(topvertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(basevertices[next]), glm::value_ptr(basevertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//	data.insert(std::end(data), glm::value_ptr(basevertices[next]), glm::value_ptr(basevertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//	data.insert(std::end(data), glm::value_ptr(basevertices[next]), glm::value_ptr(basevertices[next]) + sizeof(glm::vec3) / sizeof(float));
		//	cone.vertsToDraw++;

		//}
		//
		//      glGenVertexArrays(1, &cone.vao);
		//      glBindVertexArray(cone.vao);

		//      glGenBuffers(1, &cone.vbo);
		//      glBindBuffer(GL_ARRAY_BUFFER, cone.vbo);
		//      glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

		//glEnableVertexAttribArray(locs.vertex);
		//glVertexAttribPointer(locs.vertex, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr); // Ein zu betrachtenter Punkt besteht aus 6 float Werten
		//glEnableVertexAttribArray(locs.normal);
		//glVertexAttribPointer(locs.normal, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)size_t(3 * sizeof(float)));

		////////-------------------------------------------------------------------------------------------------------------------------------
		//// Würfel bei tessDepth = 0 und bei tessDepth >= 1 Kugel
		locs.subVertex = shader->getSubroutineIndex(GL_VERTEX_SHADER, "verts_and_normals");
		locs.subFragment = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "phong");
		std::vector<glm::vec3> basevertices;
		int tessDepth = 5;
		std::vector<float> data;
		

		basevertices.push_back(glm::vec3(-1.f, 1.f, 1.f));
		basevertices.push_back(glm::vec3(1.f, 1.f, 1.f));
		basevertices.push_back(glm::vec3(1.f, -1.f, 1.f));
		basevertices.push_back(glm::vec3(-1.f, -1.f, 1.f));

		basevertices.push_back(glm::vec3(-1.f, 1.f, -1.f));
		basevertices.push_back(glm::vec3(1.f, 1.f, -1.f));
		basevertices.push_back(glm::vec3(1.f, -1.f, -1.f));
		basevertices.push_back(glm::vec3(-1.f, -1.f, -1.f));

		// Reihenfolge inder die Dreiecke reihum gezeichnet werden sollen --> siehe Buch 107
		int indices[] = {	/*Boden:*/					2, 3, 6,

														3, 6, 7,
							/*Rueckseite links unten:*/	6, 7, 4,
							/*linke Seite:*/			7, 4, 3,
														4, 3, 0,
							/* Vorderseite:*/			3, 0, 2,
														0, 2, 1,
							/*rechte Seite:*/			2, 1, 6,
														1, 6, 5,
							/*Rueckseite rechts unten:*/6, 5, 4,
							/*Deckel:*/					5, 4, 1,
														4, 1, 0 };

		// //------------------------------- Kugel -------------------------------
		// Tessilieren der Dreiecke
		std::vector<glm::vec3> verticesToTessilate;
		for (unsigned int i = 0; i < 36; ++i)
		{
			verticesToTessilate.push_back(basevertices[indices[i]]);
		}
		std::vector<glm::vec3> verticesTessilated = tessellate(verticesToTessilate, tessDepth);	

		for (unsigned int i = 0; i < verticesTessilated.size(); ++i)
		{
			// ---------- Einfügen der Punkte nach den Indizes, wie sie im Array indices auftauchen, nach dieser Reihenfolge, werden die Dreiecke reihum gezeichnet ---------- 
			data.insert(std::end(data), glm::value_ptr(verticesTessilated[i]), glm::value_ptr(verticesTessilated[i]) + sizeof(glm::vec3) / sizeof(float));
			// Erneutes Einfügen des gleichen Vektors, um den Normalen Vektor hinzuzufügen
			data.insert(std::end(data), glm::value_ptr(verticesTessilated[i]), glm::value_ptr(verticesTessilated[i]) + sizeof(glm::vec3) / sizeof(float));
			cone.vertsToDraw++;
		}

		glGenVertexArrays(1, &cone.vao);
		glBindVertexArray(cone.vao);

		glGenBuffers(1, &cone.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, cone.vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(locs.vertex);
		glVertexAttribPointer(locs.vertex, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6 /* Ein zu betrachtenter Punkt besteht aus 6 float Werten*/, nullptr);
		glEnableVertexAttribArray(locs.normal);
		glVertexAttribPointer(locs.normal, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)size_t(3 * sizeof(float)));

		moon.vertsToDraw = cone.vertsToDraw;
		glGenVertexArrays(1, &moon.vao);
		glBindVertexArray(moon.vao);

		glGenBuffers(1, &moon.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, moon.vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(locs.vertex);
		glVertexAttribPointer(locs.vertex, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6 /* Ein zu betrachtenter Punkt besteht aus 6 float Werten*/, nullptr);
		glEnableVertexAttribArray(locs.normal);
		glVertexAttribPointer(locs.normal, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)size_t(3 * sizeof(float)));

		
		// Dreieck
		locs.subFragment = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "textured");

		//std::vector<glm::vec3> basevertices;
		//std::vector<float> data;
		std::vector<glm::vec3> basenormals;
		std::vector<glm::vec2> baseuvs;

		basevertices.clear();
		data.clear();

		// ------------- Dreieck links oben	 
		// unten links
		basevertices.push_back(glm::vec3(-0.5f, -0.5f, 0.f));
		basenormals.push_back(glm::vec3(0.f, 0.f, 1.f));
		baseuvs.push_back(glm::vec2(0.f, 0.f));
		// unten rechts
		basevertices.push_back(glm::vec3(0.5f, -0.5f, 0.f));
		basenormals.push_back(glm::vec3(0.f, 0.f, 1.f));
		baseuvs.push_back(glm::vec2(1.f, 0.f));
		// oben links
		basevertices.push_back(glm::vec3(-0.5f, 0.5f, 0.f));
		basenormals.push_back(glm::vec3(0.f, 0.f, 1.f));
		baseuvs.push_back(glm::vec2(0.f, 1.f));

		// -------------  rechts oben
		// oben links
		basevertices.push_back(glm::vec3(-0.5f, 0.5f, 0.f));
		basenormals.push_back(glm::vec3(0.f, 0.f, 1.f));
		baseuvs.push_back(glm::vec2(0.f, 1.f));
		// unten rechts
		basevertices.push_back(glm::vec3(0.5f, -0.5f, 0.f));
		basenormals.push_back(glm::vec3(0.f, 0.f, 1.f));
		baseuvs.push_back(glm::vec2(1.f, 0.f));
		// oben rechts
		basevertices.push_back(glm::vec3(0.5f, 0.5f, 0.f));
		basenormals.push_back(glm::vec3(0.f, 0.f, 1.f));
		baseuvs.push_back(glm::vec2(1.f, 1.f));

		data.insert(std::end(data), glm::value_ptr(basevertices[0]), glm::value_ptr(basevertices[0]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals[0]), glm::value_ptr(basenormals[0]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(baseuvs[0]), glm::value_ptr(baseuvs[0]) + sizeof(glm::vec2) / sizeof(float));
		disk.vertsToDraw++;

		data.insert(std::end(data), glm::value_ptr(basevertices[1]), glm::value_ptr(basevertices[1]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals[1]), glm::value_ptr(basenormals[1]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(baseuvs[1]), glm::value_ptr(baseuvs[1]) + sizeof(glm::vec2) / sizeof(float));
		disk.vertsToDraw++;

		data.insert(std::end(data), glm::value_ptr(basevertices[2]), glm::value_ptr(basevertices[2]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals[2]), glm::value_ptr(basenormals[2]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(baseuvs[2]), glm::value_ptr(baseuvs[2]) + sizeof(glm::vec2) / sizeof(float));
		disk.vertsToDraw++;

		data.insert(std::end(data), glm::value_ptr(basevertices[3]), glm::value_ptr(basevertices[3]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals[3]), glm::value_ptr(basenormals[3]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(baseuvs[3]), glm::value_ptr(baseuvs[3]) + sizeof(glm::vec2) / sizeof(float));
		disk.vertsToDraw++;

		data.insert(std::end(data), glm::value_ptr(basevertices[4]), glm::value_ptr(basevertices[4]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals[4]), glm::value_ptr(basenormals[4]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(baseuvs[4]), glm::value_ptr(baseuvs[4]) + sizeof(glm::vec2) / sizeof(float));
		disk.vertsToDraw++;

		data.insert(std::end(data), glm::value_ptr(basevertices[5]), glm::value_ptr(basevertices[5]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(basenormals[5]), glm::value_ptr(basenormals[5]) + sizeof(glm::vec3) / sizeof(float));
		data.insert(std::end(data), glm::value_ptr(baseuvs[5]), glm::value_ptr(baseuvs[5]) + sizeof(glm::vec2) / sizeof(float));
		disk.vertsToDraw++;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glGenVertexArrays(1, &disk.vao);
		glBindVertexArray(disk.vao);

		glGenBuffers(1, &disk.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, disk.vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

		// Vertex
		glEnableVertexAttribArray(locs.vertex);
		glVertexAttribPointer(locs.vertex, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
		// Normals
		glEnableVertexAttribArray(locs.normal);
		glVertexAttribPointer(locs.normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)size_t(3 * sizeof(float)));
		//UVs
		glEnableVertexAttribArray(locs.uv);
		glVertexAttribPointer(locs.uv, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)size_t(6 * sizeof(float)));

		// Texturen
		glGenSamplers(1, &sampler);
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, 16.f);

		texture = std::make_unique<cgbv::textures::Texture2D>();
		//texture->Generate("../textures/cg/wall.png", true);
		texture->Generate("../textures/cg/th.jpg", true);
		
		////-----------------------------------------------------
		// GUI
		TwInit(TW_OPENGL_CORE, nullptr);
		TwWindowSize(1280, 720);
		TwBar *tweakbar = TwNewBar("TweakBar");
		TwDefine(" TweakBar size='300 400'");

		TwAddVarRW(tweakbar, "Global Rotation", TW_TYPE_QUAT4F, &parameter.globalRotation, "showval=false opened=true");
		TwAddVarRW(tweakbar, "Lichtrichtung", TW_TYPE_DIR3F, &parameter.lightPos, "group=Light axisx=-x axisy=-y axisz=-z opened=true");
		TwAddVarRW(tweakbar, "Ambientes Licht", TW_TYPE_COLOR4F, &parameter.ambientLight,
			" group=Light");
		TwAddVarRW(tweakbar, "diffuses Licht", TW_TYPE_COLOR4F, &parameter.diffusLight,
			" group=Light alpha help='Color and transparency of the cube.' ");
		TwAddVarRW(tweakbar, "Spectacular Licht", TW_TYPE_COLOR4F, &parameter.specularLight,
			" group=Light alpha help='Color and transparency of the cube.' ");
		TwAddVarRW(tweakbar, "Emissives Material", TW_TYPE_COLOR4F, &parameter.emissivMaterial,
			" group=Material alpha help='Color and transparency of the cube.' ");
		TwAddVarRW(tweakbar, "shininess Licht", TW_TYPE_FLOAT, &parameter.shininessMaterial,
			" group=Material");
		TwAddVarRW(tweakbar, "Ambientes Material", TW_TYPE_COLOR4F, &parameter.ambientMaterial,
			" group=Material alpha help='Color and transparency of the cube.' ");
		TwAddVarRW(tweakbar, "diffuses Material", TW_TYPE_COLOR4F, &parameter.diffusMaterial,
			" group=Material alpha help='Color and transparency of the cube.' ");
		TwAddVarRW(tweakbar, "Spectacular Material", TW_TYPE_COLOR4F, &parameter.spekularMaterial,
			" group=Material alpha help='Color and transparency of the cube.' ");

		return true;

		last = std::chrono::high_resolution_clock::now();
	}


	void CGRenderer::render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//camera.setTarget(glm::vec3(0.f, 0.f, 0.f));
		//camera.moveTo(0.f, -.5f, 5.f);
		glm::mat4 view = camera.getViewMatrix();


		model = glm::mat4_cast(parameter.globalRotation);
		model *= glm::scale(glm::mat4(1.f), glm::vec3(2.f, 2.f, 2.f));

		shader->use();
		glUniform4fv(locs.ambientLight, 1, glm::value_ptr(parameter.ambientLight));
		glUniform4fv(locs.diffusLight, 1, glm::value_ptr(parameter.diffusLight));
		glUniform4fv(locs.spekularLight, 1, glm::value_ptr(parameter.specularLight));
		glUniform4fv(locs.ambientMaterial, 1, glm::value_ptr(parameter.ambientMaterial));
		glUniform4fv(locs.diffusMaterial, 1, glm::value_ptr(parameter.diffusMaterial));
		glUniform4fv(locs.spekularMaterial, 1, glm::value_ptr(parameter.spekularMaterial));
		glUniform4fv(locs.emissivMaterial, 1, glm::value_ptr(parameter.emissivMaterial));
		glUniform1f(locs.shininessMaterial, parameter.shininessMaterial);

		normal = glm::transpose(glm::inverse(view * model));
		glUniformMatrix4fv(locs.modelViewProjection, 1, GL_FALSE, glm::value_ptr(projection * view * model));
		glUniformMatrix4fv(locs.modelview, 1, GL_FALSE, glm::value_ptr(view * model));
		glUniformMatrix3fv(locs.normalmatrix, 1, GL_FALSE, glm::value_ptr(normal));
		glUniformMatrix2fv(locs.uvMatrix, 1, GL_FALSE, glm::value_ptr(uvMatrix));


		glUniform3fv(locs.lightPos, 1, glm::value_ptr(parameter.lightPos));

		glActiveTexture(GL_TEXTURE0);
		glBindSampler(0, sampler);
		glBindTexture(GL_TEXTURE_2D, texture->getTextureID());
		glUniform1i(locs.texture, 0);

		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &locs.subVertex);
		auto textured = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "textured");
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &textured);

		//Rechteck
		//glUniform1f(locs.animationUVs, animStage);
		//glPushMatrix();
		//glRotatef(10.f, 0.0, 0.0, 1.0); // Rotate the object.					   
		//glPopMatrix();					// Draw the object

		glBindVertexArray(disk.vao);
		glDrawArrays(GL_TRIANGLES, 0, disk.vertsToDraw);

		//Kugel
		model = glm::mat4_cast(parameter.globalRotation);
		model *= glm::scale(glm::mat4(1.f), glm::vec3(0.4f, 0.4f, 0.4f));
		normal = glm::transpose(glm::inverse(view * model));
		glUniformMatrix4fv(locs.modelViewProjection, 1, GL_FALSE, glm::value_ptr(projection * view * model));
		glUniformMatrix4fv(locs.modelview, 1, GL_FALSE, glm::value_ptr(view * model));
		glUniformMatrix3fv(locs.normalmatrix, 1, GL_FALSE, glm::value_ptr(normal));


		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &locs.subVertex);
		auto phong = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "phong");
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &phong);
		glBindVertexArray(cone.vao);
		glDrawArrays(GL_TRIANGLES, 0, cone.vertsToDraw);

		// Mond
		model = glm::mat4_cast(parameter.globalRotation);
		model = glm::translate(model, glm::vec3(0.6f, 1.5f, 0.6f));
		model *= glm::scale(glm::mat4(1.f), glm::vec3(0.1f, 0.1f, 0.1f));
		normal = glm::transpose(glm::inverse(view * model));
		glUniformMatrix4fv(locs.modelViewProjection, 1, GL_FALSE, glm::value_ptr(projection * view * model));
		glUniformMatrix4fv(locs.modelview, 1, GL_FALSE, glm::value_ptr(view * model));
		glUniformMatrix3fv(locs.normalmatrix, 1, GL_FALSE, glm::value_ptr(normal));

		/*auto rotation = shader->getSubroutineIndex(GL_VERTEX_SHADER, "verts_and_normals");
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &rotation);*/
		auto moonShine = shader->getSubroutineIndex(GL_FRAGMENT_SHADER, "moonShine");
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &moonShine);
		glBindVertexArray(moon.vao);
		glDrawArrays(GL_TRIANGLES, 0, moon.vertsToDraw);

		glBindSampler(0, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		TwDraw();
	}


	void CGRenderer::update()
	{
		auto now = std::chrono::high_resolution_clock::now();
		
		std::chrono::duration<float, std::milli> delta = now - last;
		std::cout << delta.count() << std::endl << uvRotation << std::endl << std::endl;
		
		animStage += (delta.count() * 0.02f);

		uvRotation += 0.05f; // (delta.count() * 10.f);
		uvMatrix = glm::mat2(std::cos(uvRotation), -std::sin(uvRotation), std::sin(uvRotation), std::cos(uvRotation));

		last = now;
	}
}