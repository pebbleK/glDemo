#include "Shader.h"

void Shader::initShader(const char* _vertexPath, const char* _fragPath) {
	std::string _vertexCode("");
	std::string _fragCode("");

	std::ifstream _vShaderFile;
	std::ifstream _fShaderFile;

	_vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	_fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		_vShaderFile.open(_vertexPath);
		_fShaderFile.open(_fragPath);

		std::stringstream _vShaderStream, _fShaderStream;
		_vShaderStream << _vShaderFile.rdbuf();
		_fShaderStream << _fShaderFile.rdbuf();

		_vertexCode = _vShaderStream.str();
		_fragCode = _fShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::string errStr = "read shader fail";
		std::cout << errStr << std::endl;
	}

	const char* _vShaderStr = _vertexCode.c_str();
	const char* _fShaderStr = _fragCode.c_str();

	//shader的编译
	unsigned int _vertexID = 0, _fragID = 0;
	char _infoLog[512];
	int _successFlag = 0;

	_vertexID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_vertexID, 1, &_vShaderStr, NULL);
	glCompileShader(_vertexID);

	glGetShaderiv(_vertexID, GL_COMPILE_STATUS, &_successFlag); //查询编译情况
	if (!_successFlag) {
		glGetShaderInfoLog(_vertexID, 512, NULL, _infoLog);
		std::string errStr(_infoLog);
		std::cout << _infoLog << std::endl;
	}

	_fragID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(_fragID, 1, &_fShaderStr, NULL);
	glCompileShader(_fragID);

	glGetShaderiv(_fragID, GL_COMPILE_STATUS, &_successFlag);
	if (!_successFlag) {
		glGetShaderInfoLog(_fragID, 512, NULL, _infoLog);
		std::string errStr(_infoLog);
		std::cout << _infoLog << std::endl;
	}

	//shader的链接
	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, _vertexID);
	glAttachShader(m_shaderProgram, _fragID);
	glLinkProgram(m_shaderProgram); //链接两个ID

	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &_successFlag);
	if (!_successFlag) {
		glGetProgramInfoLog(m_shaderProgram, 512, NULL, _infoLog);
		std::string errStr(_infoLog);
		std::cout << _infoLog << std::endl;
	}
	//一般保留最后编译好的m_shaderProgram即可
	glDeleteShader(_vertexID);
	glDeleteShader(_fragID);
}

void Shader::setMatrix(const std::string& _name, glm::mat4 _matrix)const {
	 /* glGetUniformLocation(m_shaderProgram, _name.c_str())

		- 获取着色器程序中uniform变量的位置
		- m_shaderProgram: 着色器程序的OpenGL ID
		- _name.c_str() : 将C++字符串转换为C风格字符串（OpenGL
		API需要）
		- 返回一个整数，表示uniform在着色器中的位置

		2. glUniformMatrix4fv(...)

		这是OpenGL的核心函数，用于设置矩阵uniform：
		- glUniformMatrix4fv:
		"gl"表示OpenGL，"Uniform"表示uniform变量，"Matrix4f"
		表示4x4浮点矩阵，"v"表示向量 / 数组形式
		- 第一个参数 :
		uniform的位置（来自glGetUniformLocation）
		- 第二个参数 : 要设置的矩阵数量（这里是1个）
		- 第三个参数 : GL_FALSE表示不进行转置（OpenGL使用列主
		序，glm默认也是列主序）
		- 第四个参数 : 矩阵数据的指针

		3. glm::value_ptr(_matrix)

		- glm::value_ptr() :
		glm库的函数，获取矩阵底层数据的指针
		- 因为OpenGL
		API需要原始数据指针，而glm的mat4是封装类型
		- 这个函数返回指向矩阵16个浮点数的指针 */
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, _name.c_str()), 1, GL_FALSE, glm::value_ptr(_matrix));
}

void Shader::setVec3(const std::string& _name, glm::vec3 _vec3)const {
	glUniform3fv(glGetUniformLocation(m_shaderProgram, _name.c_str()), 1, glm::value_ptr(_vec3));
}

void Shader::setFloat(const std::string& _name, float _f)const {
	glUniform1f(glGetUniformLocation(m_shaderProgram, _name.c_str()), _f);
}

void Shader::setInt(const std::string& _name, int _i)const {
	glUniform1i(glGetUniformLocation(m_shaderProgram, _name.c_str()), _i);
}


