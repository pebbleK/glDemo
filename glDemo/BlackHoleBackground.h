#pragma once

#include "Base.h"
#include "Camera.h"

class BlackHoleBackground {
public:
	BlackHoleBackground();

	bool init(int width, int height);
	void resize(int width, int height);
	void render(Camera& camera);

private:
	GLuint m_quadVAO;
	GLuint m_quadVBO;
	GLuint m_texture;
	GLuint m_drawProgram;
	GLuint m_computeProgram;
	GLuint m_cameraUBO;
	GLuint m_diskUBO;
	GLuint m_objectsUBO;

	int m_width;
	int m_height;
	int m_computeWidth;
	int m_computeHeight;

	GLuint createDrawProgram();
	GLuint createComputeProgram(const char* path);
	GLuint compileShader(GLenum type, const char* source, const char* debugName);
	GLuint compileShaderFile(GLenum type, const char* path);
	void createQuad();
	void createTexture();
	void createUniformBuffers();
	void uploadCameraUBO(Camera& camera);
	void uploadDiskUBO();
	void uploadObjectsUBO();
	void dispatchCompute();
	void drawFullScreenQuad();
};
