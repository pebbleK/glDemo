#include "Base.h"
#include "Shader.h"
#include "ffImage.h"
#include "Camera.h"
#include "IO.h"

uint VAO_cube = 0; // Using core mode
uint VAO_sun = 0;

// glm::vec3 light_pos(1.0f);
glm::vec3 light_color(1.0f);
unsigned int _texture = 0;
ffImage* _pImage = NULL;

Shader _shader_sun;
Shader _shader_scene;
Shader _shader_color;

FF::ffModel* _model;

// Material textures
uint _textureBox = 0;
uint _textureSpec = 0;

Camera _camera;
glm::mat4 _projMatrix(1.0f);
int _width = 800;
int _height = 600;

bool isLookingAtCube(glm::vec3 cameraPos, glm::vec3 cameraDir, glm::vec3 cubeCenter)
{
    glm::vec3 toCube = glm::normalize(cubeCenter - cameraPos);
    float dotValue = glm::dot(glm::normalize(cameraDir), toCube);
    // Threshold closer to 1 means more "directly facing"
    // 0.98 is approximately within 11.5 degrees
    return dotValue > 0.98f;
}

void drawOutline(Shader _shader, glm::mat4 _modelMatrix, glm::vec3 _position) {
    _shader.start();
    _shader.setMatrix("_viewMatrix", _camera.getMatrix());
    _shader.setMatrix("_projMatrix", _projMatrix);
    glStencilFunc(GL_NEVER, 1, 0xFF); // All fail
    glStencilMask(0x00); // Disable writing to stencil buffer
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    _modelMatrix = glm::mat4(1.0f);
    _modelMatrix = glm::translate(_modelMatrix, _position);
    _modelMatrix = glm::rotate(_modelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _modelMatrix = glm::scale(_modelMatrix, glm::vec3(1.1f)); // Slightly larger
    _shader.setMatrix("_modelMatrix", _modelMatrix);
    glBindVertexArray(VAO_cube);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    _shader.end();
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}

void rend() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF); // Enable writing to stencil buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glm::vec3 cubePositions = glm::vec3(0.7f, 1.0f, 2.0f);
    glm::vec3 modelPositions = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 pointLightPositions = glm::vec3(0.7f, 0.2f, 2.0f);
    _camera.update();
    _projMatrix = glm::perspective(glm::radians(45.0f), (float)_width / (float)_height, 0.1f, 100.0f);
    glm::mat4 _modelMatrix(1.0f);
    _modelMatrix = glm::translate(_modelMatrix, glm::vec3(0.0f, 0.0f, -3.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _textureBox);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _textureSpec);
    // Object rendering
    _shader_scene.start();
    _shader_scene.setVec3("view_pos", _camera.getPosition());
    // Set object material properties
    _shader_scene.setInt("myMaterial.m_diffuse", 0); // Texture unit 0
    _shader_scene.setInt("myMaterial.m_specular", 1); // Texture unit 1
    _shader_scene.setFloat("myMaterial.m_shiness", 32.0f);
    _shader_scene.setMatrix("_viewMatrix", _camera.getMatrix());
    _shader_scene.setMatrix("_projMatrix", _projMatrix);
    // Directional light
    _shader_scene.setVec3("_dirLight.m_direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    _shader_scene.setVec3("_dirLight.m_ambient", light_color * glm::vec3(0.5f));
    _shader_scene.setVec3("_dirLight.m_diffuse", light_color * glm::vec3(0.4f));
    _shader_scene.setVec3("_dirLight.m_specular", light_color * glm::vec3(0.5f));
    // Point light
    std::string number = std::to_string(0);
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
    // Replace stencil buffer content when stencil test passes
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // Draw cube
    _modelMatrix = glm::mat4(1.0f);
    _modelMatrix = glm::translate(_modelMatrix, cubePositions);
    _modelMatrix = glm::rotate(_modelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _shader_scene.setMatrix("_modelMatrix", _modelMatrix);
    glBindVertexArray(VAO_cube);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // Draw model
    _modelMatrix = glm::mat4(1.0f);
    _modelMatrix = glm::translate(_modelMatrix, modelPositions);
    _modelMatrix = glm::scale(_modelMatrix, glm::vec3(0.2f));
    _shader_scene.setMatrix("_modelMatrix", _modelMatrix);
    _model->draw(_shader_scene);
    _shader_scene.end();
    // Selection outline check
    bool lookingAtCube = isLookingAtCube(
        _camera.getPosition(),
        _camera.getDirection(),
        cubePositions
    );
    // Draw outline
    if (lookingAtCube) {
        drawOutline(_shader_color, _modelMatrix, cubePositions);
    }
    // Light source
    _shader_sun.start();
    _shader_sun.setMatrix("_modelMatrix", _modelMatrix);
    _shader_sun.setMatrix("_viewMatrix", _camera.getMatrix());
    _shader_sun.setMatrix("_projMatrix", _projMatrix);
    _modelMatrix = glm::mat4(1.0f);
    _shader_sun.setMatrix("_modelMatrix", _modelMatrix);
    _modelMatrix = glm::mat4(1.0f);
    _modelMatrix = glm::translate(_modelMatrix, pointLightPositions);
    _modelMatrix = glm::scale(_modelMatrix, glm::vec3(0.2f));
    _shader_sun.setMatrix("_modelMatrix", _modelMatrix);
    glBindVertexArray(VAO_sun);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    _shader_sun.end();
}

uint creatModel() {
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

uint creatTexture(const char* _fileName) {
	_pImage = ffImage::readFromFile(_fileName);
	if (_pImage == NULL) {
		std::cout << "Failed to load texture: " << _fileName << std::endl;
		return 0;
	}
	uint _texture = 0;
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _pImage->getWidth(), _pImage->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _pImage->getData());

	return _texture;
}

void initShader(const char* _vertexPath, const char* _fragPath) {
	_shader_scene.initShader("shader/sceneShaderv.glsl", "shader/sceneShaderf.glsl");
	_shader_sun.initShader("shader/vsunShader.glsl", "shader/fsunShader.glsl");
	_shader_color.initShader("shader/colorShaderv.glsl", "shader/colorShaderf.glsl");
	/*_shader_dir.initShader("shader/dirShaderv.glsl", "shader/dirShaderf.glsl");
	_shader_point.initShader("shader/pointShaderv.glsl", "shader/pointShaderf.glsl");
	_shader_spot.initShader("shader/spotShaderv.glsl", "shader/spotShaderf.glsl");*/
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Version configuration, version 3 and above uses core development mode
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
    _camera.lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _camera.setSpeed(0.001f); // Set movement speed
    _camera.setSensitivity(0.05f); // Set mouse sensitivity

    VAO_cube = creatModel();
    VAO_sun = creatModel();
    // light_pos = glm::vec3(2.0f, 0.0f, 0.0f); // Light position
    light_color = glm::vec3(1.0f, 1.0f, 1.0f); // Light color

    _textureBox = creatTexture("res/texture/box.png");
    _textureSpec = creatTexture("res/texture/specular.png");
    _model = new FF::ffModel("res/model/ball.obj");

    initShader("", "");

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        rend();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}