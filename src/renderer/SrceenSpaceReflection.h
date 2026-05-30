#pragma once

#include "Base.h"
#include "Camera.h"

class ScreenSpaceReflection{
public:
    ScreenSpaceReflection();
    ~ScreenSpaceReflection();

    bool init(int screenWidth, int screenHeight, int computerWidth, int computerHeight);

private:
    using DispatchComputeProc = void (APIENTRY *)(GLuint num_groups_x, GLuint num_group_y, GLuint num_groups_z);
    using BindImageTextureProc = void (APIENTRY *)(GLuint uint, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    using MemoryBarrierProc = void (APIENTRY *)(GLbitfield barriers);

    void getCamPos(Camera &_camera);

    uint createReflectionPlant();
    void draw();

    GLuint createComputeProgram(const char* path);
    GLuint createSSRProgram();

private:
    GLuint m_ssrProgram;
    GLuint m_ssrComputeProgram;
    glm::vec3 m_ssrCamera;

    GLuint m_sceneFBO;
    GLuint m_sceneColorTexture;
    GLuint m_sceneDepthTexture;

    GLuint m_cubeGBufferFBO;
    GLuint m_cubePositionTexture;
    GLuint m_cubeNormalTexture;
    GLuint m_cubeMaskTexture;

    GLuint m_reflectionTexture;

    GLuint m_cubeVAO;
    GLuint m_cubeProgram;
    GLuint m_gbufferProgram;
};