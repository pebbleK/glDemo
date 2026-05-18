# glDemo

## English

`glDemo` is a personal graphics programming demo built with C++ and OpenGL. The current version includes regular OpenGL scene rendering, external OBJ model loading, basic lighting, and a compute-shader-based black hole gravitational lensing background.

### Features

- Real-time rendering based on OpenGL Core Profile.
- OBJ/MTL model loading.
- Diffuse/specular material coefficient and texture loading.
- Basic lighting shaders for directional light, point light, and spotlight.
- Compute shader black hole gravitational lensing background.
- Layered composition between the black hole background and regular foreground models.
- Accretion disk, star field, and two orbiting lensing planets near the black hole.
- Foreground models can occlude the black hole background normally, simulating a composition between nearby objects and a distant black hole.

### Project Layout

```text
glDemo/
  glDemo.sln                 Visual Studio solution
  glDemo/
    openGLFW.cpp             Main application entry
    BlackHoleRenderer.*      Black hole background renderer
    Camera.*                 Camera movement and view matrix
    Shader.*                 Shader wrapper
    IO.*                     OBJ/MTL model loading
    ffImage.*                Image loading wrapper
    shader/
      geodesic.comp          Black hole geodesic compute shader
      sceneShaderv.glsl      Scene vertex shader
      sceneShaderf.glsl      Scene fragment shader
      vsunShader.glsl        Light cube vertex shader
      fsunShader.glsl        Light cube fragment shader
    res/model/
      ball.obj
      ball.mtl
```

### Requirements

- Windows
- Visual Studio 2022 or a compatible MSVC toolchain
- OpenGL 4.3 or newer
- A GPU and driver with compute shader support
- GLFW
- GLAD
- GLM
- Assimp

The project is currently organized as a Visual Studio project. Include and library paths for external dependencies must be configured locally in the Visual Studio project settings.

### Build And Run

1. Open `glDemo.sln` with Visual Studio.
2. Make sure the dependency paths are configured correctly, including GLFW, GLAD, GLM, and Assimp.
3. Select an `x64` configuration.
4. Build and run the `glDemo` project.

At runtime, make sure the working directory can access:

```text
shader/geodesic.comp
shader/sceneShaderv.glsl
shader/sceneShaderf.glsl
shader/vsunShader.glsl
shader/fsunShader.glsl
res/model/ball.obj
res/model/ball.mtl
```

### Controls

- `W` / `S`: Move forward / backward
- `A` / `D`: Move left / right
- `Space`: Move up
- `Left Ctrl`: Move down
- Mouse movement: Rotate camera
- `Esc`: Exit the program

### Black Hole Background

The black hole effect is not a static texture. It is computed every frame by the compute shader using the current camera rays. The rendering flow is roughly:

```text
1. Clear color and depth buffers
2. Use BlackHoleRenderer to compute the black hole background texture
3. Draw the black hole background to a fullscreen quad
4. Clear the depth buffer
5. Render foreground models and light sources normally
```

This makes the black hole behave as a distant background, while regular foreground objects can still occlude it through depth testing.

### Notes

- The current black hole effect is designed as a visual demo, not a strict physical simulation.
- The compute shader resolution is currently lower than the window resolution for performance.
- If the black hole background does not appear, confirm that the OpenGL version is at least 4.3 and check the console for shader compilation errors.

## 中文

`glDemo` 是一个基于 C++ 和 OpenGL 的个人图形学实验项目。当前版本主要包含普通 OpenGL 场景渲染、外部 OBJ 模型加载、基础光照，以及一个基于 compute shader 的黑洞引力透镜背景效果。

### 功能特性

- 基于 OpenGL Core Profile 的实时渲染。
- 支持 OBJ/MTL 模型读取。
- 支持 diffuse/specular 材质系数和贴图读取。
- 包含方向光、点光源和聚光灯的基础光照 shader。
- 使用 compute shader 生成黑洞引力透镜背景。
- 黑洞背景与普通前景模型分层合成。
- 黑洞附近包含吸积盘、星空背景和两个绕黑洞运动的透镜星球。
- 当前前景模型会正常遮挡黑洞背景，用于模拟近处物体与远处黑洞的组合画面。

### 项目结构

```text
glDemo/
  glDemo.sln                 Visual Studio solution
  glDemo/
    openGLFW.cpp             Main application entry
    BlackHoleRenderer.*      Black hole background renderer
    Camera.*                 Camera movement and view matrix
    Shader.*                 Shader wrapper
    IO.*                     OBJ/MTL model loading
    ffImage.*                Image loading wrapper
    shader/
      geodesic.comp          Black hole geodesic compute shader
      sceneShaderv.glsl      Scene vertex shader
      sceneShaderf.glsl      Scene fragment shader
      vsunShader.glsl        Light cube vertex shader
      fsunShader.glsl        Light cube fragment shader
    res/model/
      ball.obj
      ball.mtl
```

### 环境要求

- Windows
- Visual Studio 2022 或兼容的 MSVC 工具链
- OpenGL 4.3 或更高版本
- 支持 compute shader 的显卡和驱动
- GLFW
- GLAD
- GLM
- Assimp

项目当前通过 Visual Studio 工程文件组织。外部依赖的 include/lib 路径需要在本地 Visual Studio 项目属性中正确配置。

### 构建与运行

1. 使用 Visual Studio 打开 `glDemo.sln`。
2. 确认项目依赖项路径已经正确配置，包括 GLFW、GLAD、GLM 和 Assimp。
3. 选择 `x64` 配置。
4. 构建并运行 `glDemo` 项目。

运行时请确保工作目录能访问：

```text
shader/geodesic.comp
shader/sceneShaderv.glsl
shader/sceneShaderf.glsl
shader/vsunShader.glsl
shader/fsunShader.glsl
res/model/ball.obj
res/model/ball.mtl
```

### 控制方式

- `W` / `S`: 前进 / 后退
- `A` / `D`: 左移 / 右移
- `Space`: 上移
- `Left Ctrl`: 下移
- 鼠标移动: 旋转视角
- `Esc`: 退出程序

### 黑洞背景说明

黑洞效果不是一张静态贴图，而是每帧通过 compute shader 根据当前相机射线计算得到。渲染流程大致为：

```text
1. 清空颜色和深度缓冲
2. 使用 BlackHoleRenderer 计算黑洞背景纹理
3. 将黑洞背景绘制到全屏 quad
4. 清空深度缓冲
5. 正常渲染前景模型和光源
```

这种方式让黑洞作为远景存在，同时普通前景物体仍然可以通过深度测试遮挡背景。

### 注意事项

- 当前黑洞效果偏向视觉演示，不是严格物理模拟。
- compute shader 分辨率当前低于窗口分辨率，以换取性能。
- 如果看不到黑洞背景，请确认 OpenGL 版本至少为 4.3，并检查控制台中的 shader 编译错误信息。
