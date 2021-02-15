#Compiler:
CC = clang++

#Flags & Libs
CFLAGS = -std=c++17
LIBS = -lglfw -lGL -lm -lXrandr -lXi -lX11 -ldl -lXxf86vm -lpthread -lfreeimage -lAntTweakBar -lstdc++fs
LIB_PATH = -L"dependencies/x86/"
INCLUDES = -I"src/" $(ANT_INCLUDE)
ANT_INCLUDE = -I"dependencies/include/AntTweakBar/"



BV_SRC = src/bv/main.cpp src/Renderer.cpp src/bv/BVRenderer.cpp src/GLSLShader.cpp src/Texture.cpp src/glad.cpp src/FrameCounter.cpp

CG_SRC = src/cg/main.cpp src/Renderer.cpp src/cg/CGRenderer.cpp src/GLSLShader.cpp src/Texture.cpp src/glad.cpp src/FrameCounter.cpp src/Camera.cpp


Bildverarbeitung: $(BV_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $(ANT_INCLUDE) -o bin/bildverarbeitung $(BV_SRC) $(LIB_PATH) $(LIBS)
	@echo Ausführung: Bildverarbeitung...
	cd bin/; \
	./bildverarbeitung
	@echo Programm Ende


Computergrafik: $(CG_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $(ANT_INCLUDE) -o bin/computergrafik $(CG_SRC) $(LIB_PATH) $(LIBS)
	@echo Ausführung: Computergrafik...
	cd bin/; \
	./computergrafik
	@echo Programm Ende


Clean:
	@echo deleting binaries...
	rm bin/*
	@echo done