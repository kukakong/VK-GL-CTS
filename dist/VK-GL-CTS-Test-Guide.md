# VK-GL-CTS 测试使用指南

## 概述

本文档提供 VK-GL-CTS (Khronos 图形 API 一致性测试套件) 的详细使用说明，包括各平台测试程序的使用方法、Mali GPU 驱动配置以及测试结果输出配置。

---

## 1. 构建产物清单

### 1.1 Linux x86_64 (64位)

| 文件名 | 说明 |
|--------|------|
| `deqp-egl` | EGL 一致性测试 |
| `deqp-gles2` | OpenGL ES 2.0 一致性测试 |
| `deqp-gles3` | OpenGL ES 3.0 一致性测试 |
| `deqp-gles31` | OpenGL ES 3.1 一致性测试 |
| `glcts` | OpenGL CTS 综合测试 |

### 1.2 Linux x86_32 (32位)

| 文件名 | 说明 |
|--------|------|
| `deqp-egl` | EGL 一致性测试 |
| `deqp-gles2` | OpenGL ES 2.0 一致性测试 |
| `deqp-gles3` | OpenGL ES 3.0 一致性测试 |
| `deqp-gles31` | OpenGL ES 3.1 一致性测试 |
| `glcts` | OpenGL CTS 综合测试 |

---

## 2. Mali GPU 驱动配置

### 2.1 Android 平台

Android 平台上 Mali GPU 库文件通常位于以下路径：

```
# 32位系统
/vendor/lib/egl/libGLES_mali.so

# 64位系统
/vendor/lib64/egl/libGLES_mali.so
```

**配置步骤：**

1. 确认 Mali 库存在：
```bash
adb shell ls -la /vendor/lib*/egl/libGLES_mali.so
```

2. 如需使用特定库，设置环境变量：
```bash
adb shell setenv LD_LIBRARY_PATH /vendor/lib64/egl:$LD_LIBRARY_PATH
```

### 2.2 Linux 平台

Linux 平台上 Mali GPU 库文件可能位于以下路径：

```
# 常见路径
/lib/libmali.so
/lib64/libmali.so
/lib/libMali.so
/lib64/libMali.so

# 系统路径
/usr/lib/libmali.so
/usr/lib64/libmali.so
```

**配置步骤：**

1. 查找 Mali 库：
```bash
find /lib /usr -name "libmali.so" -o -name "libMali.so" 2>/dev/null
```

2. 设置库路径（假设库在 `/lib64/libmali.so`）：
```bash
export LD_LIBRARY_PATH=/lib64:$LD_LIBRARY_PATH
```

3. 对于 Vulkan 测试，配置 ICD 文件：
```bash
mkdir -p /etc/vulkan/icd.d
echo '{"icd":{"library_path":"/lib64/libmali.so","api_version":"1.2"}}' > /etc/vulkan/icd.d/mali.json
export VK_ICD_FILENAMES=/etc/vulkan/icd.d/mali.json
```

---

## 3. 测试执行方法

### 3.1 基本命令格式

```bash
./deqp-<target> [options]
```

### 3.2 常用命令行选项

| 选项 | 说明 |
|------|------|
| `--deqp-case=<pattern>` | 指定测试用例过滤模式 |
| `--deqp-caselist-file=<file>` | 从文件读取测试用例列表 |
| `--deqp-log-filename=<file>` | 指定日志输出文件 |
| `--deqp-log-format=<format>` | 日志格式 (plain, qpa, xml) |
| `--deqp-stdin-caselist` | 从标准输入读取测试用例 |
| `--deqp-watchdog` | 启用看门狗超时 |
| `--deqp-crash-handler` | 启用崩溃处理 |
| `--deqp-visibility=hidden` | 隐藏窗口运行 |

### 3.3 Linux 平台测试示例

```bash
# 进入测试目录
cd /path/to/dist/linux-x86_64

# 运行所有 EGL 测试并输出到文件
./deqp-egl --deqp-log-filename=egl_test_result.qpa

# 运行特定测试用例
./deqp-gles2 --deqp-case="dEQP-GLES2.functional.*" --deqp-log-filename=gles2_functional.qpa

# 从测试列表文件运行
./deqp-gles3 --deqp-caselist-file=testcases.txt --deqp-log-filename=gles3_result.qpa

# 使用 XML 格式输出
./deqp-gles31 --deqp-log-format=xml --deqp-log-filename=gles31_result.xml
```

### 3.4 Android 平台测试示例

```bash
# 推送测试程序到设备
adb push deqp-gles2 /data/local/tmp/
adb shell chmod +x /data/local/tmp/deqp-gles2

# 运行测试
adb shell "cd /data/local/tmp && ./deqp-gles2 --deqp-log-filename=/sdcard/gles2_result.qpa"

# 拉取测试结果
adb pull /sdcard/gles2_result.qpa ./
```

---

## 4. 测试结果输出配置

### 4.1 输出格式

| 格式 | 说明 | 文件扩展名 |
|------|------|------------|
| `qpa` | QPA 格式 (默认，详细日志) | .qpa |
| `xml` | XML 格式 | .xml |
| `plain` | 纯文本格式 | .txt |

### 4.2 输出到文件

```bash
# 方式1：使用命令行参数
./deqp-gles2 --deqp-log-filename=result.qpa

# 方式2：重定向标准输出
./deqp-gles2 2>&1 | tee test_output.log

# 方式3：指定输出目录
./deqp-gles2 --deqp-log-filename=/var/log/cts/gles2_$(date +%Y%m%d).qpa
```

### 4.3 结果文件解析

测试结果包含以下关键信息：

- **Pass**: 测试通过
- **Fail**: 测试失败
- **NotSupported**: 不支持该功能
- **CompatibilityWarning**: 兼容性警告
- **QualityWarning**: 质量警告
- **InternalError**: 内部错误

---

## 5. 高级配置

### 5.1 Vulkan ICD 配置

创建 Vulkan ICD 配置文件：

```json
{
    "icd": {
        "library_path": "/lib64/libmali.so",
        "api_version": "1.2"
    }
}
```

保存为 `/etc/vulkan/icd.d/mali.json`

### 5.2 Headless 运行配置

对于无显示环境：

```bash
./deqp-gles2 --deqp-surface-type=pbuffer --deqp-visibility=hidden
```

---

## 6. 测试用例筛选

### 6.1 通配符模式

```bash
# 运行所有 API 测试
./deqp-gles3 --deqp-case="dEQP-GLES3.api.*"

# 运行特定功能组
./deqp-gles3 --deqp-case="dEQP-GLES3.functional.texture.*"
```

### 6.2 测试列表文件

创建测试列表文件 `testcases.txt`：

```
dEQP-GLES2.functional.api.clears
dEQP-GLES2.functional.api.draw
dEQP-GLES2.functional.api.state_query
```

运行：

```bash
./deqp-gles2 --deqp-caselist-file=testcases.txt
```

---

## 7. 故障排查

### 7.1 常见问题

**问题：找不到 GPU 驱动库**

```bash
# 检查库是否存在
ls -la /lib*/libmali.so /vendor/lib*/egl/libGLES_mali.so

# 设置库路径
export LD_LIBRARY_PATH=/lib64:/vendor/lib64/egl:$LD_LIBRARY_PATH
```

**问题：测试崩溃**

```bash
# 启用崩溃处理
./deqp-gles2 --deqp-crash-handler --deqp-watchdog=enable

# 检查系统日志
dmesg | tail -50
```

---

## 8. 完整测试流程示例

```bash
#!/bin/bash

# 设置环境
export LD_LIBRARY_PATH=/lib64:$LD_LIBRARY_PATH

# 创建结果目录
RESULT_DIR="cts_results_$(date +%Y%m%d_%H%M%S)"
mkdir -p $RESULT_DIR

# 运行测试
./linux-x86_64/deqp-egl --deqp-log-filename=$RESULT_DIR/egl.qpa --deqp-watchdog
./linux-x86_64/deqp-gles2 --deqp-case="dEQP-GLES2.functional.*" --deqp-log-filename=$RESULT_DIR/gles2.qpa
./linux-x86_64/deqp-gles3 --deqp-case="dEQP-GLES3.functional.*" --deqp-log-filename=$RESULT_DIR/gles3.qpa
./linux-x86_64/deqp-gles31 --deqp-case="dEQP-GLES31.functional.*" --deqp-log-filename=$RESULT_DIR/gles31.qpa

echo "测试完成，结果保存在 $RESULT_DIR"
```

---

## 9. 附录

### 9.1 测试用例命名规则

- `dEQP-EGL.*` - EGL 测试
- `dEQP-GLES2.*` - OpenGL ES 2.0 测试
- `dEQP-GLES3.*` - OpenGL ES 3.0 测试
- `dEQP-GLES31.*` - OpenGL ES 3.1 测试
- `dEQP-VK.*` - Vulkan 测试

### 9.2 参考链接

- [Khronos CTS 官方文档](https://github.com/KhronosGroup/VK-GL-CTS)
- [Vulkan 规范](https://www.khronos.org/registry/vulkan/)
- [OpenGL ES 规范](https://www.khronos.org/opengles/)

---

*文档生成时间: 2026-06-03*
