#include "Player.h"

void PlayerCube::processInput(GLFWwindow* window) {
    glm::vec3 forward = getForward();
    glm::vec3 right = getRight();

    m_velocity = glm::vec3(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m_velocity += forward;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m_velocity -= forward;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        m_velocity -= right;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        m_velocity += right;
    }

    if (glm::length(m_velocity) > 0.0f) {
        m_velocity = glm::normalize(m_velocity) * m_moveSpeed;
    }
}

void PlayerCube::onMouseMove(double xpos, double ypos) {
    if (m_firstMouse) {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
        return;
    }

    float xOffset = static_cast<float>(xpos - m_lastMouseX);
    float yOffset = static_cast<float>(m_lastMouseY - ypos);

    m_lastMouseX = xpos;
    m_lastMouseY = ypos;

    m_yaw += xOffset * m_mouseSensitivity;
    m_pitch += yOffset * m_mouseSensitivity;

    if (m_pitch > 60.0f) {
        m_pitch = 60.0f;
    }

    if (m_pitch < -10.0f) {
        m_pitch = -10.0f;
    }
}

void PlayerCube::update(float deltaTime) {
    m_position += m_velocity * deltaTime;
}

void PlayerCube::updateCamera(Camera& camera) {
    glm::vec3 forward = getForward();

    float pitchRadians = glm::radians(m_pitch);
    float horizontalDistance = m_thirdPersonDistance * cos(pitchRadians);
    float verticalOffset = m_thirdPersonDistance * sin(pitchRadians);

    glm::vec3 target = m_position + glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 cameraPosition =
        target - forward * horizontalDistance + glm::vec3(0.0f, m_thirdPersonHeight + verticalOffset, 0.0f);

    camera.lookAt(
        cameraPosition,
        glm::normalize(target - cameraPosition),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

void PlayerCube::render(Shader& shader, const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    shader.start();

    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, m_position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-m_yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, m_scale);

    shader.setMatrix("_modelMatrix", modelMatrix);
    shader.setMatrix("_viewMatrix", viewMatrix);
    shader.setMatrix("_projMatrix", projMatrix);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    shader.end();
}