# Tron Address Hunter

![](https://img.shields.io/badge/baseon-gpu-yellowgreen.svg)
![](https://img.shields.io/badge/language-c,c++-orange.svg)
![](https://img.shields.io/badge/platform-windows-yellow.svg)

基于 GPU 加速的波场（TRON）靓号地址生成器。基于 [profanity-tron](https://github.com/GG4mida/profanity-tron) 修改，已修复原版私钥可爆破漏洞。

> **安全提示**：生成的地址建议进行多签后再使用，可保证 100% 安全。

## 快速开始

### 直接运行

将以下文件放在同一目录，双击 `run.bat` 即可：

```
├── profanity.exe      # 编译后的可执行程序
├── profanity.txt      # 匹配字典
└── run.bat            # 启动脚本（交互式）
```

`run.bat` 启动后会提示输入：
- **前缀匹配位数**（默认 0，不匹配）
- **后缀匹配位数**（默认 6，可选 4-12）
- **生成地址数量**（默认 1，可选 1-10000）

结果自动保存到 `result/result-YYYYMMDD-HHMMSS.txt`，格式为 `privatekey,address`，每找到一个地址实时写入。

### 编译（Windows）

**环境要求**：
- NVIDIA 显卡 + 驱动
- Visual Studio 2019/2022（需安装 C++ 桌面开发组件）

**编译方式一**：双击 `build.bat`（自动查找 VS 环境）

**编译方式二**：打开 "x64 Native Tools Command Prompt"，手动执行：

```cmd
cl /O2 /EHsc /std:c++17 /DNO_CURL /I "OpenCL/include" Dispatcher.cpp Mode.cpp precomp.cpp profanity.cpp SpeedSample.cpp /link /OUT:profanity.exe "OpenCL/lib/OpenCL.lib" ws2_32.lib advapi32.lib
```

## 命令参数

```
profanity.exe [OPTIONS]

  --matching        匹配规则，文件或单个地址
  --prefix-count    最少匹配前缀位数，默认 0
  --suffix-count    最少匹配后缀位数，默认 6
  --quit-count      生成指定数量后退出，默认 0（不退出）
  --skip            跳过指定索引的 GPU 设备（集成显卡报错时使用）
  --output          结果输出到文件（追加写入）
  --post            结果发送到指定 URL
```

**示例**：

```cmd
profanity.exe --matching profanity.txt --suffix-count 6 --quit-count 10 --skip 1 -o result.txt
```

## 匹配规则

`profanity.txt` 支持两种写法：

**20 字符模板**（前 10 位 + 后 10 位）：
```
TTTTTTTTTTZZZZZZZZZZ
```
匹配以 Z 结尾的地址。前缀部分 T 为通配符，后缀从末尾开始匹配。

**34 字符完整地址**：
```
TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D
```
自动提取前 10 位和后 10 位进行匹配。

## 验证

生成的私钥和地址务必验证匹配：[https://secretscan.org/PrivateKeyTron](https://secretscan.org/PrivateKeyTron)

## 致谢

基于 [profanity](https://github.com/johguse/profanity) 和 [profanity-tron](https://github.com/GG4mida/profanity-tron) 修改。
