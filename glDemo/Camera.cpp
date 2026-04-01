#include "Camera.h"

void Camera::lookAt(glm::vec3 _position, glm::vec3 _front, glm::vec3 _up) {
	m_position = _position;
	m_front = glm::normalize(_front); //传进来的方向，使其单位向量化
	m_up = _up;

	m_vMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::update() {
	m_vMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getMatrix() {
	return m_vMatrix;
}

glm::vec3 Camera::getPosition() {
	return m_position;
}

glm::vec3 Camera::getDirection() {
	return m_front;
}

void Camera::move(CAMERA_MOVE _mode) {
	switch (_mode)
	{
	case CAMERA_MOVE::MOVE_LEFT:
		m_position -= m_speed * glm::normalize(glm::cross(m_front,m_up));
		break;
	case CAMERA_MOVE::MOVE_RIGHT:
		m_position += m_speed * glm::normalize(glm::cross(m_front, m_up));
		break;
	case CAMERA_MOVE::MOVE_FRONT:
		m_position += m_speed * m_front; //m_front做了归一化
		break;
	case CAMERA_MOVE::MOVE_BACK:
		m_position -= m_speed * m_front;
		break;
	case CAMERA_MOVE::MOVE_UP:
		m_position += m_speed * m_up;
		break;
	case CAMERA_MOVE::MOVE_DOWN:
		m_position -= m_speed * m_up;
		break;
	default:
		break;
	}
}

// pitch:上下移动视角,yz平面，yaw:左右移动视角,xz平面
//修改摄像机方向就是修改m_front单位向量
void Camera::pitch(float yOffset) {
	m_pitch += yOffset * m_sensitivity;

	if (m_pitch >= 89.0f)
		m_pitch = 89.0f;
	if (m_pitch <= -89.0f)
		m_pitch = -89.0f;

	m_front.y = sin(glm::radians(m_pitch));
	m_front.x = cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
	m_front.z = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));

	m_front = glm::normalize(m_front);
	update();
}
void Camera::yaw(float xOffset) {
	m_yaw += xOffset * m_sensitivity;

	m_front.y = sin(glm::radians(m_pitch));
	m_front.x = cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
	m_front.z = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));

	m_front = glm::normalize(m_front);
	update();
}
void Camera::setSensitivity(float _s) {
	m_sensitivity = _s;
}

void Camera::onMouseMove(double _xpos, double _ypos) {
	if(m_firstMove){
		m_xpos = _xpos;
		m_ypos = _ypos;
		m_firstMove = false;
		return;
	}

	float _xOffset = _xpos - m_xpos;
	float _yOffset = -(_ypos - m_ypos); //window窗体坐标是左上角为原点，y轴向下，所以这里要取反

	m_xpos = _xpos;
	m_ypos = _ypos;

	pitch(_yOffset);
	yaw(_xOffset);
}
