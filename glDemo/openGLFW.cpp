#include"Base.h"
#include"Shader.h"
#include"ffImage.h"
#include"Camera.h"

uint VAO_cube = 0; //使用核心模式
uint VAO_sun = 0;
//glm::vec3 light_pos(1.0f);
glm::vec3 light_color(1.0f);

unsigned int _texture = 0;

ffImage* _pImage = NULL;

Shader _shader_sun;
Shader _shader_scene;
Shader _shader_color;

//材质贴图
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

	// 阈值越接近1，要求越“正对”
	// 0.98 大约是 11.5 度内
	return dotValue > 0.98f;
}

void rend() {

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFF); //允许写入模板缓冲区


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glm::vec3 cubePositions = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 pointLightPositions = glm::vec3(0.7f, 0.2f, 2.0f);

	_camera.update();
	_projMatrix = glm::perspective(glm::radians(45.0f), (float)_width / (float)_height, 0.1f, 100.0f);
	glm::mat4 _modelMatrix(1.0f);
	_modelMatrix = glm::translate(_modelMatrix, glm::vec3(0.0f, 0.0f, -3.0f));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureBox);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _textureSpec);

	//物体
	_shader_scene.start();

	_shader_scene.setVec3("view_pos", _camera.getPosition());

	//设置物体属性
	_shader_scene.setInt("myMaterial.m_diffuse", 0); //纹理单元0
	_shader_scene.setInt("myMaterial.m_specular", 1); //纹理单元1
	_shader_scene.setFloat("myMaterial.m_shiness", 32.0f);

	_shader_scene.setMatrix("_viewMatrix", _camera.getMatrix());
	_shader_scene.setMatrix("_projMatrix", _projMatrix);

	//平行光
	_shader_scene.setVec3("_dirLight.m_direction", glm::vec3(-0.2f, -1.0f, -0.3f));

	_shader_scene.setVec3("_dirLight.m_ambient", light_color * glm::vec3(0.5f));
	_shader_scene.setVec3("_dirLight.m_diffuse", light_color * glm::vec3(0.4f));
	_shader_scene.setVec3("_dirLight.m_specular", light_color * glm::vec3(0.5f));

	//点光源
	std::string number = std::to_string(0);
	_shader_scene.setVec3("_pointLight.m_pos", pointLightPositions);

	_shader_scene.setVec3("_pointLight.m_ambient", light_color * glm::vec3(0.05f));
	_shader_scene.setVec3("_pointLight.m_diffuse", light_color * glm::vec3(0.8f));
	_shader_scene.setVec3("_pointLight.m_specular", light_color * glm::vec3(1.0f));

	_shader_scene.setFloat("_pointLight.m_c", 1.0f);
	_shader_scene.setFloat("_pointLight.m_l", 0.09f);
	_shader_scene.setFloat("_pointLight.m_q", 0.032f);

	//聚光灯
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

	//绘制立方体

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); //模板测试通过则替换模板缓冲区的内容

	_modelMatrix = glm::mat4(1.0f);
	_modelMatrix = glm::translate(_modelMatrix, cubePositions);
	_modelMatrix = glm::rotate(_modelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	_shader_scene.setMatrix("_modelMatrix", _modelMatrix);
	glBindVertexArray(VAO_cube);
	glDrawArrays(GL_TRIANGLES, 0, 36);


	_shader_scene.end();

	//选中描边判断

	bool lookingAtCube = isLookingAtCube(
		_camera.getPosition(),
		_camera.getDirection(),
		cubePositions
	);

	//绘制描边
	if (lookingAtCube) {
		_shader_color.start();

		_shader_color.setMatrix("_viewMatrix", _camera.getMatrix());
		_shader_color.setMatrix("_projMatrix", _projMatrix);

		glStencilFunc(GL_NEVER, 1, 0xFF); //所有都不通过
		glStencilMask(0x00);// 禁止写入模板缓冲区

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);


		_modelMatrix = glm::mat4(1.0f);
		_modelMatrix = glm::translate(_modelMatrix, cubePositions);
		_modelMatrix = glm::rotate(_modelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		_modelMatrix = glm::scale(_modelMatrix, glm::vec3(1.1f)); //稍微放大一些

		_shader_color.setMatrix("_modelMatrix", _modelMatrix);
		glBindVertexArray(VAO_cube);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		_shader_color.end();

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
	}

	//光源
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

uint creatModel(){
	uint _VAO = 0;
	uint _VBO = 0;

	float vertices[] = {
		// 位置              // UV坐标
		// 前面 (Z = -0.5)
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,	0.0f, 0.0f, -1.0f,

		// 后面 (Z = 0.5)
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,	0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,	0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,	0.0f, 0.0f, 1.0f,

		// 左面 (X = -0.5)
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,

		// 右面 (X = 0.5)
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,	1.0f, 0.0f, 0.0f,

		 // 下面 (Y = -0.5)
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		  0.5f, -0.5f, -0.5f,  1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,	0.0f, -1.0f, 0.0f,

		 // 上面 (Y = 0.5)
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO); //下面VBO，EBO属于VAO，直接调用VAO即可

	/*unsigned int EBO = 0;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

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

//键盘输入
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) //读取窗口指令“退出窗口”
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

//鼠标输入
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	_camera.onMouseMove(xpos, ypos);
}

int main() {
	glfwInit(); //获取上下文
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //版本配置，3版本以上，使用核心开发模式
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //选择OpenGL配置模式

	GLFWwindow* window = glfwCreateWindow(_width, _height, "OpenGL Core", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); //把当前上下文绑定为当前的窗口window

	//通过glad获取opengl需要的函数的指针，比如#define
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, _width, _height); //视口渲染大小，可以做小地图
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//鼠标移动
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //隐藏鼠标并提供无限制的移动
	glfwSetCursorPosCallback(window, mouse_callback);

	//相机初始化
	_camera.lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	_camera.setSpeed(0.001f); //设置移动速度
	_camera.setSensitivity(0.05f); //设置鼠标灵敏度

	VAO_cube = creatModel();
	VAO_sun = creatModel();
	//light_pos = glm::vec3(2.0f, 0.0f, 0.0f); // 光源位置
	light_color = glm::vec3(1.0f, 1.0f, 1.0f); // 光源颜色

	_textureBox = creatTexture("res/box.png");
	_textureSpec = creatTexture("res/specular.png");
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