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

**一键编译**：

```cmd
git clone https://github.com/027123/Tron-Address-Hunter.git
cd Tron-Address-Hunter
.\build.bat
```

`build.bat` 会自动查找 Visual Studio 环境并编译生成 `profanity.exe`。

也可以手动编译（在 x64 Native Tools Command Prompt 中）：

```cmd
cl /O2 /EHsc /std:c++17 /I "OpenCL/include" /I "third_party" src\Dispatcher.cpp src\Mode.cpp src\precomp.cpp src\profanity.cpp src\SpeedSample.cpp third_party\uECC.c /link /OUT:profanity.exe "OpenCL/lib/OpenCL.lib" ws2_32.lib advapi32.lib
```

### 编译（Linux/macOS）

```bash
git clone https://github.com/027123/Tron-Address-Hunter.git
cd Tron-Address-Hunter
make
```

### 运行

编译完成后，将 `dist/` 目录下的文件复制到 `profanity.exe` 同级目录。

**方式一：交互式运行（推荐）**

双击 `run.bat`，按提示输入前缀/后缀匹配位数和生成数量，结果自动保存到 `result/` 目录。

**方式二：配置文件运行**

复制 `profanity.conf.example` 为 `profanity.conf`，编辑参数后双击 `profanity.exe` 即可。

**方式三：命令行运行**

```cmd
profanity.exe --matching profanity.txt --suffix-count 6 --quit-count 10 -o result.txt
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
  --skip              跳过指定索引的 GPU 设备（可多次使用）
  --output            结果输出到文件（追加写入，支持 {date} 占位符）
```

## 配置文件

支持通过配置文件设置参数，避免每次输入长命令。复制 `profanity.conf.example` 为 `profanity.conf`：

```ini
# 匹配规则文件
matching=profanity.txt
# 后缀匹配位数
suffix-count=6
# 找到 10 个结果后退出
quit-count=10
# 结果输出文件（{date} 自动替换为启动时间 YYYYMMDD_HHMM）
output=result_{date}.txt
# 跳过指定 GPU（可选，按 GPU 编号指定）
# skip=1
```

`{date}` 占位符会在程序启动时自动替换为当前日期和时间，例如 `result_20260322_1430.txt`。

命令行参数优先于配置文件。

## GPU 多设备支持

程序会自动检测系统中所有 GPU 设备。如果存在来自不同厂商（如 NVIDIA + AMD 核显）的 GPU，程序会自动选择计算能力最强的平台运行，跳过不兼容的设备，并输出提示信息。

如果需要手动控制使用哪个 GPU，可以通过 `--skip` 参数或配置文件中的 `skip` 选项跳过指定编号的 GPU：

```ini
# 跳过 GPU-0（使用核显）
skip=0

# 跳过 GPU-1（使用独显）
skip=1
```

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
