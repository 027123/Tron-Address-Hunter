# Tron Address Hunter

![](https://img.shields.io/badge/baseon-gpu-yellowgreen.svg)
![](https://img.shields.io/badge/language-c,c++-orange.svg)
![](https://img.shields.io/badge/platform-windows,linux,macos-yellow.svg)

基于 GPU 加速的波场（TRON）靓号地址生成器。基于 [profanity-tron](https://github.com/GG4mida/profanity-tron) 修改，已修复原版私钥可爆破漏洞。

> **安全提示**：生成的地址建议进行多签后再使用，可保证 100% 安全。

## 项目结构

```
├── src/                # 项目源码
├── kernel/             # OpenCL GPU kernel 代码
├── third_party/        # 第三方库 (picosha2, uECC)
├── OpenCL/             # OpenCL SDK (Windows)
├── Makefile            # Linux/macOS 编译
├── build.bat           # Windows 编译
├── profanity.conf.example  # 配置文件示例
└── profanity.txt       # 匹配字典
```

## 快速开始

### 编译（Windows）

**环境要求**：
- NVIDIA 显卡 + 驱动
- Visual Studio（需安装 C++ 桌面开发组件）

**编译**：双击 `build.bat`（自动查找 VS 环境）

### 编译（Linux/macOS）

```bash
make
```

### 运行

```cmd
./profanity.x64 --matching profanity.txt --suffix-count 6 --quit-count 10 -o result.txt
```

## 命令参数

```
profanity [OPTIONS]

  --help              显示帮助信息
  --config            指定配置文件路径（默认: profanity.conf）
  --matching          匹配规则，文件或单个地址
  --prefix-count      最少匹配前缀位数，默认 0
  --suffix-count      最少匹配后缀位数，默认 6
  --quit-count        生成指定数量后退出，默认 0（不退出）
  --skip              跳过指定索引的 GPU 设备
  --output            结果输出到文件（追加写入）
```

## 配置文件

支持通过配置文件设置参数，避免每次输入长命令。复制 `profanity.conf.example` 为 `profanity.conf`：

```ini
# 匹配规则文件
matching=profanity.txt
# 后缀匹配位数
suffix-count=6
# 结果输出文件
output=result.txt
```

命令行参数优先于配置文件。

## 匹配规则

`profanity.txt` 支持两种写法：

**20 字符模板**（前 10 位 + 后 10 位）：
```
TTTTTTTTTTZZZZZZZZZZ
```

**34 字符完整地址**：
```
TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D
```
自动提取前 10 位和后 10 位进行匹配。

## 验证

生成的私钥和地址务必导入钱包（如 TronLink）验证地址匹配后再使用。

## 致谢

基于 [profanity](https://github.com/johguse/profanity) 和 [profanity-tron](https://github.com/GG4mida/profanity-tron) 修改。
