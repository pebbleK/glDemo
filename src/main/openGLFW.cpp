#include "Base.h"
#include "Shader.h"
#include "Camera.h"
#include "read.h"
#include "BlackHoleRenderer.h"
#include "SrceenSpaceReflection.h"

uint VAO_sun = 0;

glm::vec3 light_color(1.0f);

Shader _shader_sun;
Shader _shader_scene;

Model* _model;

Camera _camera;
BlackHoleRenderer _blackHoleRenderer;
ScreenSpaceReflection _screenSpaceReflection;

glm::mat4 _projMatrix(1.0f);
int _width = 800;
int _height = 600;
glm::vec3 _blackHolePosition(0.0f, 0.0f, -60.0f);

void rend() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::vec3 modelPositions = glm::vec3(0.5f, 2.5f, 0.5f);
    glm::vec3 pointLightPositions = glm::vec3(0.7f, 2.2f, 2.0f);
	_camera.update();
	_projMatrix = glm::perspective(glm::radians(45.0f), (float)_width / (float)_height, 0.1f, 100.0f);
	glm::mat4 _modelMatrix(1.0f);

	_blackHoleRenderer.renderBackground(_camera, _blackHolePosition);
	glClear(GL_DEPTH_BUFFER_BIT);

    _screenSpaceReflection.beginScenePass();

	// Object rendering
    _shader_scene.start();
    _shader_scene.setVec3("view_pos", _camera.getPosition());
    _shader_scene.setMatrix("_viewMatrix", _camera.getMatrix());
    _shader_scene.setMatrix("_projMatrix", _projMatrix);
    // Directional light
    _shader_scene.setVec3("_dirLight.m_direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    _shader_scene.setVec3("_dirLight.m_ambient", light_color * glm::vec3(0.5f));
    _shader_scene.setVec3("_dirLight.m_diffuse", light_color * glm::vec3(0.4f));
    _shader_scene.setVec3("_dirLight.m_specular", light_color * glm::vec3(0.5f));
    // Point light
    _shader_scene.setVec3("_pointLight.m_pos", pointLightPositions);
    _shader_scene.setVec3("_pointLight.m_ambient", light_color * glm::vec3(0.05f));
    _shader_scene.setVec3("_pointLight.m_diffuse", light_color * glm::vec3(0.8f));
    _shader_scene.setVec3("_pointLight.m_specular", light_color * glm::vec3(1.0f));
    _shader_scene.setFloat("_pointLight.m_c", 1.0f);
    _shader_scene.setFloat("_pointLight.m_l", 0.09f);
    _shader_scene.setFloat("_pointLight.m_q", 0.032f);
    // Spotlight
    _shader_scene.setVec3("_spotLight.m_pos", _camera.getPosition());
    _shader_scene.setVec3("_spotLight.m_direction", _camera.getDirection());
    _shader_scene.setFloat("_spotLight.m_cutOff", glm::cos(glm::radians(12.5f)));
    _shader_scene.setFloat("_spotLight.m_outerCutOff", glm::cos(glm::radians(15.0f)));
    _shader_scene.setVec3("_spotLight.m_ambient", light_color * glm::vec3(0.0f));
    _shader_scene.setVec3("_spotLight.m_diffuse", light_color * glm::vec3(1.0f));
    _shader_scene.setVec3("_spotLight.m_specular", light_color * glm::vec3(1.0f));
    _shader_scene.setFloat("_spotLight.m_c", 1.0f);
    _shader_scene.setFloat("_spotLight.m_l", 0.09f);
    _shader_scene.setFloat("_spotLight.m_q", 0.032f);

    // Draw model
    _modelMatrix = glm::mat4(1.0f);
    _modelMatrix = glm::translate(_modelMatrix, modelPositions);
    _modelMatrix = glm::rotate(_modelMatrix, glm::radians(190.0f), glm::vec3(0, 1.0, 0));
    _modelMatrix = glm::rotate(_modelMatrix, glm::radians(-30.0f), glm::vec3(1.0, 0, 0));
    _modelMatrix = glm::scale(_modelMatrix, glm::vec3(0.2f));
    _shader_scene.setMatrix("_modelMatrix", _modelMatrix);
    _model->draw(_shader_scene);
    _shader_scene.end();

	// Light source
	_shader_sun.start();
	_shader_sun.setMatrix("_viewMatrix", _camera.getMatrix());
	_shader_sun.setMatrix("_projMatrix", _projMatrix);
	_modelMatrix = glm::mat4(1.0f);
	_modelMatrix = glm::translate(_modelMatrix, pointLightPositions);
	_modelMatrix = glm::scale(_modelMatrix, glm::vec3(0.2f));
    _shader_sun.setMatrix("_modelMatrix", _modelMatrix);
    glBindVertexArray(VAO_sun);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    _shader_sun.end();

    _screenSpaceReflection.endScenePass();

    // _screenSpaceReflection.beginCubeGBufferPass();

    // _screenSpaceReflection.drawCubeGBuffer(cubeModelMatrix, _camera.getMatrix(), _projMatrix);

    // _screenSpaceReflection.endCubeGBufferPass();

    // _screenSpaceReflection.debugDrawCubeMask();
}

uint creatLightModel() {
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

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return _VAO;
}

void initShader(const char* _vertexPath, const char* _fragPath) {
	_shader_scene.initShader("shader/sceneShaderv.glsl", "shader/sceneShaderf.glsl");
	_shader_sun.initShader("shader/vsunShader.glsl", "shader/fsunShader.glsl");
	/*_shader_dir.initShader("shader/dirShaderv.glsl", "shader/dirShaderf.glsl");
	_shader_point.initShader("shader/pointShaderv.glsl", "shader/pointShaderf.glsl");
	_shader_spot.initShader("shader/spotShaderv.glsl", "shader/spotShaderf.glsl");*/
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	_width = width;
	_height = height;
	glViewport(0, 0, width, height);
	_blackHoleRenderer.resize(width, height);
}

//keyboard input
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) //Read window command "Exit window"
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		_camera.move(CAMERA_MOVE::MOVE_FRONT);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		_camera.move(CAMERA_MOVE::MOVE_BACK);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		_camera.move(CAMERA_MOVE::MOVE_LEFT);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		_camera.move(CAMERA_MOVE::MOVE_RIGHT);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		_camera.move(CAMERA_MOVE::MOVE_UP);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		_camera.move(CAMERA_MOVE::MOVE_DOWN);
	}
}

// Mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    _camera.onMouseMove(xpos, ypos);
}

int main() {
	glfwInit(); // Get context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Compute shader needs OpenGL 4.3.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Select OpenGL configuration mode

    GLFWwindow* window = glfwCreateWindow(_width, _height, "OpenGL Core", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Bind current context to current window

    // Get OpenGL function pointers via GLAD, e.g., #define
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

	glViewport(0, 0, _width, _height); // Viewport rendering size, can be used for minimap
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Mouse movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor and provide unlimited movement
    glfwSetCursorPosCallback(window, mouse_callback);

	// Camera initialization
	_camera.lookAt(glm::vec3(0.0f, 2.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	_camera.setSpeed(0.01f); // Set movement speed
	_camera.setSensitivity(0.05f); // Set mouse sensitivity

    VAO_sun = creatLightModel();
    light_color = glm::vec3(1.0f, 1.0f, 1.0f); // Light color

	_model = new Model("res/model/ball.obj");

	initShader("", "");
	if (!_blackHoleRenderer.init(_width, _height, 200, 150)) {
		std::cout << "Failed to initialize black hole renderer" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

    if (!_screenSpaceReflection.init(_width, _height, 200, 150)) {
    std::cout << "Failed to initialize screen space reflection" << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        // create front and back framebuffers when first creating window
        rend(); // automatically draw to back framebuffer by default

        glfwSwapBuffers(window); // swap back framebuffer to the front
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
