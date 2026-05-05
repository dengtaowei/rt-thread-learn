# RT-Thread QEMU + GDB + VS Code 调试（qemu-vexpress-a9）

## 1. 依赖安装

```bash
pip install scons
sudo apt update
sudo apt install qemu-system-arm
```

要求系统里可用以下命令：

- `scons`
- `arm-none-eabi-gcc`
- `arm-none-eabi-gdb`
- `qemu-system-arm`

## 2. 一键启动（推荐）

在工程根目录执行：

```bash
./scripts/dev-qemu-vexpress-a9.sh
```

这个脚本会先编译 `bsp/qemu-vexpress-a9`，然后启动 QEMU 并在 `1234` 端口等待 GDB 连接。

如果你只想启动 QEMU（不重新编译）：

```bash
./scripts/dev-qemu-vexpress-a9.sh --skip-build
```

## 3. VS Code 调试

已生成配置文件：

- `.vscode/tasks.json`
- `.vscode/launch.json`

可用任务：

- `RT-Thread: Build qemu-vexpress-a9`
- `RT-Thread: Run QEMU (gdb wait)`
- `RT-Thread: Build + Run QEMU (one-click)`

调试配置：

- `RT-Thread Kernel Startup (from _reset)`

使用步骤：

1. 运行 one-click 任务或脚本，让 QEMU 停在 GDB 等待状态。
2. 在 VS Code 中按 `F5` 启动 `RT-Thread Kernel Startup (from _reset)`。
3. 在源码打断点，进行单步、变量查看和线程切换调试。

## 4. 常见问题

- `qemu-system-arm: command not found`
  - 未安装 QEMU，执行依赖安装命令。
- `scons: command not found`
  - 未安装 SCons，执行 `pip install scons`。
- 断点不命中
  - 先确认 `bsp/qemu-vexpress-a9/rtthread.elf` 与 `rtthread.bin` 是同一次构建产物；
  - 再确认 QEMU 是通过脚本以 `-S -gdb tcp::1234` 启动的。

## 5. 提交到 Git 前清理

为便于项目迁移到其他机器重新编译，提交前建议先清理本地构建和运行产物：

```bash
./scripts/clean-qemu-vexpress-a9.sh
```

该脚本会删除 `build/`、`rtthread.elf/bin/map`、`sd.bin` 等可再生文件，不会删除源码和调试配置。

## 6. 新机器快速搭建

在新机器上建议按以下顺序执行：

1. 安装工具链：`arm-none-eabi-gcc`、`arm-none-eabi-gdb`、`qemu-system-arm`、`scons`。
2. 克隆仓库并进入根目录。
3. 执行 `./scripts/dev-qemu-vexpress-a9.sh` 完成编译并启动 QEMU gdb wait。
4. 在 VS Code 按 `F5` 启动 `RT-Thread Kernel Startup (from _reset)`。
