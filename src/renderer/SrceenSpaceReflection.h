#pragma once

#include "Base.h"
#include "Camera.h"

class ScreenSpaceReflection{
public:
    ScreenSpaceReflection();
    ~ScreenSpaceReflection();

    bool init(int screenWidth, int screenHeight, int computerWidth, int computerHeight);
    void resize(int screenWidth, int screenHeight);
    void dispatch(Camera &camera, const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix);

    void beginScenePass();
    void endScenePass();

    GLuint getSceneColorTexture() const;
    GLuint getSceneDepthTexture() const;

    /*
    debug调试：把离屏渲染的颜色纹理m_sceneColorTexture绘制到屏幕上，
    方便观察当前FBO的中间结果。
    */
    void debugDrawSceneColor();
    void debugDrawSceneDepth();
 
    void drawReflectionCube(Camera &camera,
    const glm::mat4 modelMatrix,
    const glm::mat4 viewMatrix,
    const glm::mat4 projMatrix);

private:
    using DispatchComputeProc = void (APIENTRY *)(GLuint num_groups_x, GLuint num_group_y, GLuint num_groups_z);
    using BindImageTextureProc = void (APIENTRY *)(GLuint uint, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    using MemoryBarrierProc = void (APIENTRY *)(GLbitfield barriers);
    bool loadComputeFunctions();

    void createFullScreenQuad();

    void getCamPos(Camera &_camera);

    uint createReflectionPlant();

    bool createSceneFramebuffer(int width, int height);

    GLuint createComputeProgram(const char* path);
    GLuint createFullscreenProgram();

    bool createCubeGBuffer(int width, int height);
    GLuint createGBufferProgram();

private:
    int m_screenWidth;
    int m_screenHeight;
    int m_computeWidth;
    int m_computeHeight;
    glm::vec3 m_ssrCamera;

    GLuint m_ssrComputeProgram;
    GLuint m_gbufferProgram;
    GLuint m_cubeProgram;
    GLuint m_fullscreenProgram;

    GLuint m_cubeVAO;
    GLuint m_cubeVBO;
    GLuint m_quadVAO;
    GLuint m_quadVBO;

    GLuint m_sceneFBO;
    GLuint m_sceneColorTexture;
    GLuint m_sceneDepthTexture;

    // save to compute
    GLuint m_cubeGBufferFBO;
    GLuint m_cubePositionTexture;
    GLuint m_cubeNormalTexture;
    GLuint m_cubeMaskTexture;
    GLuint m_cubeDepthTexture;

    // output
    GLuint m_reflectionTexture;

};
