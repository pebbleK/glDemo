#pragma once

#include "Main.h"
#include "Camera.h"

#ifndef APIENTRY
#define APIENTRY
#endif

class BlackHoleRenderer {
public:
	BlackHoleRenderer();
	~BlackHoleRenderer();

	bool init(int screenWidth, int screenHeight, int computeWidth, int computeHeight);
	void resize(int screenWidth, int screenHeight);
	void renderBackground(Camera& camera, const glm::vec3& blackHoleWorldPos);

private:
	using DispatchComputeProc = void (APIENTRY *)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
	using BindImageTextureProc = void (APIENTRY *)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
	using MemoryBarrierProc = void (APIENTRY *)(GLbitfield barriers);

	struct CameraUBOData {
		glm::vec3 pos; float _pad0;
		glm::vec3 right; float _pad1;
		glm::vec3 up; float _pad2;
		glm::vec3 forward; float _pad3;
		float tanHalfFov;
		float aspect;
		int moving;
		int _pad4;
	};

	struct LensObjectsUBOData {
		int numObjects;
		float _pad0, _pad1, _pad2;
		glm::vec4 posRadius[2];
		glm::vec4 color[2];
	};

	GLuint createComputeProgram(const char* path);
	GLuint createFullscreenProgram();
	void createFullscreenQuad();
	void createOutputTexture();
	void createUBOs();
	void dispatch(Camera& camera, const glm::vec3& blackHoleWorldPos);
	void drawFullscreenQuad();
	void uploadCameraUBO(Camera& camera);
	void uploadDiskUBO();
	void uploadLensObjectsUBO();
	void uploadBlackHoleUBO(const glm::vec3& blackHoleWorldPos);
	bool loadComputeFunctions();
	void destroyGLObjects();

	int m_screenWidth;
	int m_screenHeight;
	int m_computeWidth;
	int m_computeHeight;

	GLuint m_computeProgram;
	GLuint m_fullscreenProgram;
	GLuint m_quadVAO;
	GLuint m_quadVBO;
	GLuint m_outputTexture;
	GLuint m_cameraUBO;
	GLuint m_diskUBO;
	GLuint m_lensObjectsUBO;
	GLuint m_blackHoleUBO;

	DispatchComputeProc m_glDispatchCompute;
	BindImageTextureProc m_glBindImageTexture;
	MemoryBarrierProc m_glMemoryBarrier;
};
