#pragma once
#include "Base.h"
#include "Camera.h"
#include "Shader.h"

class PlayerCube{
public:
    PlayerCube();
    ~PlayerCube();

    void init();
    void processInput(GLFWwindow *window);
    void onMouseMove(double xpos, double ypos);
    // 需要消除速度受帧率的影响
    void update(float deltaTime);
    void updateCamera(Camera &camera);
    void render(Shader &shader, const glm::mat4 &viewMatrix, const glm::mat4 &ProjMatrix);

    glm::vec3 getPosition() const;
    glm::vec3 getForward() const;

private:
    void createMesh();
    void destoryMesh();

    glm::vec3 getRight() const;

private:
    GLuint m_VAO;
    GLuint m_VBO;

    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_scale;

    float m_yaw;
    float m_pitch;
    float m_moveSpeed;
    float m_mouseSensitivity;
    float m_thirdPersonDistance;
    float m_thirdPersonHeight;

    double m_lastMouseX;
    double m_lastMouseY;
    bool m_firstMouse;

};