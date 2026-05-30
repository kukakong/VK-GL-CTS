# VK-GL-CTS Code Wiki

> Khronos 图形 API 一致性测试套件（Conformance Test Suite）完整技术文档

---

## 目录

1. [项目概述](#1-项目概述)
2. [项目整体架构](#2-项目整体架构)
3. [目录结构总览](#3-目录结构总览)
4. [核心框架层（framework）](#4-核心框架层framework)
   - 4.1 [delibs 基础库](#41-delibs-基础库)
   - 4.2 [common 测试通用模块（tcutil）](#42-common-测试通用模块tcutil)
   - 4.3 [opengl OpenGL 工具模块（glutil）](#43-opengl-opengl-工具模块glutil)
   - 4.4 [egl EGL 工具模块（eglutil）](#44-egl-egl-工具模块eglutil)
   - 4.5 [referencerenderer 参考渲染器](#45-referencerenderer-参考渲染器)
   - 4.6 [randomshaders 随机着色器生成器](#46-randomshaders-随机着色器生成器)
   - 4.7 [qphelper C 辅助库](#47-qphelper-c-辅助库)
   - 4.8 [xexml XML 解析库](#48-xexml-xml-解析库)
   - 4.9 [platform 平台抽象层](#49-platform-平台抽象层)
5. [测试模块层（modules）](#5-测试模块层modules)
   - 5.1 [EGL 测试模块](#51-egl-测试模块)
   - 5.2 [OpenGL ES 2.0 测试模块](#52-opengl-es-20-测试模块)
   - 5.3 [OpenGL ES 3.0 测试模块](#53-opengl-es-30-测试模块)
   - 5.4 [OpenGL ES 3.1 测试模块](#54-opengl-es-31-测试模块)
   - 5.5 [GL 共享测试模块](#55-gl-共享测试模块)
   - 5.6 [内部测试模块](#56-内部测试模块)
6. [Vulkan CTS（external/vulkancts）](#6-vulkan-ctsexternalvulkancts)
   - 6.1 [Vulkan 框架层](#61-vulkan-框架层)
   - 6.2 [Vulkan 测试模块层](#62-vulkan-测试模块层)
7. [OpenGL CTS（external/openglcts）](#7-opengl-ctsexternalopenglcts)
   - 7.1 [通用测试模块](#71-通用测试模块)
   - 7.2 [OpenGL 桌面测试模块](#72-opengl-桌面测试模块)
   - 7.3 [OpenGL ES 测试模块](#73-opengl-es-测试模块)
   - 7.4 [OpenGL ES 扩展测试模块](#74-opengl-es-扩展测试模块)
8. [执行器与执行服务器](#8-执行器与执行服务器)
   - 8.1 [Executor 执行器框架](#81-executor-执行器框架)
   - 8.2 [ExecServer 执行服务器](#82-execserver-执行服务器)
9. [外部依赖](#9-外部依赖)
10. [构建系统与项目运行方式](#10-构建系统与项目运行方式)
11. [模块间依赖关系](#11-模块间依赖关系)
12. [测试执行流程](#12-测试执行流程)
13. [辅助脚本](#13-辅助脚本)

---

## 1. 项目概述

**VK-GL-CTS**（VK-GL Conformance Test Suite）是 Khronos 组织维护的图形 API 一致性测试套件，起源于 dEQP（drawElements Quality Program）。该项目用于验证图形驱动实现是否符合 Vulkan、OpenGL、OpenGL ES、EGL 等 API 规范。

### 核心特性

- **多 API 支持**：覆盖 Vulkan、Vulkan SC、OpenGL、OpenGL ES 2.0/3.0/3.1/3.2、EGL
- **跨平台**：支持 Windows、Linux、macOS、Android、iOS、Fuchsia、QNX 等
- **可扩展架构**：模块化设计，便于添加新 API 测试
- **远程执行**：支持通过 Executor/ExecServer 进行远程分布式测试
- **参考渲染器**：内置软件参考渲染器用于验证正确性

### 技术栈

| 类别 | 技术 |
|------|------|
| 主要语言 | C++ (C++11/14)、C |
| 构建系统 | CMake (≥ 3.20.0) |
| 脚本语言 | Python 3 |
| 着色器编译 | glslang / SPIR-V Tools |
| 图像处理 | libpng |
| 数据压缩 | zlib |
| 测试描述 | Amber |

---

## 2. 项目整体架构

VK-GL-CTS 采用分层架构设计，自底向上分为以下层次：

```
┌─────────────────────────────────────────────────────────────────────┐
│                        测试模块层 (Modules)                         │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌───────────┐ ┌─────────┐ │
│  │ EGL Tests│ │GLES2 Tests│ │GLES3 Tests│ │Vulkan Tests│ │GL CTS   │ │
│  └──────────┘ └──────────┘ └──────────┘ └───────────┘ └─────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                       测试框架层 (Framework)                        │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │ tcutil   │ │ glutil   │ │ eglutil  │ │ vkutil   │ │Platform │ │
│  │ (common) │ │ (opengl) │ │  (egl)   │ │ (vulkan) │ │  Layer  │ │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └─────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                      基础库层 (delibs)                              │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │  debase  │ │  decpp   │ │ depool   │ │ dethread │ │ deutil  │ │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └─────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                     执行与通信层 (Executor/ExecServer)              │
│  ┌──────────────────────┐    ┌──────────────────────────────────┐  │
│  │   BatchExecutor      │◄──►│      ExecutionServer             │  │
│  │   CommLink/TcpIpLink │    │   TcpServer/TestDriver           │  │
│  └──────────────────────┘    └──────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────────────┤
│                       外部依赖 (External)                           │
│  glslang | SPIR-V Tools | libpng | zlib | Amber | jsoncpp          │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. 目录结构总览

```
VK-GL-CTS/
├── android/                  # Android 构建配置（CTS、APK）
├── doc/                      # 文档（测试日志样式、测试规范模板）
├── execserver/               # 执行服务器（远程测试执行服务端）
├── executor/                 # 执行器框架（批量测试执行与通信）
├── external/                 # 外部依赖与扩展测试套件
│   ├── amber/                #   Amber 测试框架
│   ├── glslang/              #   GLSL/SPIR-V 编译器
│   ├── jsoncpp/              #   JSON 解析库
│   ├── libpng/               #   PNG 图像库
│   ├── openglcts/            #   OpenGL/OpenGL ES CTS 测试模块
│   ├── spirv-tools/          #   SPIR-V 工具
│   ├── vulkan-validationlayers/ # Vulkan 验证层
│   ├── vulkancts/            #   Vulkan CTS 测试模块
│   ├── vulkansc-pcutil/      #   Vulkan SC 管线缓存工具
│   ├── vulkan-video-samples/ #   Vulkan 视频示例
│   └── zlib/                 #   zlib 压缩库
├── framework/                # 核心测试框架
│   ├── common/               #   通用测试工具（tcutil）
│   ├── delibs/               #   基础库（debase/decpp/depool/dethread/deutil/deimage/destream）
│   ├── egl/                  #   EGL 工具库（eglutil）
│   ├── opengl/               #   OpenGL 工具库（glutil）
│   ├── platform/             #   平台抽象层（各平台实现）
│   ├── qphelper/             #   C 辅助库（日志/崩溃处理/看门狗）
│   ├── randomshaders/        #   随机着色器生成器
│   ├── referencerenderer/    #   参考渲染器
│   └── xexml/                #   XML 解析库
├── modules/                  # 内置测试模块
│   ├── egl/                  #   EGL 测试
│   ├── gles2/                #   OpenGL ES 2.0 测试
│   ├── gles3/                #   OpenGL ES 3.0 测试（功能+性能）
│   ├── gles31/               #   OpenGL ES 3.1 测试
│   ├── glshared/             #   GL 共享测试工具
│   └── internal/             #   内部框架测试
├── scripts/                  # 辅助脚本
│   ├── android/              #   Android 构建脚本
│   ├── ctsbuild/             #   CTS 构建脚本
│   ├── egl/                  #   EGL 代码生成脚本
│   ├── khr_util/             #   Khronos 注册表工具
│   ├── log/                  #   日志分析脚本
│   ├── opengl/               #   OpenGL 代码生成脚本
│   └── src_util/             #   源码检查工具
├── CMakeLists.txt            # 顶层 CMake 构建文件
├── README.md                 # 项目说明
└── LICENSE                   # Apache 2.0 许可证
```

---

## 4. 核心框架层（framework）

### 4.1 delibs 基础库

delibs 是整个项目的基础库集合，提供跨平台的底层功能，分为 C 语言接口（debase 等）和 C++ 封装（decpp）两个层次。

#### 4.1.1 debase — C 基础库

| 文件 | 功能 |
|------|------|
| `deDefs.h` | 基础类型定义、宏、编译器抽象 |
| `deMath.h/c` | 数学工具函数（最大/最小值、对齐、浮点运算） |
| `deMemory.h/c` | 内存管理（分配、释放、对齐分配） |
| `deString.h/c` | 字符串操作工具 |
| `deRandom.h/c` | 随机数生成器 |
| `deSha1.h/c` | SHA-1 哈希计算 |
| `deFloat16.h/c` | 16 位浮点数转换 |
| `deInt32.h/c` | 32 位整数运算工具 |

**关键定义**：
- `DE_OS` 系列宏：`DE_OS_WIN32`、`DE_OS_UNIX`、`DE_OS_ANDROID` 等，用于编译时平台检测
- `DE_CPU` 系列宏：`DE_CPU_X86`、`DE_CPU_ARM` 等，用于 CPU 架构检测
- `DE_ASSERT` / `DE_TEST_ASSERT`：断言宏

#### 4.1.2 decpp — C++ 基础库

| 文件 | 功能 |
|------|------|
| `deDefs.hpp` | C++ 基础定义和异常类 |
| `deUniquePtr.hpp` | 唯一指针（类似 std::unique_ptr） |
| `deSharedPtr.hpp` | 共享指针（类似 std::shared_ptr） |
| `deThread.hpp` | C++ 线程封装 |
| `deMutex.hpp` | C++ 互斥锁封装 |
| `deSemaphore.hpp` | C++ 信号量封装 |
| `deSocket.hpp` | C++ 套接字封装 |
| `deCommandLine.hpp` | C++ 命令行解析 |
| `deProcess.hpp` | C++ 进程管理 |
| `deDynamicLibrary.hpp` | C++ 动态库加载 |
| `deFilePath.hpp` | C++ 文件路径操作 |
| `deDirectoryIterator.hpp` | C++ 目录遍历 |
| `deIOStream.hpp` | C++ I/O 流工具 |
| `deRingBuffer.hpp` | C++ 环形缓冲区 |
| `deThreadPool.hpp` | C++ 线程池 |
| `deArrayBuffer.hpp` | C++ 数组缓冲区 |
| `deBlockBuffer.hpp` | C++ 块缓冲区 |
| `deRandom.hpp` | C++ 随机数工具 |
| `deSha1.hpp` | C++ SHA-1 工具 |
| `deSTLUtil.hpp` | STL 辅助工具 |
| `deStringUtil.hpp` | 字符串工具 |
| `deClock.hpp` | 时钟和计时工具 |
| `deTimer.hpp` | 定时器工具 |

#### 4.1.3 depool — 内存池库

| 文件 | 功能 |
|------|------|
| `deMemPool.h/c` | 内存池管理器 |
| `dePoolArray.h/c` | 池化动态数组 |
| `dePoolHash.h/c` | 池化哈希表 |
| `dePoolHashSet.h/c` | 池化哈希集合 |
| `dePoolHashArray.h/c` | 池化哈希数组 |
| `dePoolHeap.h/c` | 池化堆 |
| `dePoolSet.h/c` | 池化有序集合 |
| `dePoolMultiSet.h/c` | 池化多重集合 |
| `dePoolStringBuilder.h/c` | 池化字符串构建器 |

#### 4.1.4 dethread — 线程与同步原语

| 文件 | 功能 |
|------|------|
| `deAtomic.h/c` | 原子操作 |
| `deMutex.h` | 互斥锁（Unix/Win32 实现） |
| `deSemaphore.h` | 信号量（Unix/Win32 实现） |
| `deThread.h` | 线程（Unix/Win32 实现） |
| `deThreadLocal.h` | 线程局部存储 |
| `deSingleton.h/c` | 单例模式 |
| `deSpinBarrier.h` | 自旋屏障 |

#### 4.1.5 deutil — 实用工具

| 文件 | 功能 |
|------|------|
| `deClock.h/c` | 时钟/计时功能 |
| `deCommandLine.h/c` | 命令行参数解析 |
| `deDynamicLibrary.h/c` | 动态库加载 |
| `deFile.h/c` | 文件操作 |
| `deProcess.h/c` | 进程管理 |
| `deSocket.h/c` | 套接字通信 |
| `deTimer.h/c` | 定时器 |

#### 4.1.6 deimage — 图像处理

| 文件 | 功能 |
|------|------|
| `deImage.h/c` | 基础图像结构和操作 |
| `deARGB.h` | ARGB 像素格式定义 |
| `deTarga.h/c` | Targa (TGA) 图像读写 |

#### 4.1.7 destream — 流操作

| 文件 | 功能 |
|------|------|
| `deIOStream.h` | I/O 流接口 |
| `deInStream.h` | 输入流接口 |
| `deOutStream.h` | 输出流接口 |
| `deFileStream.h/c` | 文件流实现 |
| `deRingbuffer.h/c` | 环形缓冲区 |
| `deThreadStream.h/c` | 线程安全流 |
| `deStreamCpyThread.h/c` | 流复制线程 |

### 4.2 common 测试通用模块（tcutil）

tcutil 是测试框架的核心，提供测试用例管理、执行引擎、命令行处理、日志记录等核心功能。

#### 关键类与文件

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `tcuApp.hpp/cpp` | `tcu::App` | 应用程序入口类，管理测试执行主循环和全局配置 |
| `tcuTestCase.hpp/cpp` | `tcu::TestCase` | 测试用例基类，所有具体测试用例的父类 |
| | `tcu::TestNode` | 测试树节点基类 |
| | `tcu::TestCaseGroup` | 测试用例组，组织测试层次结构 |
| `tcuTestPackage.hpp/cpp` | `tcu::TestPackage` | 测试包类，组织一组相关测试用例 |
| `tcuTestContext.hpp/cpp` | `tcu::TestContext` | 测试上下文，提供测试执行所需的共享资源 |
| `tcuTestSessionExecutor.hpp/cpp` | `tcu::TestSessionExecutor` | 测试会话执行器，管理整个测试会话的执行流程 |
| `tcuCommandLine.hpp/cpp` | `tcu::CommandLine` | 命令行参数解析器，配置测试运行参数 |
| `tcuTestLog.hpp/cpp` | `tcu::TestLog` | 测试日志记录器，支持多种日志条目类型 |
| `tcuPlatform.hpp/cpp` | `tcu::Platform` | 平台抽象基类，提供跨平台接口 |
| `tcuRenderTarget.hpp/cpp` | `tcu::RenderTarget` | 渲染目标描述 |
| `tcuResource.hpp/cpp` | `tcu::ResourceProvider` / `tcu::DirArchive` | 资源提供者和目录归档访问 |
| `tcuTexture.hpp/cpp` | `tcu::TextureFormat` / `tcu::ConstPixelBufferAccess` | 纹理格式和像素缓冲区访问 |
| `tcuTextureUtil.hpp/cpp` | 纹理工具函数 | 纹理操作辅助函数 |
| `tcuImageCompare.hpp/cpp` | `tcu::computeImageCompareResult` | 图像比较算法（精确/模糊/双线性） |
| `tcuFuzzyImageCompare.hpp/cpp` | `tcu::FuzzyImageCompare` | 模糊图像比较算法 |
| `tcuFloat.hpp/cpp` | `tcu::Float` / `tcu::Float16` | 浮点数操作工具 |
| `tcuFloatFormat.hpp/cpp` | `tcu::FloatFormat` | 浮点格式描述 |
| `tcuInterval.hpp/cpp` | `tcu::Interval` | 区间算术（用于精度验证） |
| `tcuMatrix.hpp/cpp` | `tcu::Matrix` | 矩阵运算 |
| `tcuVector.hpp` | `tcu::Vector` | 向量类型 |
| `tcuArray.hpp/cpp` | `tcu::Array` | 动态数组 |
| `tcuMaybe.hpp/cpp` | `tcu::Maybe` | 可选值类型 |
| `tcuEither.hpp/cpp` | `tcu::Either` | 二选一类型 |
| `tcuStringTemplate.hpp/cpp` | `tcu::StringTemplate` | 字符串模板引擎 |
| `tcuFactoryRegistry.hpp/cpp` | `tcu::FactoryRegistry` | 工厂注册表 |
| `tcuResultCollector.hpp/cpp` | `tcu::ResultCollector` | 测试结果收集器 |
| `tcuSeedBuilder.hpp/cpp` | `tcu::SeedBuilder` | 随机种子构建器 |
| `tcuWaiverUtil.hpp/cpp` | `tcu::WaiverUtil` | 测试豁免工具 |
| `tcuCompressedTexture.hpp/cpp` | `tcu::CompressedTexture` | 压缩纹理处理 |
| `tcuAstcUtil.hpp/cpp` | ASTC 工具 | ASTC 纹理编解码辅助 |
| `tcuImageIO.hpp/cpp` | `tcu::ImageIO` | 图像 I/O（PNG 读写） |
| `tcuCPUWarmup.hpp/cpp` | CPU 预热 | CPU 频率稳定化预热 |
| `tcuTestHierarchyIterator.hpp/cpp` | `tcu::TestHierarchyIterator` | 测试层次结构迭代器 |
| `tcuTestHierarchyUtil.hpp/cpp` | 测试层次工具 | 测试树遍历和过滤工具 |
| `tcuPixelFormat.hpp` | `tcu::PixelFormat` | 像素格式描述 |
| `tcuRGBA.hpp/cpp` | `tcu::RGBA8` | RGBA8 像素类型 |
| `tcuSurface.hpp/cpp` | `tcu::Surface` | 像素表面（用于渲染结果捕获） |
| `tcuTexLookupVerifier.hpp/cpp` | 纹理查找验证 | 纹理采样结果验证器 |
| `tcuTexCompareVerifier.hpp/cpp` | 纹理比较验证 | 纹理比较结果验证器 |
| `tcuRasterizationVerifier.hpp/cpp` | 光栅化验证 | 光栅化结果验证器 |
| `tcuThreadUtil.hpp/cpp` | 线程工具 | 多线程辅助工具 |

#### 测试执行主循环（tcuMain.cpp）

```cpp
int main(int argc, char **argv) {
    tcu::CommandLine cmdLine(argc, argv);       // 解析命令行
    tcu::DirArchive archive(cmdLine.getArchiveDir()); // 创建资源归档
    tcu::TestLog log(cmdLine.getLogFileName(), cmdLine.getLogFlags()); // 创建日志
    de::UniquePtr<tcu::Platform> platform(createPlatform()); // 创建平台
    de::UniquePtr<tcu::App> app(new tcu::App(*platform, archive, log, cmdLine)); // 创建应用

    for (;;) {
        if (!app->iterate()) break;  // 迭代执行测试
    }
}
```

### 4.3 opengl OpenGL 工具模块（glutil）

提供 OpenGL / OpenGL ES 的上下文管理、着色器编译、绘制工具等。

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `gluRenderContext.hpp/cpp` | `glu::RenderContext` | 渲染上下文抽象基类 |
| `gluContextFactory.hpp/cpp` | `glu::ContextFactory` | 上下文工厂 |
| `gluContextInfo.hpp/cpp` | `glu::ContextInfo` | 上下文信息查询 |
| `gluShaderProgram.hpp/cpp` | `glu::ShaderProgram` | 着色器程序管理 |
| `gluShaderLibrary.hpp/cpp` | `glu::ShaderLibrary` | 着色器库（XML 格式） |
| `gluShaderUtil.hpp/cpp` | `glu::ShaderUtil` | 着色器编译工具 |
| `gluDrawUtil.hpp/cpp` | `glu::DrawUtil` | 绘制工具（网格生成等） |
| `gluTexture.hpp/cpp` | `glu::Texture` | 纹理创建和管理 |
| `gluTextureUtil.hpp/cpp` | `glu::TextureUtil` | 纹理格式转换工具 |
| `gluTextureTestUtil.hpp/cpp` | `glu::TextureTestUtil` | 纹理测试工具 |
| `gluObjectWrapper.hpp/cpp` | `glu::ObjectWrapper` | GL 对象 RAII 封装 |
| `gluFboRenderContext.hpp/cpp` | `glu::FboRenderContext` | FBO 渲染上下文 |
| `gluRenderConfig.hpp/cpp` | `glu::RenderConfig` | 渲染配置 |
| `gluVarType.hpp/cpp` | `glu::VarType` | 变量类型系统 |
| `gluVarTypeUtil.hpp/cpp` | `glu::VarTypeUtil` | 变量类型工具 |
| `gluCallLogWrapper.hpp/cpp` | `glu::CallLogWrapper` | GL API 调用日志包装器 |
| `gluStrUtil.hpp/cpp` | `glu::StrUtil` | GL 枚举/函数名字符串工具 |
| `gluPixelTransfer.hpp/cpp` | `glu::PixelTransfer` | 像素传输工具 |
| `gluProgramInterfaceQuery.hpp/cpp` | `glu::ProgramInterfaceQuery` | 程序接口查询工具 |
| `gluStateReset.hpp/cpp` | `glu::StateReset` | GL 状态重置 |
| `gluPlatform.hpp/cpp` | `glu::Platform` | OpenGL 平台接口 |
| `gluDummyRenderContext.hpp/cpp` | `glu::DummyRenderContext` | 空渲染上下文（用于非渲染测试） |

### 4.4 egl EGL 工具模块（eglutil）

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `egluPlatform.hpp/cpp` | `eglu::Platform` | EGL 平台接口 |
| `egluConfigFilter.hpp/cpp` | `eglu::ConfigFilter` | EGL 配置过滤器 |
| `egluConfigInfo.hpp/cpp` | `eglu::ConfigInfo` | EGL 配置信息 |
| `eguNativeDisplay.hpp/cpp` | `eglu::NativeDisplay` | 原生显示抽象 |
| `egluNativeWindow.hpp/cpp` | `eglu::NativeWindow` | 原生窗口抽象 |
| `egluNativePixmap.hpp/cpp` | `eglu::NativePixmap` | 原生像素图抽象 |
| `egluGLContextFactory.hpp/cpp` | `eglu::GLContextFactory` | GL 上下文工厂（基于 EGL） |
| `egluGLFunctionLoader.hpp/cpp` | `eglu::GLFunctionLoader` | GL 函数加载器 |
| `egluGLUtil.hpp/cpp` | `eglu::GLUtil` | GL/EGL 互操作工具 |
| `egluUtil.hpp/cpp` | `eglu::Util` | EGL 通用工具 |
| `egluCallLogWrapper.hpp/cpp` | `eglu::CallLogWrapper` | EGL API 调用日志包装器 |
| `egluStrUtil.hpp/cpp` | `eglu::StrUtil` | EGL 枚举/函数名字符串工具 |
| `egluUnique.hpp/cpp` | `eglu::Unique` | EGL 对象 RAII 封装 |
| `egluDefs.hpp/cpp` | `eglu::Defs` | EGL 基础定义 |

### 4.5 referencerenderer 参考渲染器

软件实现的参考渲染器，用于验证 GPU 渲染结果的正确性。

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `rrRenderer.hpp/cpp` | `rr::Renderer` | 参考渲染器主类，执行完整渲染管线 |
| `rrRasterizer.hpp/cpp` | `rr::Rasterizer` | 软件光栅化器 |
| `rrShaders.hpp/cpp` | `rr::Shader` | 着色器执行接口 |
| `rrShadingContext.hpp/cpp` | `rr::ShadingContext` | 着色上下文 |
| `rrFragmentOperations.hpp/cpp` | `rr::FragmentOperations` | 片段操作（深度测试/模板测试/混合） |
| `rrFragmentPacket.hpp` | `rr::FragmentPacket` | 片段数据包 |
| `rrPrimitiveAssembler.hpp` | `rr::PrimitiveAssembler` | 图元装配器 |
| `rrPrimitivePacket.hpp/cpp` | `rr::PrimitivePacket` | 图元数据包 |
| `rrPrimitiveTypes.hpp` | `rr::PrimitiveType` | 图元类型定义 |
| `rrVertexAttrib.hpp/cpp` | `rr::VertexAttrib` | 顶点属性 |
| `rrVertexPacket.hpp/cpp` | `rr::VertexPacket` | 顶点数据包 |
| `rrRenderState.hpp` | `rr::RenderState` | 渲染状态 |
| `rrMultisamplePixelBufferAccess.hpp/cpp` | 多采样像素缓冲区访问 | |
| `rrDefs.hpp/cpp` | `rr::Defs` | 参考渲染器基础定义 |
| `rrGenericVector.hpp` | `rr::GenericVector` | 通用向量类型 |

### 4.6 randomshaders 随机着色器生成器

自动生成随机着色器程序用于压力测试和模糊测试。

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `rsgProgramGenerator.hpp/cpp` | `rsg::ProgramGenerator` | 着色器程序生成器 |
| `rsgShaderGenerator.hpp/cpp` | `rsg::ShaderGenerator` | 着色器生成器 |
| `rsgExpressionGenerator.hpp/cpp` | `rsg::ExpressionGenerator` | 表达式生成器 |
| `rsgFunctionGenerator.hpp/cpp` | `rsg::FunctionGenerator` | 函数生成器 |
| `rsgProgramExecutor.hpp/cpp` | `rsg::ProgramExecutor` | 着色器程序执行器 |
| `rsgExpression.hpp/cpp` | `rsg::Expression` | 表达式基类 |
| `rsgStatement.hpp/cpp` | `rsg::Statement` | 语句基类 |
| `rsgShader.hpp/cpp` | `rsg::Shader` | 着色器表示 |
| `rsgVariable.hpp/cpp` | `rsg::Variable` | 变量表示 |
| `rsgVariableType.hpp/cpp` | `rsg::VariableType` | 变量类型 |
| `rsgVariableValue.hpp/cpp` | `rsg::VariableValue` | 变量值 |
| `rsgVariableManager.hpp/cpp` | `rsg::VariableManager` | 变量管理器 |
| `rsgGeneratorState.hpp/cpp` | `rsg::GeneratorState` | 生成器状态 |
| `rsgNameAllocator.hpp/cpp` | `rsg::NameAllocator` | 名称分配器 |
| `rsgSamplers.hpp/cpp` | `rsg::Samplers` | 采样器 |
| `rsgBinaryOps.hpp/cpp` | `rsg::BinaryOps` | 二元操作 |
| `rsgBuiltinFunctions.hpp/cpp` | `rsg::BuiltinFunctions` | 内建函数 |
| `rsgPrettyPrinter.hpp/cpp` | `rsg::PrettyPrinter` | 代码美化打印 |
| `rsgToken.hpp/cpp` | `rsg::Token` | 词法标记 |

### 4.7 qphelper C 辅助库

纯 C 实现的辅助库，提供底层日志记录、崩溃处理和看门狗功能。

| 文件 | 关键函数 | 说明 |
|------|---------|------|
| `qpTestLog.h/c` | `qpTestLog_create` / `qpTestLog_write` | 测试日志 C 接口（底层实现） |
| `qpCrashHandler.h/c` | `qpCrashHandler_init` | 崩溃信号处理器 |
| `qpWatchDog.h/c` | `qpWatchDog_create` / `qpWatchDog_touch` | 看门狗定时器（检测挂起） |
| `qpDebugOut.h/c` | `qpPrint` / `qpPrintf` | 调试输出 |
| `qpXmlWriter.h/c` | `qpXmlWriter_create` | XML 写入器 |
| `qpInfo.h/c` | `qpGetReleaseId` / `qpGetTargetName` | 版本和构建信息 |

### 4.8 xexml XML 解析库

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `xeXMLParser.hpp/cpp` | `xe::XMLParser` | 轻量级 XML 解析器 |
| `xeDefs.hpp/cpp` | `xe::Defs` | XML 解析器基础定义 |

### 4.9 platform 平台抽象层

提供各操作系统的平台实现，每个平台实现 `tcu::Platform` 接口和相关的窗口/显示管理。

| 目录 | 平台 | 关键文件 |
|------|------|---------|
| `android/` | Android | `tcuAndroidPlatform.hpp/cpp`、`tcuAndroidNativeActivity.hpp/cpp`、`tcuAndroidJNI.cpp` |
| `lnx/` | Linux | `tcuLnxPlatform.hpp/cpp`、`tcuLnxVulkanPlatform.hpp/cpp`、X11/Wayland 子目录 |
| `win32/` | Windows | `tcuWin32Platform.hpp/cpp`、`tcuWin32VulkanPlatform.hpp/cpp`、`tcuWGL.hpp/cpp` |
| `osx/` | macOS | `tcuOSXPlatform.hpp/cpp`、`tcuOSXVulkanPlatform.hpp/cpp` |
| `ios/` | iOS | `tcuIOSPlatform.hh/mm`、`tcuEAGLView.h/m` |
| `fuchsia/` | Fuchsia | `tcuFuchsiaPlatform.cpp` |
| `qnx/` | QNX | `tcuQnxScreenPlatform.hpp/cpp` |
| `raspi/` | Raspberry Pi | `tcuRaspiPlatform.hpp/cpp` |
| `surfaceless/` | 无窗口 Linux | `tcuSurfacelessPlatform.hpp/cpp` |
| `null/` | 空平台（测试用） | `tcuNullPlatform.hpp/cpp` |
| `nullws/` | 无窗口系统 | `tcuNullWSPlatform.hpp/cpp` |
| `vanilla/` | 通用默认 | `tcuVanillaPlatform.cpp` |

---

## 5. 测试模块层（modules）

### 5.1 EGL 测试模块

路径：`modules/egl/`，前缀 `tegl`

| 文件 | 说明 |
|------|------|
| `teglTestPackage.hpp/cpp` | EGL 测试包入口 |
| `teglApiCase.hpp/cpp` | EGL API 测试基类 |
| `teglChooseConfigTests.hpp/cpp` | 配置选择测试 |
| `teglCreateContextTests.hpp/cpp` | 上下文创建测试 |
| `teglCreateSurfaceTests.hpp/cpp` | Surface 创建测试 |
| `teglSwapBuffersTests.hpp/cpp` | 缓冲区交换测试 |
| `teglSyncTests.hpp/cpp` | 同步对象测试 |
| `teglRenderTests.hpp/cpp` | 渲染测试 |
| `teglImageTests.hpp/cpp` | EGLImage 测试 |
| `teglMultiContextTests.hpp/cpp` | 多上下文测试 |
| `teglMultiThreadTests.hpp/cpp` | 多线程测试 |
| `teglRobustnessTests.hpp/cpp` | 鲁棒性测试 |
| `teglPartialUpdateTests.hpp/cpp` | 局部更新测试 |
| `teglNegativeApiTests.hpp/cpp` | 负面 API 测试 |
| `teglBufferAgeTests.hpp/cpp` | 缓冲区年龄测试 |
| `teglWideColorTests.hpp/cpp` | 广色域测试 |
| `teglMemoryStressTests.hpp/cpp` | 内存压力测试 |
| `teglMakeCurrentPerfTests.hpp/cpp` | MakeCurrent 性能测试 |

### 5.2 OpenGL ES 2.0 测试模块

路径：`modules/gles2/`，前缀 `tes2`

| 文件 | 说明 |
|------|------|
| `tes2TestPackage.hpp/cpp` | GLES2 测试包 |
| `tes2Context.hpp/cpp` | GLES2 测试上下文 |
| `tes2CapabilityTests.hpp/cpp` | 能力测试 |
| `tes2InfoTests.hpp/cpp` | 信息查询测试 |

### 5.3 OpenGL ES 3.0 测试模块

路径：`modules/gles3/`，前缀 `tes3`

包含功能测试（`functional/`）和性能测试（`performance/`）两个子目录。

**功能测试**（`functional/`，前缀 `es3f`）：

| 文件 | 说明 |
|------|------|
| `es3fFboApiTests.hpp` | FBO API 测试 |
| `es3fFboRenderTest.hpp` | FBO 渲染测试 |
| `es3fFboDepthbufferTests.hpp` | FBO 深度缓冲测试 |
| `es3fTextureFilteringTests.hpp` | 纹理过滤测试 |
| `es3fTextureMipmapTests.hpp` | 纹理 Mipmap 测试 |
| `es3fTextureSpecificationTests.cpp` | 纹理规格测试 |
| `es3fShaderLoopTests.cpp` | 着色器循环测试 |
| `es3fShaderOperatorTests.cpp` | 着色器运算符测试 |
| `es3fShaderTextureFunctionTests.cpp` | 着色器纹理函数测试 |
| `es3fShaderBuiltinVarTests.hpp` | 着色器内建变量测试 |
| `es3fBlendTests.cpp` | 混合测试 |
| `es3fDepthStencilTests.cpp` | 深度/模板测试 |
| `es3fClippingTests.cpp` | 裁剪测试 |
| `es3fInstancedRenderingTests.hpp` | 实例化渲染测试 |
| `es3fUniformBlockTests.hpp` | Uniform 块测试 |
| `es3fNegativeFragmentApiTests.hpp` | 负面片段 API 测试 |
| `es3fNegativeTextureApiTests.hpp` | 负面纹理 API 测试 |

**性能测试**（`performance/`，前缀 `es3p`）：

| 文件 | 说明 |
|------|------|
| `es3pPerformanceTests.hpp/cpp` | 性能测试入口 |
| `es3pBlendTests.hpp/cpp` | 混合性能测试 |
| `es3pTextureFilteringTests.hpp/cpp` | 纹理过滤性能测试 |
| `es3pShaderCompilerTests.hpp/cpp` | 着色器编译性能测试 |
| `es3pShaderOperatorTests.hpp/cpp` | 着色器运算性能测试 |
| `es3pStateChangeTests.hpp/cpp` | 状态切换性能测试 |

### 5.4 OpenGL ES 3.1 测试模块

路径：`modules/gles31/`，前缀 `tes31`

| 文件 | 说明 |
|------|------|
| `tes31TestPackage.hpp/cpp` | GLES31 测试包 |
| `tes31Context.hpp/cpp` | GLES31 测试上下文 |

### 5.5 GL 共享测试模块

路径：`modules/glshared/`，前缀 `gls`

提供 OpenGL / OpenGL ES 各版本共享的测试工具和测试用例。

| 文件 | 说明 |
|------|------|
| `glsShaderLibrary.hpp/cpp` | 着色器库解析器 |
| `glsShaderLibraryCase.hpp/cpp` | 着色器库测试用例 |
| `glsShaderExecUtil.hpp/cpp` | 着色器执行工具 |
| `glsShaderRenderCase.hpp/cpp` | 着色器渲染测试用例 |
| `glsShaderPerformanceCase.hpp/cpp` | 着色器性能测试用例 |
| `glsBuiltinPrecisionTests.hpp/cpp` | 内建函数精度测试 |
| `glsBufferTestUtil.hpp/cpp` | 缓冲区测试工具 |
| `glsFboUtil.hpp/cpp` | FBO 工具 |
| `glsFboCompletenessTests.hpp/cpp` | FBO 完整性测试 |
| `glsTextureTestUtil.hpp/cpp` | 纹理测试工具 |
| `glsVertexArrayTests.hpp/cpp` | 顶点数组测试 |
| `glsUniformBlockCase.hpp/cpp` | Uniform 块测试用例 |
| `glsRandomUniformBlockCase.hpp/cpp` | 随机 Uniform 块测试 |
| `glsDrawTest.hpp/cpp` | 绘制测试 |
| `glsLifetimeTests.hpp/cpp` | 对象生命周期测试 |
| `glsInteractionTestUtil.hpp/cpp` | 交互测试工具 |
| `glsCalibration.hpp/cpp` | 性能校准工具 |
| `glsStateQueryUtil.hpp/cpp` | 状态查询工具 |
| `glsScissorTests.hpp/cpp` | 裁剪测试 |
| `glsSamplerObjectTest.hpp/cpp` | 采样器对象测试 |
| `glsLongStressCase.hpp/cpp` | 长时间压力测试 |
| `glsMemoryStressCase.hpp/cpp` | 内存压力测试 |

### 5.6 内部测试模块

路径：`modules/internal/`，前缀 `dit`

用于验证框架自身正确性的内部测试。

| 文件 | 说明 |
|------|------|
| `ditTestPackage.hpp/cpp` | 内部测试包 |
| `ditFrameworkTests.hpp/cpp` | 框架测试 |
| `ditImageCompareTests.hpp/cpp` | 图像比较测试 |
| `ditImageIOTests.hpp/cpp` | 图像 I/O 测试 |
| `ditTextureFormatTests.hpp/cpp` | 纹理格式测试 |
| `ditAstcTests.hpp/cpp` | ASTC 测试 |
| `ditSRGB8ConversionTest.hpp/cpp` | sRGB8 转换测试 |
| `ditDelibsTests.hpp/cpp` | delibs 测试 |
| `ditBuildInfoTests.hpp/cpp` | 构建信息测试 |
| `ditSeedBuilderTests.hpp/cpp` | 种子构建器测试 |
| `ditTestLogTests.hpp/cpp` | 测试日志测试 |
| `ditVulkanTests.hpp/cpp` | Vulkan 框架测试 |

---

## 6. Vulkan CTS（external/vulkancts）

### 6.1 Vulkan 框架层

路径：`external/vulkancts/framework/vulkan/`，前缀 `vk`

提供 Vulkan API 的封装、设备管理、内存管理、管线构建等基础设施。

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `vkDefs.hpp/cpp` | `vk::Defs` | Vulkan 基础类型定义 |
| `vkPlatform.hpp/cpp` | `vk::Platform` | Vulkan 平台接口 |
| `vkDeviceUtil.hpp/cpp` | `vk::DeviceUtil` | 设备创建和管理工具 |
| `vkDeviceFeatures.hpp/cpp` | `vk::DeviceFeatures` | 设备特性管理 |
| `vkDeviceProperties.hpp/cpp` | `vk::DeviceProperties` | 设备属性管理 |
| `vkMemUtil.hpp/cpp` | `vk::Allocator` / `vk::MemoryRequirement` | 内存分配和管理 |
| `vkImageUtil.hpp/cpp` | `vk::ImageUtil` | 图像格式和操作工具 |
| `vkRef.hpp/cpp` | `vk::Reference` / `vk::Move` | Vulkan 对象 RAII 引用 |
| `vkRefUtil.hpp/cpp` | `vk::RefUtil` | 引用工具函数 |
| `vkBufferWithMemory.hpp/cpp` | `vk::BufferWithMemory` | 带内存的缓冲区 |
| `vkImageWithMemory.hpp/cpp` | `vk::ImageWithMemory` | 带内存的图像 |
| `vkPipelineConstructionUtil.hpp/cpp` | `vk::PipelineConstructionUtil` | 管线构建工具 |
| `vkShaderProgram.hpp/cpp` | `vk::ShaderProgram` | 着色器程序管理 |
| `vkShaderToSpirV.hpp/cpp` | `vk::ShaderToSpirV` | GLSL 到 SPIR-V 编译 |
| `vkSpirVAsm.hpp/cpp` | `vk::SpirVAsm` | SPIR-V 汇编工具 |
| `vkPrograms.hpp/cpp` | `vk::Programs` | 着色器程序注册表 |
| `vkBarrierUtil.hpp/cpp` | `vk::BarrierUtil` | 屏障创建工具 |
| `vkCmdUtil.hpp/cpp` | `vk::CmdUtil` | 命令缓冲区工具 |
| `vkObjUtil.hpp/cpp` | `vk::ObjUtil` | Vulkan 对象创建工具 |
| `vkQueryUtil.hpp/cpp` | `vk::QueryUtil` | 查询工具 |
| `vkBuilderUtil.hpp/cpp` | `vk::BuilderUtil` | 构建器工具 |
| `vkAllocationCallbackUtil.hpp/cpp` | `vk::AllocationCallbackUtil` | 分配回调工具 |
| `vkDebugReportUtil.hpp/cpp` | `vk::DebugReportUtil` | 调试报告工具 |
| `vkResourceInterface.hpp/cpp` | `vk::ResourceInterface` | 资源接口（支持多进程） |
| `vkNullDriver.hpp/cpp` | `vk::NullDriver` | 空驱动（用于测试框架本身） |
| `vkStrUtil.hpp/cpp` | `vk::StrUtil` | Vulkan 枚举/函数名字符串工具 |
| `vkTypeUtil.hpp/cpp` | `vk::TypeUtil` | 类型转换工具 |
| `vkApiVersion.hpp/cpp` | `vk::ApiVersion` | API 版本管理 |
| `vkWsiPlatform.hpp/cpp` | `vk::WsiPlatform` | 窗口系统集成平台 |
| `vkWsiUtil.hpp/cpp` | `vk::WsiUtil` | WSI 工具 |
| `vkRayTracingUtil.hpp/cpp` | `vk::RayTracingUtil` | 光线追踪工具 |
| `vkBinaryRegistry.hpp/cpp` | `vk::BinaryRegistry` | 二进制注册表 |
| `vkMd5Sum.hpp/cpp` | `vk::Md5Sum` | MD5 校验和 |
| `vkSafetyCriticalUtil.hpp/cpp` | `vk::SafetyCriticalUtil` | 安全关键工具 |
| `vkShaderObjectUtil.hpp/cpp` | `vk::ShaderObjectUtil` | 着色器对象工具 |
| `vkPipelineBinaryUtil.hpp/cpp` | `vk::PipelineBinaryUtil` | 管线二进制工具 |
| `vkYCbCrImageWithMemory.hpp/cpp` | `vk::YCbCrImageWithMemory` | YCbCr 图像工具 |
| `generated/vulkan/` | 自动生成的代码 | Vulkan API 类型、函数指针、扩展等 |

### 6.2 Vulkan 测试模块层

路径：`external/vulkancts/modules/vulkan/`，前缀 `vkt`

Vulkan 测试按功能领域组织为独立子目录：

| 子目录 | 说明 |
|--------|------|
| `api/` | API 基础功能测试（缓冲区、图像、命令、描述符、管线等） |
| `binding_model/` | 绑定模型测试（描述符集、推送常量、动态偏移等） |
| `clipping/` | 裁剪测试 |
| `draw/` | 绘制测试（顶点、索引、间接、实例化等） |
| `dynamic_state/` | 动态状态测试 |
| `fragment_ops/` | 片段操作测试（深度/模板/裁剪） |
| `fragment_shading_rate/` | 片段着色率测试 |
| `fragment_shader_interlock/` | 片段着色器互锁测试 |
| `fragment_shading_barycentric/` | 重心坐标测试 |
| `image/` | 图像测试（格式、布局、原子操作等） |
| `image_processing/` | 图像处理测试 |
| `memory_model/` | 内存模型测试 |
| `mesh_shader/` | 网格着色器测试 |
| `multiview/` | 多视图测试 |
| `postmortem/` | 故障后处理测试（设备丢失、着色器超时等） |
| `query_pool/` | 查询池测试 |
| `ray_tracing/` | 光线追踪测试（加速结构、内建函数等） |
| `amber/` | Amber 测试框架集成 |

---

## 7. OpenGL CTS（external/openglcts）

### 7.1 通用测试模块

路径：`external/openglcts/modules/common/`，前缀 `glc`

提供所有 OpenGL / OpenGL ES 版本共享的测试用例和工具。

| 文件 | 说明 |
|------|------|
| `glcTestPackage.hpp/cpp` | 通用测试包基类 |
| `glcTestCase.hpp/cpp` | 通用测试用例基类 |
| `glcTestCaseWrapper.hpp/cpp` | 测试用例包装器 |
| `glcContext.hpp/cpp` | 通用上下文 |
| `glcShaderLibrary.hpp/cpp` | 着色器库 |
| `glcShaderLibraryCase.hpp/cpp` | 着色器库测试用例 |
| `glcInfoTests.hpp/cpp` | 信息查询测试 |
| `glcConfigPackage.hpp/cpp` | 配置包 |
| `glcConfigList.hpp/cpp` | 配置列表 |
| `glcCompressedFormatTests.hpp/cpp` | 压缩格式测试 |
| `glcFramebufferBlitTests.hpp/cpp` | 帧缓冲区 Blit 测试 |
| `glcShaderLoopTests.hpp/cpp` | 着色器循环测试 |
| `glcShaderFunctionTests.hpp/cpp` | 着色器函数测试 |
| `glcUniformBlockTests.hpp/cpp` | Uniform 块测试 |
| `glcRobustnessTests.hpp/cpp` | 鲁棒性测试 |
| `glcKHRDebugTests.hpp/cpp` | KHR_debug 测试 |
| `glcNoErrorTests.hpp/cpp` | 无错误上下文测试 |
| `glcSampleVariablesTests.hpp/cpp` | 采样变量测试 |
| `glcTextureStorageTests.hpp/cpp` | 纹理存储测试 |
| `meshShader/` | 网格着色器测试子模块 |
| `subgroups/` | 子组操作测试子模块 |

### 7.2 OpenGL 桌面测试模块

路径：`external/openglcts/modules/gl/`

| 文件 | 说明 |
|------|------|
| `gl3cTestPackages.hpp/cpp` | OpenGL 3.x 测试包 |
| `gl4cTestPackages.hpp/cpp` | OpenGL 4.x 测试包 |
| `gl4cComputeShaderTests.hpp/cpp` | 计算着色器测试 |
| `gl4cDirectStateAccessTests.hpp/cpp` | 直接状态访问测试 |
| `gl4cEnhancedLayoutsTests.hpp/cpp` | 增强布局测试 |
| `gl4cSparseTextureTests.hpp/cpp` | 稀疏纹理测试 |
| `gl4cShaderAtomicCountersTests.hpp/cpp` | 原子计数器测试 |
| `gl4cShaderStorageBufferObjectTests.hpp/cpp` | SSBO 测试 |
| `gl4cProgramInterfaceQueryTests.hpp/cpp` | 程序接口查询测试 |
| `gl4cCopyImageTests.hpp/cpp` | 图像复制测试 |
| `gl4cTextureViewTests.hpp/cpp` | 纹理视图测试 |
| `gl4cBufferStorageTests.hpp/cpp` | 缓冲区存储测试 |
| `gl4cMultiBindTests.hpp/cpp` | 多重绑定测试 |
| `gl4cClipControlTests.hpp/cpp` | 裁剪控制测试 |
| `gl4cConditionalRenderInvertedTests.hpp/cpp` | 条件渲染测试 |
| `gl4cSpirvExtensionsTests.hpp/cpp` | SPIR-V 扩展测试 |

### 7.3 OpenGL ES 测试模块

| 路径 | 前缀 | 说明 |
|------|------|------|
| `modules/gles2/` | `es2c` | OpenGL ES 2.0 CTS 测试 |
| `modules/gles3/` | `es3c` | OpenGL ES 3.0 CTS 测试 |
| `modules/gles31/` | `es31c` | OpenGL ES 3.1 CTS 测试 |
| `modules/gles32/` | `es32c` | OpenGL ES 3.2 CTS 测试 |

### 7.4 OpenGL ES 扩展测试模块

路径：`external/openglcts/modules/glesext/`，前缀 `esextc`

| 子目录 | 说明 |
|--------|------|
| `geometry_shader/` | 几何着色器扩展测试 |
| `gpu_shader5/` | GPU Shader 5 扩展测试 |
| `draw_buffers_indexed/` | 索引化绘制缓冲区扩展测试 |
| `draw_elements_base_vertex/` | Base Vertex 绘制扩展测试 |
| `disjoint_timer_query/` | 离散计时器查询扩展测试 |
| `fragment_shading_rate/` | 片段着色率扩展测试 |

---

## 8. 执行器与执行服务器

### 8.1 Executor 执行器框架

路径：`executor/`，前缀 `xe`

提供批量测试执行、远程通信和结果收集功能。

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `xeBatchExecutor.hpp/cpp` | `xe::BatchExecutor` | 批量执行器，协调测试集执行 |
| `xeBatchResult.hpp/cpp` | `xe::BatchResult` | 批量测试结果管理 |
| `xeCommLink.hpp/cpp` | `xe::CommLink` | 通信链路抽象接口 |
| `xeTcpIpLink.hpp/cpp` | `xe::TcpIpLink` | TCP/IP 通信链路实现 |
| `xeLocalTcpIpLink.hpp/cpp` | `xe::LocalTcpIpLink` | 本地 TCP/IP 通信链路 |
| `xeCallQueue.hpp/cpp` | `xe::CallQueue` | 线程安全调用队列 |
| `xeTestCase.hpp/cpp` | `xe::TestCase` | 测试用例表示 |
| `xeTestCaseListParser.hpp/cpp` | `xe::TestCaseListParser` | 测试用例列表解析 |
| `xeTestCaseResult.hpp/cpp` | `xe::TestCaseResult` | 测试结果表示 |
| `xeTestLogParser.hpp/cpp` | `xe::TestLogParser` | 测试日志解析 |
| `xeTestLogWriter.hpp/cpp` | `xe::TestLogWriter` | 测试日志写入 |
| `xeTestResultParser.hpp/cpp` | `xe::TestResultParser` | 测试结果解析 |
| `xeContainerFormatParser.hpp/cpp` | `xe::ContainerFormatParser` | 容器格式解析 |
| `xeXMLWriter.hpp/cpp` | `xe::XMLWriter` | XML 写入器 |

**Executor 工具**（`executor/tools/`）：

| 文件 | 说明 |
|------|------|
| `xeBatchResultToJUnit.cpp` | 批量结果转 JUnit XML |
| `xeBatchResultToXml.cpp` | 批量结果转 XML |
| `xeCommandLineExecutor.cpp` | 命令行执行器 |
| `xeExtractSampleLists.cpp` | 提取采样列表 |
| `xeExtractShaderPrograms.cpp` | 提取着色器程序 |
| `xeExtractValues.cpp` | 提取测试值 |
| `xeMergeTestLogs.cpp` | 合并测试日志 |
| `xeTestLogCompare.cpp` | 测试日志比较 |

### 8.2 ExecServer 执行服务器

路径：`execserver/`，前缀 `xs`

在目标设备上运行的测试执行服务器，接收 Executor 的命令并执行测试。

| 文件 | 关键类/函数 | 说明 |
|------|------------|------|
| `xsExecutionServer.hpp/cpp` | `xs::ExecutionServer` | 执行服务器主类 |
| `xsTcpServer.hpp/cpp` | `xs::TcpServer` | TCP 服务器 |
| `xsTestDriver.hpp/cpp` | `xs::TestDriver` | 测试驱动器 |
| `xsTestProcess.hpp/cpp` | `xs::TestProcess` | 测试进程抽象 |
| `xsPosixTestProcess.hpp/cpp` | `xs::PosixTestProcess` | POSIX 测试进程实现 |
| `xsWin32TestProcess.hpp/cpp` | `xs::Win32TestProcess` | Win32 测试进程实现 |
| `xsPosixFileReader.hpp/cpp` | `xs::PosixFileReader` | POSIX 文件读取器 |
| `xsProtocol.hpp/cpp` | `xs::Protocol` | 通信协议定义 |

**通信协议**：Executor 与 ExecServer 之间通过自定义 TCP 协议通信，支持：
- 测试用例列表请求
- 测试执行请求
- 测试结果和日志传输
- 心跳和状态查询

---

## 9. 外部依赖

| 依赖 | 路径 | 用途 |
|------|------|------|
| **glslang** | `external/glslang/` | GLSL/HLSL 到 SPIR-V 编译器前端 |
| **SPIR-V Tools** | `external/spirv-tools/` | SPIR-V 着色器优化和验证 |
| **SPIR-V Headers** | `external/spirv-headers/` | SPIR-V 规范头文件 |
| **libpng** | `external/libpng/` | PNG 图像读写（测试结果截图） |
| **zlib** | `external/zlib/` | 数据压缩（libpng 依赖） |
| **Amber** | `external/amber/` | 图形测试描述和执行框架 |
| **jsoncpp** | `external/jsoncpp/` | JSON 解析（部分测试配置） |
| **Vulkan Validation Layers** | `external/vulkan-validationlayers/` | Vulkan 验证层（仅 64 位） |
| **Vulkan SC PC Util** | `external/vulkansc-pcutil/` | Vulkan SC 管线缓存工具 |
| **Vulkan Video Samples** | `external/vulkan-video-samples/` | Vulkan 视频解码/编码测试数据 |
| **RenderDoc** | `external/renderdoc/` | RenderDoc 调试集成（可选） |

外部源码获取脚本：

| 脚本 | 说明 |
|------|------|
| `external/fetch_sources.py` | 获取 glslang、SPIR-V Tools 等外部源码 |
| `external/fetch_video_decode_samples.py` | 获取视频解码测试数据 |
| `external/fetch_video_encode_samples.py` | 获取视频编码测试数据 |
| `external/fetch_optional_video_encode_samples.py` | 获取可选视频编码测试数据 |

---

## 10. 构建系统与项目运行方式

### 10.1 构建前置条件

- **CMake** ≥ 3.20.0
- **Python** 3（用于代码生成和构建脚本）
- **C++ 编译器**：支持 C++11/14（MSVC/GCC/Clang）
- **Git**（版本信息生成）

### 10.2 获取外部源码

```bash
cd external
python3 fetch_sources.py
```

### 10.3 构建步骤

```bash
# 创建构建目录
mkdir build && cd build

# 配置（默认目标平台）
cmake ..

# 配置（指定目标平台，如 Android）
cmake .. -DDEQP_TARGET=android -DANDROID_NDK_PATH=/path/to/ndk

# 配置（选择特定构建目标）
cmake .. -DSELECTED_BUILD_TARGETS="deqp-vk deqp-vksc"

# 构建
cmake --build . --parallel

# 仅构建 Vulkan CTS
cmake --build . --target deqp-vk

# 仅构建 OpenGL ES CTS
cmake --build . --target deqp-gles2
cmake --build . --target deqp-gles3
cmake --build . --target deqp-gles31
```

### 10.4 主要构建目标

| 目标 | 说明 |
|------|------|
| `deqp-vk` | Vulkan 一致性测试 |
| `deqp-vksc` | Vulkan SC 一致性测试 |
| `deqp-gles2` | OpenGL ES 2.0 测试 |
| `deqp-gles3` | OpenGL ES 3.0 测试 |
| `deqp-gles31` | OpenGL ES 3.1 测试 |
| `deqp-egl` | EGL 测试 |
| `deqp-internal` | 内部框架测试 |
| `glcts` | OpenGL/OpenGL ES CTS |

### 10.5 运行测试

```bash
# 运行所有 Vulkan 测试
./deqp-vk

# 运行指定测试用例
./deqp-vk --deqp-case=dEQP-VK.api.smoke

# 列出所有测试用例
./deqp-vk --deqp-caselist=help

# 指定日志文件
./deqp-vk --deqp-log-filename=test.qpa

# 运行 OpenGL ES 3.0 测试
./deqp-gles3

# 运行 OpenGL CTS
./glcts
```

### 10.6 CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `DEQP_TARGET` | `default` | 目标平台 |
| `SELECTED_BUILD_TARGETS` | `""` | 选择性构建目标列表 |
| `GLES_ALLOW_DIRECT_LINK` | `OFF` | 允许直接链接 GLES 库 |
| `DEQP_ANDROID_EXE` | `OFF` | Android 上构建为可执行文件 |
| `DEQP_ANDROID_EXE_LOGCAT` | `OFF` | Android 日志输出到 logcat |
| `DEQP_LOG_NODE_SOURCE` | `OFF` | 记录节点源码位置 |
| `DEQP_DISABLE_VK_VIDEO_TESTS` | `OFF` | 禁用 Vulkan 视频测试 |

### 10.7 Android 构建

```bash
python scripts/android/build_apk.py \
    --sdk /path/to/android-sdk \
    --ndk /path/to/android-ndk \
    --abis arm64-v8a
```

---

## 11. 模块间依赖关系

### 11.1 库依赖图

```
deqp-vk (可执行文件)
  ├── deqp-vk-package (静态库)
  │   ├── vktTestCase (Vulkan 测试用例基类)
  │   ├── vkutil (Vulkan 框架)
  │   │   ├── tcutil (通用测试框架)
  │   │   │   ├── qphelper (C 辅助库)
  │   │   │   ├── decpp (C++ 基础库)
  │   │   │   │   └── debase (C 基础库)
  │   │   │   ├── depool (内存池)
  │   │   │   ├── dethread (线程)
  │   │   │   ├── deutil (实用工具)
  │   │   │   └── destream (流操作)
  │   │   └── glslang / SPIR-V Tools
  │   └── glutil (OpenGL 工具)
  │       └── eglutil (EGL 工具)
  └── tcutil-platform (平台库)
      └── 平台特定实现

deqp-gles3 (可执行文件)
  ├── deqp-gles3-package (静态库)
  │   ├── tes3TestCase (GLES3 测试用例)
  │   ├── glsShared (GL 共享测试)
  │   ├── glutil (OpenGL 工具)
  │   ├── eglutil (EGL 工具)
  │   └── tcutil (通用测试框架)
  └── tcutil-platform (平台库)

deqp-egl (可执行文件)
  ├── deqp-egl-package (静态库)
  │   ├── teglTestCase (EGL 测试用例)
  │   ├── eglutil (EGL 工具)
  │   ├── glutil (OpenGL 工具)
  │   └── tcutil (通用测试框架)
  └── tcutil-platform (平台库)
```

### 11.2 模块包含路径

```
framework/delibs/debase
framework/delibs/decpp
framework/delibs/depool
framework/delibs/dethread
framework/delibs/deutil
framework/delibs/destream
framework/common
framework/qphelper
framework/opengl
framework/opengl/wrapper
framework/referencerenderer
framework/opengl/simplereference
framework/randomshaders
framework/egl
framework/egl/wrapper
framework/xexml
external/vulkancts/framework/vulkan
```

---

## 12. 测试执行流程

### 12.1 本地执行流程

```
1. main() (tcuMain.cpp)
   │
   ├── 解析命令行参数 (tcu::CommandLine)
   ├── 创建资源归档 (tcu::DirArchive)
   ├── 创建测试日志 (tcu::TestLog)
   ├── 创建平台对象 (createPlatform())
   └── 创建应用对象 (tcu::App)
       │
       ├── 初始化测试包注册表
       ├── 构建测试层次树
       └── 创建 TestSessionExecutor
           │
           └── iterate() 循环
               │
               ├── 选择下一个测试用例
               ├── 执行测试用例
               │   ├── 初始化测试上下文
               │   ├── 调用 TestCase::iterate()
               │   ├── 收集测试结果
               │   └── 写入测试日志
               └── 返回是否继续
```

### 12.2 远程执行流程

```
Executor (主机端)                    ExecServer (目标设备端)
    │                                       │
    ├── 连接到 ExecServer ─────────────────►│
    │                                       ├── 接受连接
    ├── 请求测试用例列表 ──────────────────►│
    │                                       ├── 枚举测试用例
    │◄────────────── 返回测试用例列表 ───────┤
    │                                       │
    ├── 请求执行测试 ──────────────────────►│
    │                                       ├── 启动测试进程
    │                                       ├── 执行测试
    │◄────────────── 返回测试结果/日志 ──────┤
    │                                       │
    ├── 请求执行下一个测试 ────────────────►│
    │   ...                                 │
    └── 断开连接 ─────────────────────────►│
                                            └── 清理资源
```

### 12.3 测试用例生命周期

```
1. TestPackage::init()          — 初始化测试包
2. TestCaseGroup::init()        — 构建测试层次结构
3. TestCase::init()             — 初始化测试用例
4. TestCase::iterate()          — 执行测试迭代（可能多次）
5. TestCase::deinit()           — 清理测试用例
6. 结果记录到 TestLog           — Pass/Fail/NotSupported/QualityWarning/etc.
```

---

## 13. 辅助脚本

### 13.1 代码生成脚本

| 脚本 | 说明 |
|------|------|
| `scripts/gen_egl.py` | 生成 EGL 封装代码 |
| `scripts/opengl/gen_all.py` | 生成所有 OpenGL 封装代码 |
| `scripts/opengl/gen_call_log_wrapper.py` | 生成 API 调用日志包装器 |
| `scripts/opengl/gen_str_util.py` | 生成枚举字符串工具 |
| `scripts/opengl/gen_func_ptrs.py` | 生成函数指针 |
| `scripts/opengl/gen_wrapper.py` | 生成 API 包装器 |
| `scripts/opengl/gen_es_static_library.py` | 生成 ES 静态库 |
| `scripts/opengl/gen_query_util.py` | 生成查询工具 |
| `scripts/egl/str_util.py` | EGL 字符串工具生成 |

### 13.2 构建与发布脚本

| 脚本 | 说明 |
|------|------|
| `scripts/make_release.py` | 创建发布包 |
| `scripts/build_caselists.py` | 构建测试用例列表 |
| `scripts/build_android_mustpass.py` | 构建 Android Mustpass 列表 |
| `scripts/ctsbuild/build.py` | CTS 构建入口 |
| `scripts/launchcontrol_build.py` | LaunchControl 构建集成 |
| `scripts/gen_android_bp.py` | 生成 Android.bp 文件 |
| `scripts/gen_khronos_cts_bp.py` | 生成 Khronos CTS Android.bp |

### 13.3 测试与分析脚本

| 脚本 | 说明 |
|------|------|
| `scripts/run_internal_tests.py` | 运行内部测试 |
| `scripts/run_nightly.py` | 运行夜间测试 |
| `scripts/check_build_sanity.py` | 检查构建完整性 |
| `scripts/check_swiftshader_runtime.py` | SwiftShader 运行时检查 |
| `scripts/list_test_changes.py` | 列出测试变更 |
| `scripts/caselist_diff.py` | 测试用例列表差异比较 |
| `scripts/mustpass.py` | Mustpass 列表管理 |
| `scripts/testset.py` | 测试集管理 |

### 13.4 源码检查脚本

| 脚本 | 说明 |
|------|------|
| `scripts/src_util/check_boms.py` | 检查 BOM 标记 |
| `scripts/src_util/check_encoding.py` | 检查文件编码 |
| `scripts/src_util/check_file_size_limit.py` | 检查文件大小限制 |
| `scripts/src_util/check_include_guards.py` | 检查头文件保护宏 |
| `scripts/src_util/check_license.py` | 检查许可证头 |
| `scripts/src_util/check_whitespace.py` | 检查空白字符 |
| `scripts/src_util/pre_commit.py` | 预提交检查 |

### 13.5 日志分析脚本

| 脚本 | 说明 |
|------|------|
| `scripts/log/log_parser.py` | 测试日志解析 |
| `scripts/log/log_to_csv.py` | 日志转 CSV |
| `scripts/log/log_to_xml.py` | 日志转 XML |
| `scripts/log/bottleneck_report.py` | 性能瓶颈报告 |

---

## 附录：命名约定

| 前缀 | 模块 |
|------|------|
| `de` | delibs 基础库（C 接口） |
| `de` | delibs 基础库（C++ 接口） |
| `qp` | qphelper C 辅助库 |
| `tcu` | 通用测试框架（tcutil） |
| `glu` | OpenGL 工具库（glutil） |
| `eglu` | EGL 工具库（eglutil） |
| `rr` | 参考渲染器 |
| `rsg` | 随机着色器生成器 |
| `xe` | Executor 执行器 |
| `xs` | ExecServer 执行服务器 |
| `tegl` | EGL 测试模块 |
| `tes2` | OpenGL ES 2.0 测试模块 |
| `tes3` | OpenGL ES 3.0 测试模块 |
| `tes31` | OpenGL ES 3.1 测试模块 |
| `gls` | GL 共享测试工具 |
| `dit` | 内部测试模块 |
| `vk` | Vulkan 框架 |
| `vkt` | Vulkan 测试模块 |
| `glc` | OpenGL CTS 通用模块 |
| `gl3c` | OpenGL 3.x CTS |
| `gl4c` | OpenGL 4.x CTS |
| `es2c` | OpenGL ES 2.0 CTS |
| `es3c` | OpenGL ES 3.0 CTS |
| `es31c` | OpenGL ES 3.1 CTS |
| `es32c` | OpenGL ES 3.2 CTS |
| `esextc` | OpenGL ES 扩展 CTS |

---

*本文档自动生成于 2026-05-30，基于 VK-GL-CTS 项目源码分析*
