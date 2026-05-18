#include "BlackHoleBackground.h"

#include <algorithm>

namespace {
	const float BLACK_HOLE_RS = 1.269e10f;
	const float BLACK_HOLE_CAMERA_DISTANCE = 6.34194e10f;
	const float G_CONST = 6.67430e-11f;
	const float C_CONST = 299792458.0f;
	const float BLACK_HOLE_MASS = 8.54e36f;

	struct ObjectData {
		glm::vec4 posRadius;
		glm::vec4 color;
		float mass;
	};

	float schwarzschildRadius(float mass) {
		return 2.0f * G_CONST * mass / (C_CONST * C_CONST);
	}

	std::vector<ObjectData> createObjects() {
		return {
			{ glm::vec4(4e11f, 0.0f, 0.0f, 4e10f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 1.98892e30f },
			{ glm::vec4(0.0f, 0.0f, 4e11f, 4e10f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 1.98892e30f },
			{ glm::vec4(0.0f, 0.0f, 0.0f, schwarzschildRadius(BLACK_HOLE_MASS)), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), BLACK_HOLE_MASS }
		};
	}
}

BlackHoleBackground::BlackHoleBackground() {
	m_quadVAO = 0;
	m_quadVBO = 0;
	m_texture = 0;
	m_drawProgram = 0;
	m_computeProgram = 0;
	m_cameraUBO = 0;
	m_diskUBO = 0;
	m_objectsUBO = 0;
	m_width = 800;
	m_height = 600;
	m_computeWidth = 200;
	m_computeHeight = 150;
}

bool BlackHoleBackground::init(int width, int height) {
	m_width = width;
	m_height = height;
	m_computeWidth = 200;
	m_computeHeight = 150;

	m_drawProgram = createDrawProgram();
	m_computeProgram = createComputeProgram("shader/geodesic.comp");
	if (m_drawProgram == 0 || m_computeProgram == 0) {
		return false;
	}

	createQuad();
	createTexture();
	createUniformBuffers();
	uploadDiskUBO();
	uploadObjectsUBO();
	return true;
}

void BlackHoleBackground::resize(int width, int height) {
	m_width = width;
	m_height = height;
}

void BlackHoleBackground::render(Camera& camera) {
	uploadCameraUBO(camera);
	dispatchCompute();
	drawFullScreenQuad();
}

GLuint BlackHoleBackground::compileShader(GLenum type, const char* source, const char* debugName) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
		std::cout << "Shader compile error (" << debugName << "): " << infoLog << std::endl;
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint BlackHoleBackground::compileShaderFile(GLenum type, const char* path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "Failed to open shader: " << path << std::endl;
		return 0;
	}

	std::stringstream stream;
	stream << file.rdbuf();
	std::string source = stream.str();
	return compileShader(type, source.c_str(), path);
}

GLuint BlackHoleBackground::createDrawProgram() {
	const char* vertexSource = R"(
#version 430 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
	gl_Position = vec4(aPos, 0.0, 1.0);
	TexCoord = aTexCoord;
})";

	const char* fragmentSource = R"(
#version 430 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTexture;
void main() {
	FragColor = texture(screenTexture, TexCoord);
})";

	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, "black hole background vertex");
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, "black hole background fragment");
	if (vertexShader == 0 || fragmentShader == 0) {
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetProgramInfoLog(program, 1024, nullptr, infoLog);
		std::cout << "Black hole draw program link error: " << infoLog << std::endl;
		glDeleteProgram(program);
		return 0;
	}

	return program;
}

GLuint BlackHoleBackground::createComputeProgram(const char* path) {
	GLuint computeShader = compileShaderFile(GL_COMPUTE_SHADER, path);
	if (computeShader == 0) {
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, computeShader);
	glLinkProgram(program);
	glDeleteShader(computeShader);

	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetProgramInfoLog(program, 1024, nullptr, infoLog);
		std::cout << "Black hole compute program link error: " << infoLog << std::endl;
		glDeleteProgram(program);
		return 0;
	}

	return program;
}

void BlackHoleBackground::createQuad() {
	float quadVertices[] = {
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &m_quadVAO);
	glGenBuffers(1, &m_quadVBO);
	glBindVertexArray(m_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void BlackHoleBackground::createTexture() {
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_computeWidth, m_computeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void BlackHoleBackground::createUniformBuffers() {
	glGenBuffers(1, &m_cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_cameraUBO);

	glGenBuffers(1, &m_diskUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_diskUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_diskUBO);

	glGenBuffers(1, &m_objectsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_objectsUBO);
	GLsizeiptr objectBufferSize = sizeof(int) + 3 * sizeof(float) + 16 * (sizeof(glm::vec4) + sizeof(glm::vec4)) + 16 * sizeof(float);
	glBufferData(GL_UNIFORM_BUFFER, objectBufferSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_objectsUBO);
}

void BlackHoleBackground::uploadCameraUBO(Camera& camera) {
	struct UBOData {
		glm::vec3 pos; float pad0;
		glm::vec3 right; float pad1;
		glm::vec3 up; float pad2;
		glm::vec3 forward; float pad3;
		float tanHalfFov;
		float aspect;
		int moving;
		int pad4;
	} data;

	glm::vec3 forward = glm::normalize(camera.getDirection());
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	// Treat the black hole as a distant background and reuse only the main camera orientation.
	data.pos = -forward * BLACK_HOLE_CAMERA_DISTANCE;
	data.right = right;
	data.up = up;
	data.forward = forward;
	data.tanHalfFov = tan(glm::radians(45.0f) * 0.5f);
	data.aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
	data.moving = 0;
	data.pad4 = 0;

	glBindBuffer(GL_UNIFORM_BUFFER, m_cameraUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), &data);
}

void BlackHoleBackground::uploadDiskUBO() {
	float diskInnerRadius = BLACK_HOLE_RS * 2.2f;
	float diskOuterRadius = BLACK_HOLE_RS * 5.2f;
	float diskRayCount = 2.0f;
	float thickness = 1e9f;
	float diskData[4] = { diskInnerRadius, diskOuterRadius, diskRayCount, thickness };

	glBindBuffer(GL_UNIFORM_BUFFER, m_diskUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(diskData), diskData);
}

void BlackHoleBackground::uploadObjectsUBO() {
	struct UBOData {
		int numObjects;
		float pad0;
		float pad1;
		float pad2;
		glm::vec4 posRadius[16];
		glm::vec4 color[16];
		float mass[16];
	} data = {};

	std::vector<ObjectData> objects = createObjects();
	size_t count = std::min(objects.size(), static_cast<size_t>(16));
	data.numObjects = static_cast<int>(count);
	for (size_t i = 0; i < count; ++i) {
		data.posRadius[i] = objects[i].posRadius;
		data.color[i] = objects[i].color;
		data.mass[i] = objects[i].mass;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_objectsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
}

void BlackHoleBackground::dispatchCompute() {
	glUseProgram(m_computeProgram);
	glBindImageTexture(0, m_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	GLuint groupsX = static_cast<GLuint>(ceil(m_computeWidth / 16.0f));
	GLuint groupsY = static_cast<GLuint>(ceil(m_computeHeight / 16.0f));
	glDispatchCompute(groupsX, groupsY, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void BlackHoleBackground::drawFullScreenQuad() {
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glUseProgram(m_drawProgram);
	glBindVertexArray(m_quadVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glUniform1i(glGetUniformLocation(m_drawProgram, "screenTexture"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}
