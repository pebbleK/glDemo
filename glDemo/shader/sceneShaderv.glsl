#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

out vec2 outUV;
out vec3 outFragPos;
out vec3 outNormal; // 传递到片段着色器的法线

uniform mat4 _modelMatrix;
uniform mat4 _viewMatrix;
uniform mat4 _projMatrix;

void main(){
	gl_Position = _projMatrix * _viewMatrix * _modelMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	outUV = aUV;
	outFragPos = vec3(_modelMatrix * vec4(aPos, 1.0)); //面通过模型矩阵转换后的世界坐标
	outNormal = mat3(transpose(inverse(_modelMatrix))) * aNormal; //法线通过模型矩阵的逆转置矩阵转换（计算了法线矩阵）
};