# VK-GL-CTS 编译报告

## 编译环境

- **日期**: 2026-06-04
- **操作系统**: Linux
- **编译器**: Clang 18.0.3 (Android NDK r27c)
- **目标平台**: Android ARM64, Android ARM32

## 编译产物

### Android ARM64
| 文件 | 大小 | 描述 |
|------|------|------|
| android-arm64-vk.tar.gz | 27MB | Vulkan CTS 测试程序 (deqp-vk) |
| android-arm64-gl.tar.gz | 14MB | OpenGL CTS 测试程序 (glcts) |

### Android ARM32
| 文件 | 大小 | 描述 |
|------|------|------|
| android-arm32-vk.tar.gz | 25MB | Vulkan CTS 测试程序 (deqp-vk) |
| android-arm32-gl.tar.gz | 13MB | OpenGL CTS 测试程序 (glcts) |

## 编译配置

### CMake 参数
```
-DCMAKE_TOOLCHAIN_FILE=/workspace/toolchain-android-arm64.cmake
-DDEQP_TARGET=null
-DCMAKE_BUILD_TYPE=Release
-DDEQP_DISABLE_VKSC=ON
-DDEQP_ANDROID_EXE=ON
```

### 工具链文件配置
- **Android NDK**: r27c
- **API Level**: 28
- **STL**: c++_static
- **DE_OS**: DE_OS_ANDROID
- **DE_CPU**: DE_CPU_ARM_64 / DE_CPU_ARM

## 问题修复记录

### 1. DE_OS 检测问题

**问题描述**: 
自定义工具链文件没有设置 `DE_OS` 为 `DE_OS_ANDROID`，导致代码走到 UNIX 分支，使用了 Android NDK 不支持的 `__assert_fail` 函数。

**错误信息**:
```
/workspace/framework/delibs/debase/deDefs.c:132:5: error: call to undeclared function '__assert_fail'
```

**修复方案**:
在工具链文件中添加：
```cmake
set(DE_OS "DE_OS_ANDROID")
set(DE_CPU "DE_CPU_ARM_64")
set(DE_COMPILER "DE_COMPILER_CLANG")
```

### 2. VulkanSC 编译依赖问题

**问题描述**:
vulkansc-pcutil 的 ExternalProject 配置没有正确传递 Android 工具链信息，导致编译失败。

**修复方案**:
添加 `DEQP_DISABLE_VKSC` 选项，禁用 VulkanSC 相关组件：
```cmake
option(DEQP_DISABLE_VKSC "Disable Vulkan SC tests and related components" OFF)
```

### 3. DE_PLATFORM_USE_LIBRARY_TYPE 宏问题

**问题描述**:
`tcu::null::Platform` 类的 `createLibrary` 方法签名与 `vk::Platform` 基类不匹配。

**错误信息**:
```
error: allocating an object of abstract class type 'tcu::null::Platform'
note: unimplemented pure virtual method 'createLibrary' in 'Platform'
```

**修复方案**:
在 `tcuNullPlatform.hpp` 和 `tcuNullPlatform.cpp` 中添加条件编译：
```cpp
#ifdef DE_PLATFORM_USE_LIBRARY_TYPE
    virtual vk::Library *createLibrary(vk::Platform::LibraryType libraryType = vk::Platform::LIBRARY_TYPE_VULKAN,
                                       const char *libraryPath = nullptr) const;
#else
    virtual vk::Library *createLibrary(const char *libraryPath = nullptr) const;
#endif
```

### 4. MFD_CLOEXEC 未定义问题

**问题描述**:
Android NDK 不定义 `MFD_CLOEXEC` 宏。

**修复方案**:
在 `vktMemoryMapPlacedTests.cpp` 中添加：
```cpp
#if defined(DE_OS_ANDROID) && !defined(MFD_CLOEXEC)
#define MFD_CLOEXEC 0x0001U
#endif
```

### 5. memfd_create 不可用问题

**问题描述**:
Android 不支持 `memfd_create` 系统调用。

**修复方案**:
在 Android 上使用 ashmem 替代：
```cpp
#if defined(DE_OS_ANDROID)
        int memfd = open("/dev/ashmem", O_RDWR);
        if (memfd < 0)
            TCU_THROW(NotSupportedError, "ashmem open failed - memfd_create not available on Android");
#else
        int memfd = memfd_create("mapplaced-test", MFD_CLOEXEC);
#endif
```

### 6. Android APP vs EXE 编译模式问题

**问题描述**:
默认编译模式为 Android APP，需要 JNI 相关文件，但缺少 `xsExecutionServer.hpp`。

**修复方案**:
使用 `DEQP_ANDROID_EXE=ON` 编译独立可执行文件。

## 使用说明

### 解压测试程序
```bash
# Android ARM64
tar -xzf android-arm64-vk.tar.gz
tar -xzf android-arm64-gl.tar.gz

# Android ARM32
tar -xzf android-arm32-vk.tar.gz
tar -xzf android-arm32-gl.tar.gz
```

### 推送到 Android 设备
```bash
adb push deqp-vk /data/local/tmp/
adb shell chmod +x /data/local/tmp/deqp-vk
adb shell /data/local/tmp/deqp-vk
```

## 文件验证

### Android ARM64 可执行文件
```
deqp-vk: ELF 64-bit LSB pie executable, ARM aarch64, dynamically linked, interpreter /system/bin/linker64
glcts: ELF 64-bit LSB pie executable, ARM aarch64, dynamically linked, interpreter /system/bin/linker64
```

### Android ARM32 可执行文件
```
deqp-vk: ELF 32-bit LSB pie executable, ARM, EABI5, dynamically linked, interpreter /system/bin/linker
glcts: ELF 32-bit LSB pie executable, ARM, EABI5, dynamically linked, interpreter /system/bin/linker
```
