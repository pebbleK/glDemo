#include "SrceenSpaceReflection.h"

ScreenSpaceReflection::ScreenSpaceReflection()
    : m_ssrCamera(0){

    }

bool ScreenSpaceReflection::init(int screenWidth, int screenHeight, int computerWidth, int computerHeight){
    createComputeProgram("shader/ssReflection.comp");
}

uint ScreenSpaceReflection::createReflectionPlant(){
    uint _VAO = 0;
    uint _VBO = 0;
    float vertices[] = {
        // Position           // UV coordinates    // Normal
        // Front face (Z = -0.5)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        // Back face (Z = 0.5)
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        // Left face (X = -0.5)
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        // Right face (X = 0.5)
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         // Bottom face (Y = -0.5)
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
          0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
         // Top face (Y = 0.5)
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);

    glGenBuffers(1, &_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 3));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 5));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // return default
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return _VAO;
}

void ScreenSpaceReflection::draw(){
    glUseProgram(m_ssrProgram);

    glUseProgram(0);
}

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

GLuint ScreenSpaceReflection::createComputeProgram(const char* path) {
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