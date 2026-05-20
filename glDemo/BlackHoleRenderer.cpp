#include "BlackHoleRenderer.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER 0x91B9
#endif
#ifndef GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#endif
#ifndef GL_TEXTURE_FETCH_BARRIER_BIT
#define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#endif
#ifndef GL_TEXTURE_UPDATE_BARRIER_BIT
#define GL_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#endif
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9
#endif

namespace {
constexpr float kSpeedOfLight = 299792458.0f;
constexpr float kGravity = 6.67430e-11f;
constexpr float kSagittariusMass = 8.54e36f;
constexpr float kSagittariusRs = 2.0f * kGravity * kSagittariusMass / (kSpeedOfLight * kSpeedOfLight);
constexpr float kSceneToMeters = 2.0e9f;

std::string readTextFile(const char* path) {
	std::ifstream in(path);
	if (!in.is_open()) {
		std::cerr << "Failed to open shader: " << path << std::endl;
		return std::string();
	}

	std::stringstream ss;
	ss << in.rdbuf();
	return ss.str();
}

bool checkShader(GLuint shader, const char* label) {
	GLint ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (ok == GL_TRUE) {
		return true;
	}

	GLint logLen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
	std::vector<char> log(std::max(logLen, 1));
	glGetShaderInfoLog(shader, logLen, nullptr, log.data());
	std::cerr << label << " compile error:\n" << log.data() << std::endl;
	return false;
}

bool checkProgram(GLuint program, const char* label) {
	GLint ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &ok);
	if (ok == GL_TRUE) {
		return true;
	}

	GLint logLen = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
	std::vector<char> log(std::max(logLen, 1));
	glGetProgramInfoLog(program, logLen, nullptr, log.data());
	std::cerr << label << " link error:\n" << log.data() << std::endl;
	return false;
}
}

BlackHoleRenderer::BlackHoleRenderer()
	: m_screenWidth(0)
	, m_screenHeight(0)
	, m_computeWidth(0)
	, m_computeHeight(0)
	, m_computeProgram(0)
	, m_fullscreenProgram(0)
	, m_quadVAO(0)
	, m_quadVBO(0)
	, m_outputTexture(0)
	, m_cameraUBO(0)
	, m_diskUBO(0)
	, m_lensObjectsUBO(0)
	, m_blackHoleUBO(0)
	, m_glDispatchCompute(nullptr)
	, m_glBindImageTexture(nullptr)
	, m_glMemoryBarrier(nullptr) {
}

BlackHoleRenderer::~BlackHoleRenderer() {
	destroyGLObjects();
}

bool BlackHoleRenderer::init(int screenWidth, int screenHeight, int computeWidth, int computeHeight) {
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_computeWidth = computeWidth;
	m_computeHeight = computeHeight;

	if (!loadComputeFunctions()) {
		return false;
	}

	m_fullscreenProgram = createFullscreenProgram();
	m_computeProgram = createComputeProgram("shader/geodesic.comp");
	if (m_fullscreenProgram == 0 || m_computeProgram == 0) {
		return false;
	}

	createFullscreenQuad();
	createOutputTexture();
	createUBOs();
	return true;
}

void BlackHoleRenderer::resize(int screenWidth, int screenHeight) {
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}

void BlackHoleRenderer::renderBackground(Camera& camera, const glm::vec3& blackHoleWorldPos) {
	if (m_computeProgram == 0 || m_fullscreenProgram == 0) {
		return;
	}

	dispatch(camera, blackHoleWorldPos);
	drawFullscreenQuad();
}

bool BlackHoleRenderer::loadComputeFunctions() {
	m_glDispatchCompute = reinterpret_cast<DispatchComputeProc>(glfwGetProcAddress("glDispatchCompute"));
	m_glBindImageTexture = reinterpret_cast<BindImageTextureProc>(glfwGetProcAddress("glBindImageTexture"));
	m_glMemoryBarrier = reinterpret_cast<MemoryBarrierProc>(glfwGetProcAddress("glMemoryBarrier"));

	if (!m_glDispatchCompute || !m_glBindImageTexture || !m_glMemoryBarrier) {
		std::cerr << "OpenGL 4.3 compute shader functions are unavailable." << std::endl;
		return false;
	}
	return true;
}

GLuint BlackHoleRenderer::createComputeProgram(const char* path) {
	std::string source = readTextFile(path);
	if (source.empty()) {
		return 0;
	}

	const char* src = source.c_str();
	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	if (!checkShader(shader, path)) {
		glDeleteShader(shader);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);
	glDeleteShader(shader);
	if (!checkProgram(program, path)) {
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

GLuint BlackHoleRenderer::createFullscreenProgram() {
	const char* vertexSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
})";

	const char* fragmentSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTexture;
void main() {
    FragColor = texture(screenTexture, TexCoord);
})";

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, nullptr);
	glCompileShader(vertexShader);
	if (!checkShader(vertexShader, "black hole fullscreen vertex")) {
		glDeleteShader(vertexShader);
		return 0;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
	glCompileShader(fragmentShader);
	if (!checkShader(fragmentShader, "black hole fullscreen fragment")) {
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if (!checkProgram(program, "black hole fullscreen")) {
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

void BlackHoleRenderer::createFullscreenQuad() {
	float quadVertices[] = {
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
	};

	glGenVertexArrays(1, &m_quadVAO);
	glGenBuffers(1, &m_quadVBO);
	glBindVertexArray(m_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);
}

void BlackHoleRenderer::createOutputTexture() {
	glGenTextures(1, &m_outputTexture);
	glBindTexture(GL_TEXTURE_2D, m_outputTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_computeWidth, m_computeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void BlackHoleRenderer::createUBOs() {
	glGenBuffers(1, &m_cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUBOData), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_cameraUBO);

	glGenBuffers(1, &m_diskUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_diskUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_diskUBO);

	glGenBuffers(1, &m_lensObjectsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_lensObjectsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LensObjectsUBOData), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_lensObjectsUBO);

	glGenBuffers(1, &m_blackHoleUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_blackHoleUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_blackHoleUBO);
}

void BlackHoleRenderer::dispatch(Camera& camera, const glm::vec3& blackHoleWorldPos) {
	glBindTexture(GL_TEXTURE_2D, m_outputTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_computeWidth, m_computeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glUseProgram(m_computeProgram);
	glUniform2i(glGetUniformLocation(m_computeProgram, "uResolution"), m_computeWidth, m_computeHeight);
	uploadCameraUBO(camera);
	uploadDiskUBO();
	uploadLensObjectsUBO();
	uploadBlackHoleUBO(blackHoleWorldPos);

	m_glBindImageTexture(0, m_outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	m_glDispatchCompute(static_cast<GLuint>(std::ceil(m_computeWidth / 16.0f)), static_cast<GLuint>(std::ceil(m_computeHeight / 16.0f)), 1);
	m_glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
}

void BlackHoleRenderer::drawFullscreenQuad() {
	glUseProgram(m_fullscreenProgram);
	glBindVertexArray(m_quadVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_outputTexture);
	glUniform1i(glGetUniformLocation(m_fullscreenProgram, "screenTexture"), 0);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void BlackHoleRenderer::uploadCameraUBO(Camera& camera) {
	CameraUBOData data = {};
	glm::mat4 view = camera.getMatrix();
	glm::vec3 right = glm::normalize(glm::vec3(view[0][0], view[1][0], view[2][0]));
	glm::vec3 up = glm::normalize(glm::vec3(view[0][1], view[1][1], view[2][1]));
	glm::vec3 forward = glm::normalize(camera.getDirection());

	data.pos = camera.getPosition() * kSceneToMeters;
	data.right = right;
	data.up = up;
	data.forward = forward;
	data.tanHalfFov = std::tan(glm::radians(45.0f * 0.5f));
	data.aspect = static_cast<float>(m_screenWidth) / static_cast<float>(std::max(m_screenHeight, 1));
	data.moving = 0;

	glBindBuffer(GL_UNIFORM_BUFFER, m_cameraUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
}

void BlackHoleRenderer::uploadDiskUBO() {
	float diskData[4] = {
		kSagittariusRs * 2.2f,
		kSagittariusRs * 5.2f,
		2.0f,
		1.0e9f,
	};

	glBindBuffer(GL_UNIFORM_BUFFER, m_diskUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(diskData), diskData);
}

void BlackHoleRenderer::uploadLensObjectsUBO() {
	float time = static_cast<float>(glfwGetTime());
	LensObjectsUBOData data = {};
	data.numObjects = 2;

	float orbitA = 20.0f;
	float orbitB = 40.0f;
	float angleA = time * 0.35f;
	float angleB = time * 0.22f + 3.1415926f;

	glm::vec3 planetALocal(
		std::cos(angleA) * orbitA,
		1.2f * std::sin(time * 0.17f),
		std::sin(angleA) * orbitA
	);
	glm::vec3 planetBLocal(
		std::cos(angleB) * orbitB,
		-1.0f * std::sin(time * 0.13f),
		std::sin(angleB) * orbitB
	);

	float angleRateA = 5.35f;
	float angleRateB = 10.35f;

	data.posRadius[0] = glm::vec4(planetALocal * kSceneToMeters, angleRateA * kSceneToMeters);
	data.posRadius[1] = glm::vec4(planetBLocal * kSceneToMeters, angleRateB * kSceneToMeters);
	data.color[0] = glm::vec4(0.20f, 0.55f, 1.0f, 1.0f);
	data.color[1] = glm::vec4(1.0f, 0.42f, 0.18f, 1.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, m_lensObjectsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
}

void BlackHoleRenderer::uploadBlackHoleUBO(const glm::vec3& blackHoleWorldPos) {
	glm::vec3 scaledPos = blackHoleWorldPos * kSceneToMeters;
	float data[4] = { scaledPos.x, scaledPos.y, scaledPos.z, 0.0f };
	glBindBuffer(GL_UNIFORM_BUFFER, m_blackHoleUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), data);
}

void BlackHoleRenderer::destroyGLObjects() {
	if (m_blackHoleUBO != 0) glDeleteBuffers(1, &m_blackHoleUBO);
	if (m_lensObjectsUBO != 0) glDeleteBuffers(1, &m_lensObjectsUBO);
	if (m_diskUBO != 0) glDeleteBuffers(1, &m_diskUBO);
	if (m_cameraUBO != 0) glDeleteBuffers(1, &m_cameraUBO);
	if (m_outputTexture != 0) glDeleteTextures(1, &m_outputTexture);
	if (m_quadVBO != 0) glDeleteBuffers(1, &m_quadVBO);
	if (m_quadVAO != 0) glDeleteVertexArrays(1, &m_quadVAO);
	if (m_computeProgram != 0) glDeleteProgram(m_computeProgram);
	if (m_fullscreenProgram != 0) glDeleteProgram(m_fullscreenProgram);
}
