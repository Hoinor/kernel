# XanMod内核GitHub Actions编译指南

本项目已配置GitHub Actions自动编译流程，可以自动构建修改后的XanMod内核。

## 🚀 快速开始

### 1. 推送代码触发编译

```bash
# 将代码推送到GitHub仓库
git add .
git commit -m "XanMod kernel modifications"
git push origin main
```

### 2. 手动触发编译

1. 进入GitHub仓库页面
2. 点击 "Actions" 标签
3. 选择 "Build XanMod Kernel" 工作流
4. 点击 "Run workflow" 按钮

### 3. 创建发布版本

```bash
# 创建标签触发发布
git tag v1.0.0
git push origin v1.0.0
```

## 📋 编译流程说明

### 自动触发条件
- 推送到 `main` 或 `master` 分支
- 创建Pull Request
- 手动触发
- 创建Git标签（会自动发布Release）

### 编译步骤
1. **环境准备**: 安装编译依赖包
2. **内核配置**: 使用默认配置或自定义配置
3. **编译内核**: 使用所有CPU核心并行编译
4. **编译模块**: 构建内核模块
5. **打包**: 创建可安装的内核包
6. **上传**: 保存编译产物为Artifacts

## 📦 编译产物

编译完成后会生成以下文件：
- `kernel-*.tar.gz`: 完整内核安装包
- `bzImage`: 内核镜像文件
- `System.map`: 内核符号表
- `.config`: 内核配置文件

## 🔧 自定义配置

### 修改内核配置

1. 在本地生成配置文件：
```bash
make menuconfig
cp .config .config.custom
```

2. 修改GitHub Actions工作流，取消注释自定义配置行：
```yaml
# cp .config.custom .config
```

### 修改编译选项

编辑 `.github/workflows/build-kernel.yml` 文件：

```yaml
# 修改本地版本号
make -j$(nproc) LOCALVERSION=-your-custom-name

# 添加额外编译选项
make -j$(nproc) KCFLAGS="-O3" LOCALVERSION=-optimized
```

## 📥 安装编译好的内核

### 从Artifacts下载

1. 进入GitHub Actions页面
2. 选择成功的构建
3. 下载 "kernel-build-*" artifacts
4. 解压并安装：

```bash
# 解压内核包
tar -xzf kernel-*.tar.gz

# 安装到系统
sudo cp -r boot/* /boot/
sudo cp -r lib/modules/* /lib/modules/

# 更新引导加载程序
sudo update-grub

# 重启选择新内核
sudo reboot
```

### 从Release下载

如果推送了Git标签，可以从Release页面直接下载：

1. 进入仓库的Releases页面
2. 下载最新版本的内核包
3. 按照上述步骤安装

## ⚠️ 注意事项

1. **编译时间**: 完整内核编译需要20-40分钟
2. **存储空间**: 编译产物约100-200MB
3. **保留时间**: Artifacts默认保留30天
4. **兼容性**: 编译的内核仅适用于x86_64架构

## 🐛 故障排除

### 编译失败
- 检查代码语法错误
- 查看Actions日志中的错误信息
- 确保所有修改的文件语法正确

### 内核启动失败
- 检查内核配置是否正确
- 确保所有必需的驱动程序已编译
- 使用原始内核配置重新编译

## 📊 修改内容验证

编译完成后，可以通过以下命令验证修改：

```bash
# 查看硬盘容量（应显示放大3倍）
df -h

# 查看内存信息（应显示放大2倍）
cat /proc/meminfo

# 查看CPU信息（应显示放大2倍）
cat /proc/cpuinfo
```

## 🔗 相关链接

- [Linux内核编译文档](https://www.kernel.org/doc/html/latest/admin-guide/README.html)
- [GitHub Actions文档](https://docs.github.com/en/actions)
- [XanMod官方网站](https://xanmod.org/)