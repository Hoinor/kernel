# XanMod内核快速安装指南

## 🚀 一键自动安装

### 方法1：从GitHub Actions下载

1. **下载编译好的内核包**
   - 进入GitHub仓库的Actions页面
   - 选择最新的成功构建
   - 下载 `kernel-build-*` artifacts

2. **解压并安装**
   ```bash
   # 解压下载的文件
   unzip kernel-build-*.zip
   tar -xzf kernel-*.tar.gz
   
   # 进入解压目录
   cd kernel-*
   
   # 一键自动安装（推荐）
   sudo ./install-kernel.sh
   ```

### 方法2：从Release下载

1. **下载Release版本**
   - 进入GitHub仓库的Releases页面
   - 下载最新版本的 `kernel-*.tar.gz`

2. **解压并安装**
   ```bash
   # 解压内核包
   tar -xzf kernel-*.tar.gz
   
   # 一键自动安装
   sudo ./install-kernel.sh
   ```

## 🔧 自动安装脚本功能

安装脚本 `install-kernel.sh` 会自动完成以下操作：

✅ **安全检查**
- 检查root权限
- 检测系统类型（Ubuntu/Debian/CentOS/Fedora/Arch等）
- 验证内核包完整性

✅ **备份保护**
- 自动备份当前内核到 `/boot/backup-日期时间/`
- 保留原内核作为启动选项

✅ **完整安装**
- 安装内核文件到 `/boot/`
- 安装内核模块到 `/lib/modules/`
- 生成initramfs/initrd
- 更新GRUB引导配置
- 设置新内核为默认启动项

✅ **验证测试**
- 验证所有文件安装正确
- 检查权限设置
- 提供重启选项

## ⚡ 超快速安装（一行命令）

```bash
# 下载并安装最新版本（需要先设置GitHub仓库URL）
curl -L https://github.com/YOUR_USERNAME/YOUR_REPO/releases/latest/download/kernel-*.tar.gz | tar -xz && cd kernel-* && sudo ./install-kernel.sh
```

## 🛡️ 安全特性

### 自动备份
- 原内核文件备份到 `/boot/backup-YYYYMMDD-HHMMSS/`
- GRUB菜单保留原内核选项
- 如果新内核有问题，可以选择原内核启动

### 回滚方案
如果新内核启动失败：

1. **GRUB菜单回滚**
   - 重启时在GRUB菜单选择原内核
   - 或选择"Advanced options"中的原内核版本

2. **手动回滚**
   ```bash
   # 恢复备份的内核文件
   sudo cp /boot/backup-*/vmlinuz-* /boot/
   sudo cp /boot/backup-*/initrd.img-* /boot/
   sudo update-grub
   ```

## 📋 安装后验证

重启系统后，运行以下命令验证XanMod内核的修改效果：

### 验证硬盘容量（应显示放大3倍）
```bash
df -h
# 输出示例：
# /dev/sda1  300G  150G  135G  53% /  （实际100G显示为300G）
```

### 验证内存信息（应显示放大2倍）
```bash
cat /proc/meminfo | head -5
# 输出示例：
# MemTotal:       16777216 kB  （实际8GB显示为16GB）
# MemFree:         8388608 kB  （实际4GB显示为8GB）
```

### 验证CPU信息（应显示放大2倍）
```bash
cat /proc/cpuinfo | grep "cpu MHz"
# 输出示例：
# cpu MHz         : 5000.000  （实际2.5GHz显示为5GHz）
```

### 验证内核版本
```bash
uname -r
# 输出示例：
# 6.16.0-xanmod-custom
```

## ⚠️ 重要提醒

1. **备份重要数据**：安装前建议备份重要数据
2. **测试环境**：建议先在测试环境中验证
3. **网络连接**：确保安装过程中网络连接稳定
4. **磁盘空间**：确保 `/boot` 分区有足够空间（至少500MB）

## 🆘 故障排除

### 安装失败
```bash
# 检查磁盘空间
df -h /boot

# 检查权限
ls -la /boot/

# 查看安装日志
dmesg | tail -20
```

### 启动失败
1. 在GRUB菜单选择原内核
2. 检查 `/var/log/boot.log` 或 `/var/log/kern.log`
3. 使用备份内核文件恢复

### 性能问题
```bash
# 检查内核模块加载
lsmod

# 检查系统资源
top
free -h
```

## 📞 获取帮助

- 查看详细安装说明：`cat INSTALL.md`
- 查看GitHub Actions构建日志
- 提交Issue到GitHub仓库

---

**享受您的XanMod内核体验！** 🎉