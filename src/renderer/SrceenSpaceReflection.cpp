#include "SrceenSpaceReflection.h"
#include "Base.h"
#include <cmath>

ScreenSpaceReflection::ScreenSpaceReflection()
: m_ssrComputeProgram(0)
, m_ssrCamera(0)
, m_sceneFBO(0)
, m_sceneColorTexture(0)
, m_sceneDepthTexture(0)
, m_cubeGBufferFBO(0)
, m_cubePositionTexture(0)
, m_cubeNormalTexture(0)
, m_cubeMaskTexture(0)
, m_cubeDepthTexture(0)
, m_reflectionTexture(0)
, m_gbufferProgram(0)
, m_fullscreenProgram(0)
, m_cubeProgram(0)
, m_cubeVAO(0)
, m_quadVAO(0)
, m_glDispatchCompute(nullptr)
, m_glBindImageTexture(nullptr)
, m_glMemoryBarrier(nullptr)
{}

ScreenSpaceReflection::~ScreenSpaceReflection(){}

bool ScreenSpaceReflection::init(int screenWidth, int screenHeight, int computeWidth, int computeHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_computeWidth = computeWidth;
    m_computeHeight = computeHeight;

    if (!loadComputeFunctions()) {
    return false;
}

    bool ok = createSceneFramebuffer(screenWidth, screenHeight);
    if(!ok){
        return false;
    }

    if(!createCubeGBuffer(screenWidth, screenHeight)){
        return false;
    }

    m_quadVAO = createFullScreenQuad();
    m_fullscreenProgram = createFullscreenProgram();

    m_cubeVAO = createReflectionPlant();
    m_gbufferProgram = createGBufferProgram();

    m_ssrComputeProgram = createComputeProgram("shader/ssReflection.comp");
    if(m_ssrComputeProgram == 0){
        return false;
    }

    createReflectionTexture();

    m_cubeProgram = createReflectionCubeProgram();

    return m_fullscreenProgram != 0
    && m_quadVAO != 0
    && m_cubeVAO != 0
    && m_gbufferProgram != 0
    && m_reflectionTexture != 0
    && m_cubeProgram != 0;
}

// 构建FBO存放反射需要的数据
bool ScreenSpaceReflection::createSceneFramebuffer(int width, int height){
    glGenFramebuffers(1, &m_sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);

    // color texture
    glGenTextures(1, &m_sceneColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // depth texture
    glGenTextures(1, &m_sceneDepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // color, depth挂载到sceneFBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_sceneDepthTexture, 0);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    if(!complete){
        std::cout << "scene framebuffer failure" << std::endl;
    }

    // return default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return complete;
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

GLuint ScreenSpaceReflection::createFullscreenProgram(){
    const char* vertexSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main(){
        TexCoord = aTexCoord;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
    )"; 

    const char* fragmentSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D screenTexture;
    uniform int debugMode;
    void main(){
        if (debugMode == 1) {
            vec3 debugColor = texture(screenTexture, TexCoord).xyz;
            FragColor = vec4(debugColor * 0.5 + 0.5, 1.0);
        } else {
            FragColor = texture(screenTexture, TexCoord);
        }
    }
    )";

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

GLuint ScreenSpaceReflection::createReflectionCubeProgram(){
    const char* vertexSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec2 aUV;
    layout(location = 2) in vec3 aNormal;
    uniform mat4 uModelMatrix;
    uniform mat4 uViewMatrix;
    uniform mat4 uProjMatrix;
    void main()
    {
        gl_Position = uProjMatrix * uViewMatrix * uModelMatrix * vec4(aPos, 1.0);
    })";
    const char* fragmentSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform sampler2D uReflectionTexture;
    uniform vec2 uResolution;
    void main()
    {
        vec2 screenUV = gl_FragCoord.xy / uResolution;
        vec3 reflection = texture(uReflectionTexture, screenUV).rgb;
        FragColor = vec4(reflection, 1.0);
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

uint ScreenSpaceReflection::createFullScreenQuad(){
    uint _VAO;
    uint _VBO;

    float quadVertices[] = {
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &_VAO);
	glGenBuffers(1, &_VBO);
	glBindVertexArray(_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return _VAO;
}

void ScreenSpaceReflection::debugDraw() {
    glUseProgram(m_fullscreenProgram);
    glBindVertexArray(m_quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_reflectionTexture);
    glUniform1i(glGetUniformLocation(m_fullscreenProgram, "screenTexture"), 0);
    glUniform1i(glGetUniformLocation(m_fullscreenProgram, "debugMode"), 0);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glUseProgram(0);
}

bool ScreenSpaceReflection::loadComputeFunctions() {
    m_glDispatchCompute = reinterpret_cast<DispatchComputeProc>(glfwGetProcAddress("glDispatchCompute"));
    m_glBindImageTexture = reinterpret_cast<BindImageTextureProc>(glfwGetProcAddress("glBindImageTexture"));
    m_glMemoryBarrier = reinterpret_cast<MemoryBarrierProc>(glfwGetProcAddress("glMemoryBarrier"));

    if (!m_glDispatchCompute || !m_glBindImageTexture || !m_glMemoryBarrier) {
        std::cerr << "OpenGL 4.3 compute shader functions are unavailable." << std::endl;
        return false;
    }

    return true;
}

void ScreenSpaceReflection::resize(int screenWidth, int screenHeight){

}

void ScreenSpaceReflection::drawCubeGBuffer(
    const glm::mat4 modelMatrix,
    const glm::mat4 viewMatrix,
    const glm::mat4 projMatrix){
        glUseProgram(m_gbufferProgram);

        glUniformMatrix4fv(
        glGetUniformLocation(m_gbufferProgram, "uModelMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(modelMatrix));

        glUniformMatrix4fv(
        glGetUniformLocation(m_gbufferProgram, "uViewMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(viewMatrix));

        glUniformMatrix4fv(
        glGetUniformLocation(m_gbufferProgram, "uProjMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(projMatrix));

        glBindVertexArray(m_cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUseProgram(0);
}

void ScreenSpaceReflection::drawReflectionCube(
    const glm::mat4& modelMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix
) {
    glUseProgram(m_cubeProgram);

    glUniformMatrix4fv(
        glGetUniformLocation(m_cubeProgram, "uModelMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(modelMatrix)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(m_cubeProgram, "uViewMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(viewMatrix)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(m_cubeProgram, "uProjMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(projMatrix)
    );

    glUniform2f(
        glGetUniformLocation(m_cubeProgram, "uResolution"),
        static_cast<float>(m_screenWidth),
        static_cast<float>(m_screenHeight)
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_reflectionTexture);
    glUniform1i(glGetUniformLocation(m_cubeProgram, "uReflectionTexture"), 0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glBindVertexArray(m_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glUseProgram(0);
}

void ScreenSpaceReflection::dispatch(Camera &camera, const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix){
    if(m_ssrComputeProgram == 0 || m_reflectionTexture == 0){
        return;
    }

    glUseProgram(m_ssrComputeProgram);

    glUniform2i(glGetUniformLocation(m_ssrComputeProgram, "uResolution"), m_screenWidth, m_screenHeight);

    glUniform3fv(glGetUniformLocation(m_ssrComputeProgram, "uCameraPos"), 1, glm::value_ptr(camera.getPosition()));

    glUniformMatrix4fv(glGetUniformLocation(m_ssrComputeProgram, "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glUniformMatrix4fv(glGetUniformLocation(m_ssrComputeProgram, "uProjMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sceneColorTexture);
    glUniform1i(glGetUniformLocation(m_ssrComputeProgram, "uSceneColor"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_sceneDepthTexture);
    glUniform1i(glGetUniformLocation(m_ssrComputeProgram, "uSceneDepth"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_cubePositionTexture);
    glUniform1i(glGetUniformLocation(m_ssrComputeProgram, "uCubePosition"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_cubeNormalTexture);
    glUniform1i(glGetUniformLocation(m_ssrComputeProgram, "uCubeNormal"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_cubeMaskTexture);
    glUniform1i(glGetUniformLocation(m_ssrComputeProgram, "uCubeMask"), 4);

    m_glBindImageTexture(0, m_reflectionTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    GLuint groupX = static_cast<GLuint>(std::ceil(m_screenWidth / 16.0f));
    GLuint groupY = static_cast<GLuint>(std::ceil(m_screenHeight / 16.0f));
    m_glDispatchCompute(groupX, groupY, 1);

    m_glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    glUseProgram(0);
}

GLuint ScreenSpaceReflection::createGBufferProgram(){
    const char* vertexSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec2 aUV;
    layout(location = 2) in vec3 aNormal;

    out vec3 WorldPos;
    out vec3 WorldNormal;

    uniform mat4 uModelMatrix;
    uniform mat4 uViewMatrix;
    uniform mat4 uProjMatrix;

    void main(){
        vec4 world = uModelMatrix * vec4(aPos, 1.0);
        WorldPos = world.xyz;
        WorldNormal = mat3(transpose(inverse(uModelMatrix))) * aNormal;
        gl_Position = uProjMatrix * uViewMatrix * world;
    }
    )"; 

    const char* fragmentSource = R"(
    #version 330 core
    layout(location = 0) out vec4 outPosition;
    layout(location = 1) out vec4 outNormal;
    layout(location = 2) out vec4 outMask;

    in vec3 WorldPos;
    in vec3 WorldNormal;

    void main(){
        outPosition = vec4(WorldPos, 1.0);
        outNormal = vec4(normalize(WorldNormal), 1.0);
        outMask = vec4(1.0, 1.0, 1.0, 1.0);
    }
    )";

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

bool ScreenSpaceReflection::createCubeGBuffer(int width, int height){
    glGenFramebuffers(1, &m_cubeGBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_cubeGBufferFBO);

    // position
    glGenTextures(1, &m_cubePositionTexture);
    glBindTexture(GL_TEXTURE_2D, m_cubePositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // normal
    glGenTextures(1, &m_cubeNormalTexture);
    glBindTexture(GL_TEXTURE_2D, m_cubeNormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // mask
    glGenTextures(1, &m_cubeMaskTexture);
    glBindTexture(GL_TEXTURE_2D, m_cubeMaskTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // depth
    glGenTextures(1, &m_cubeDepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_cubeDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // GBuffer挂载到cubeFBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_cubePositionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_cubeNormalTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_cubeMaskTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_cubeDepthTexture, 0);

    // 挂载
    GLenum drawBuffers[] = { 
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffers);

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    if(!complete){
        std::cout << "cube framebuffer failure" << std::endl;
    }

    // return default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return complete;
}

void ScreenSpaceReflection::createReflectionTexture(){
    glGenTextures(1, &m_reflectionTexture);
    glBindTexture(GL_TEXTURE_2D, m_reflectionTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_screenWidth, m_screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void ScreenSpaceReflection::drawReflectionTexture(){

}

// 主程序使用
void ScreenSpaceReflection::beginScenePass(){
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
    glViewport(0, 0, m_screenWidth, m_screenHeight);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenSpaceReflection::endScenePass(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenSpaceReflection::beginCubeGBufferPass(){
    glBindFramebuffer(GL_FRAMEBUFFER, m_cubeGBufferFBO);
    glViewport(0, 0, m_screenWidth, m_screenHeight);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenSpaceReflection::endCubeGBufferPass(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint ScreenSpaceReflection::getSceneColorTexture() const{
    return m_sceneColorTexture;
}

GLuint ScreenSpaceReflection::getSceneDepthTexture() const{
    return m_sceneDepthTexture;
}
