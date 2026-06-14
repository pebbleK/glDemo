#include "Player.h"

PlayerCube::PlayerCube()
: m_VAO(0)
, m_VBO(0)
, m_position(0.0f, 0.0f, 0.0f)
, m_velocity(0.0f)
, m_scale(0.5f, 1.0f, 0.5f)
, m_yaw(-90.0f)
, m_pitch(10.0f)
, m_moveSpeed(3.0f)
, m_mouseSensitivity(0.08f)
, m_thirdPersonDistance(4.0f)
, m_thirdPersonHeight(1.2f)
, m_lastMouseX(0.0)
, m_lastMouseY(0.0)
, m_firstMouse(true)
, m_verticalVelocity(0.0f)
, m_gravity(-0.98f)
, m_isGrounded(false)
{}

PlayerCube::~PlayerCube() {
}

void PlayerCube::init() {
    createMesh();
}

void PlayerCube::createMesh(){
    if(m_VAO != 0){
        return;
    }

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

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PlayerCube::processInput(GLFWwindow* window) {
    glm::vec3 forward = getForward();
    glm::vec3 right = getRight();
    glm::vec3 up = getUp();

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

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && m_isGrounded) {
    m_verticalVelocity = 5.0f;
    m_isGrounded = false;
    }

    // if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    //     m_velocity += up;
    // }

    // if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    //     m_velocity -= up;
    // }

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
    float yOffset = static_cast<float>(ypos - m_lastMouseY);

    m_lastMouseX = xpos;
    m_lastMouseY = ypos;

    m_yaw += xOffset * m_mouseSensitivity;
    m_pitch += yOffset * m_mouseSensitivity;

    if (m_pitch > 60.0f) {
        m_pitch = 60.0f;
    }

    if (m_pitch < -60.0f) {
        m_pitch = -60.0f;
    }
}

void PlayerCube::updateTime(float deltaTime, const TerrainCollider& terrain) {
    m_position.x += m_velocity.x * deltaTime;
    m_position.z += m_velocity.z * deltaTime;

    m_verticalVelocity += m_gravity * deltaTime;
    m_position.y += m_verticalVelocity * deltaTime;

    float groundY = 0.0f;
    if (terrain.getHeightAt(m_position.x, m_position.z, groundY)) {
        float halfHeight = m_scale.y * 0.5f;

        if (m_position.y - halfHeight < groundY) {
            m_position.y = groundY + halfHeight;
            m_verticalVelocity = 0.0f;
            m_isGrounded = true;
        } else {
            m_isGrounded = false;
        }
    } else {
        m_isGrounded = false;
    }
}
void PlayerCube::updateCamera(Camera& camera) {
    glm::vec3 forward = getForward();

    float pitchRadians = glm::radians(m_pitch);
    float horizontalDistance = m_thirdPersonDistance * cos(pitchRadians);
    float verticalOffset = m_thirdPersonDistance * sin(pitchRadians);

    glm::vec3 target = m_position + glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 cameraPosition =
        target - forward * horizontalDistance + glm::vec3(0.0f, m_thirdPersonHeight + verticalOffset, 0.0f);

    camera.lookAt(cameraPosition,
        glm::normalize(target - cameraPosition),
        glm::vec3(0.0f, 1.0f, 0.0f));
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

glm::vec3 PlayerCube::getPosition() const {
    return m_position;
}

glm::vec3 PlayerCube::getForward() const {
    float yawRadians = glm::radians(m_yaw);
    return glm::normalize(glm::vec3(
        cos(yawRadians),
        0.0f,
        sin(yawRadians)
    ));
}

glm::vec3 PlayerCube::getRight() const {
    return glm::normalize(glm::cross(getForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 PlayerCube::getUp() const{
    return glm::vec3(0.0f ,1.0f, 0.0f);
}