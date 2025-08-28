# Windows环境下安装XanMod内核指南

由于您当前在Windows环境下工作，以下是几种在Linux系统中安装编译好的XanMod内核的方法。

## 🖥️ 安装环境选择

### 方法1：WSL2环境（推荐用于测试）

**注意**：WSL2使用的是微软提供的Linux内核，无法直接替换。但可以用于测试内核功能。

```powershell
# 在PowerShell中启用WSL2
wsl --install

# 进入WSL2 Ubuntu环境
wsl

# 在WSL2中测试内核功能（模拟）
sudo apt update
sudo apt install build-essential
```

### 方法2：虚拟机安装（推荐用于完整测试）

#### 使用VirtualBox
1. **下载VirtualBox**：https://www.virtualbox.org/
2. **创建Linux虚拟机**（推荐Ubuntu 22.04 LTS）
3. **在虚拟机中安装内核**

#### 使用VMware
1. **下载VMware Workstation**
2. **创建Linux虚拟机**
3. **在虚拟机中安装内核**

### 方法3：双系统安装（用于生产环境）

如果您有Linux双系统或专用Linux机器，可以直接安装。

## 📦 获取编译好的内核

### 从GitHub Actions下载

1. **在Windows中下载**
   ```powershell
   # 使用PowerShell下载（替换为您的仓库URL）
   Invoke-WebRequest -Uri "https://github.com/YOUR_USERNAME/YOUR_REPO/actions" -OutFile "actions.html"
   
   # 或直接在浏览器中访问GitHub Actions页面
   # https://github.com/YOUR_USERNAME/YOUR_REPO/actions
   ```

2. **下载artifacts**
   - 进入最新的成功构建
   - 下载 `kernel-build-*` 压缩包
   - 保存到易于访问的位置（如 `C:\Users\Hoin\Downloads\`）

### 从Release下载

```powershell
# 使用PowerShell下载最新Release
$repo = "YOUR_USERNAME/YOUR_REPO"
$latest = Invoke-RestMethod -Uri "https://api.github.com/repos/$repo/releases/latest"
$downloadUrl = $latest.assets | Where-Object { $_.name -like "kernel-*.tar.gz" } | Select-Object -First 1 -ExpandProperty browser_download_url
Invoke-WebRequest -Uri $downloadUrl -OutFile "kernel-latest.tar.gz"
```

## 🔄 传输文件到Linux环境

### 传输到WSL2

```powershell
# 从Windows复制到WSL2
copy "C:\Users\Hoin\Downloads\kernel-*.tar.gz" "\\wsl$\Ubuntu\home\username\"

# 或在WSL2中直接访问Windows文件
wsl
cd /mnt/c/Users/Hoin/Downloads/
cp kernel-*.tar.gz ~/
```

### 传输到虚拟机

#### VirtualBox共享文件夹
1. 虚拟机设置 → 共享文件夹
2. 添加Windows文件夹路径
3. 在Linux中挂载：
   ```bash
   sudo mkdir /mnt/shared
   sudo mount -t vboxsf SharedFolder /mnt/shared
   cp /mnt/shared/kernel-*.tar.gz ~/
   ```

#### VMware共享文件夹
1. 虚拟机设置 → 选项 → 共享文件夹
2. 启用共享文件夹
3. 在Linux中访问：
   ```bash
   cp /mnt/hgfs/SharedFolder/kernel-*.tar.gz ~/
   ```

#### 使用SCP/SFTP
```powershell
# 使用WinSCP或其他SFTP客户端
# 或使用PowerShell的SCP
scp kernel-*.tar.gz username@linux-ip:~/
```

## 🚀 在Linux环境中安装

### 自动安装（推荐）

```bash
# 解压内核包
tar -xzf kernel-*.tar.gz
cd kernel-*/

# 一键自动安装
sudo ./install-kernel.sh
```

### 手动安装

```bash
# 备份当前内核
sudo cp /boot/vmlinuz-$(uname -r) /boot/vmlinuz-$(uname -r).backup
sudo cp /boot/initrd.img-$(uname -r) /boot/initrd.img-$(uname -r).backup

# 安装新内核
sudo cp boot/* /boot/
sudo cp -r lib/modules/* /lib/modules/

# 更新引导加载程序
sudo update-grub

# 重启
sudo reboot
```

## 🧪 测试和验证

### 在虚拟机中测试

1. **重启虚拟机**
2. **验证内核版本**
   ```bash
   uname -r
   # 应显示：6.16.0-xanmod-custom
   ```

3. **验证修改效果**
   ```bash
   # 硬盘容量（放大3倍）
   df -h
   
   # 内存信息（放大2倍）
   cat /proc/meminfo | head -5
   
   # CPU信息（放大2倍）
   cat /proc/cpuinfo | grep "cpu MHz"
   ```

### 性能测试

```bash
# 系统信息
neofetch

# CPU性能测试
sysbench cpu run

# 内存测试
free -h

# 磁盘IO测试
dd if=/dev/zero of=testfile bs=1G count=1 oflag=direct
```

## 🔧 Windows工具推荐

### 文件传输工具
- **WinSCP**：图形化SFTP/SCP客户端
- **FileZilla**：FTP/SFTP客户端
- **MobaXterm**：集成SSH/SFTP的终端

### 虚拟机管理
- **VirtualBox**：免费虚拟机软件
- **VMware Workstation**：专业虚拟机软件
- **Hyper-V**：Windows内置虚拟化

### 远程连接工具
- **PuTTY**：SSH客户端
- **Windows Terminal**：现代终端应用
- **MobaXterm**：多功能终端

## 📋 自动化脚本（PowerShell）

创建一个PowerShell脚本自动化整个过程：

```powershell
# download-and-install.ps1
param(
    [string]$VMName = "Ubuntu-Test",
    [string]$VMUser = "username",
    [string]$VMPassword = "password"
)

# 下载最新内核
Write-Host "下载最新内核包..."
$repo = "YOUR_USERNAME/YOUR_REPO"
$latest = Invoke-RestMethod -Uri "https://api.github.com/repos/$repo/releases/latest"
$downloadUrl = $latest.assets | Where-Object { $_.name -like "kernel-*.tar.gz" } | Select-Object -First 1 -ExpandProperty browser_download_url
Invoke-WebRequest -Uri $downloadUrl -OutFile "kernel-latest.tar.gz"

# 传输到虚拟机（需要配置SSH密钥或使用其他方法）
Write-Host "传输到虚拟机..."
# scp kernel-latest.tar.gz $VMUser@$VMName:~/

# 在虚拟机中执行安装（需要SSH连接）
Write-Host "在虚拟机中安装内核..."
# ssh $VMUser@$VMName "tar -xzf kernel-latest.tar.gz && cd kernel-*/ && sudo ./install-kernel.sh"

Write-Host "安装完成！请重启虚拟机以使用新内核。"
```

## ⚠️ 注意事项

1. **虚拟机资源**：确保虚拟机有足够的内存和磁盘空间
2. **网络配置**：确保虚拟机可以访问互联网
3. **快照备份**：安装前创建虚拟机快照
4. **测试环境**：建议先在测试环境验证

## 🆘 故障排除

### 虚拟机问题
- 检查虚拟化是否启用（BIOS设置）
- 确保有足够的系统资源
- 检查网络连接

### 文件传输问题
- 检查防火墙设置
- 验证SSH服务状态
- 确认文件路径正确

### 内核安装问题
- 检查磁盘空间
- 验证文件完整性
- 查看系统日志

---

通过以上方法，您可以在Windows环境下成功编译、传输和安装XanMod内核，并验证硬盘容量、内存和CPU信息的修改效果。