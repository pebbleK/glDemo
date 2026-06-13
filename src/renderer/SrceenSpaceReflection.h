#pragma once

#include "Main.h"
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

    void beginCubeGBufferPass();
    void endCubeGBufferPass();

    GLuint getSceneColorTexture() const;
    GLuint getSceneDepthTexture() const;

    /*
    debug调试：把离屏渲染的颜色纹理m_sceneColorTexture绘制到屏幕上，
    方便观察当前FBO的中间结果。
    */
    void debugDraw();
 
    void drawCubeGBuffer(
    const glm::mat4 modelMatrix,
    const glm::mat4 viewMatrix,
    const glm::mat4 projMatrix);

    void drawReflectionCube(
    const glm::mat4& modelMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix);

    void blitSceneToDefaultFramebuffer();

    void setCubeModelMatrix(const glm::mat4 cubeModelMatrix);
    glm::mat4& getCubeModelMatrix();

private:
#ifndef APIENTRY
#define APIENTRY
#endif

    using DispatchComputeProc = void (APIENTRY *)(GLuint num_groups_x, GLuint num_group_y, GLuint num_groups_z);
    using BindImageTextureProc = void (APIENTRY *)(GLuint uint, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    using MemoryBarrierProc = void (APIENTRY *)(GLbitfield barriers);
    bool loadComputeFunctions();

    GLuint createComputeProgram(const char* path);

    bool createSceneFramebuffer(int width, int height);
    GLuint createFullscreenProgram();
    uint createFullScreenQuad();

    void getCamPos(Camera &_camera);

    //准备comp需要的数据缓冲
    bool createCubeGBuffer(int width, int height);
    GLuint createGBufferProgram();
    uint createReflectionPlant();

    void createReflectionTexture();
    void drawReflectionTexture();

    GLuint createReflectionCubeProgram();

private:
    int m_screenWidth;
    int m_screenHeight;
    int m_computeWidth;
    int m_computeHeight;
    glm::mat4 m_CubeModelMatrix;

    GLuint m_ssrComputeProgram;
    GLuint m_gbufferProgram;
    GLuint m_fullscreenProgram;
    GLuint m_cubeProgram;

    GLuint m_cubeVAO;
    GLuint m_quadVAO;

    GLuint m_sceneFBO;
    GLuint m_sceneColorTexture;
    GLuint m_sceneDepthTexture;

    // prepare to compute
    GLuint m_cubeGBufferFBO;
    GLuint m_cubePositionTexture;
    GLuint m_cubeNormalTexture;
    GLuint m_cubeMaskTexture;
    GLuint m_cubeDepthTexture;

    // output
    DispatchComputeProc m_glDispatchCompute;
    BindImageTextureProc m_glBindImageTexture;
    MemoryBarrierProc m_glMemoryBarrier;
    GLuint m_reflectionTexture;

};
