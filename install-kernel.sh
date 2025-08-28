#!/bin/bash

# XanMod内核自动安装脚本
# 此脚本将自动安装编译好的内核并设置为默认启动项

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查是否为root用户
check_root() {
    if [[ $EUID -ne 0 ]]; then
        log_error "此脚本需要root权限运行"
        echo "请使用: sudo $0"
        exit 1
    fi
}

# 检查系统类型
check_system() {
    if [[ ! -f /etc/os-release ]]; then
        log_error "无法检测系统类型"
        exit 1
    fi
    
    source /etc/os-release
    log_info "检测到系统: $PRETTY_NAME"
    
    # 检查是否为支持的系统
    case $ID in
        ubuntu|debian)
            BOOTLOADER_CMD="update-grub"
            ;;
        centos|rhel|fedora)
            BOOTLOADER_CMD="grub2-mkconfig -o /boot/grub2/grub.cfg"
            ;;
        arch|manjaro)
            BOOTLOADER_CMD="grub-mkconfig -o /boot/grub/grub.cfg"
            ;;
        *)
            log_warning "未知系统类型，将尝试使用通用命令"
            BOOTLOADER_CMD="update-grub"
            ;;
    esac
}

# 备份当前内核
backup_current_kernel() {
    log_info "备份当前内核..."
    
    CURRENT_KERNEL=$(uname -r)
    BACKUP_DIR="/boot/backup-$(date +%Y%m%d-%H%M%S)"
    
    mkdir -p "$BACKUP_DIR"
    
    # 备份当前内核文件
    if [[ -f "/boot/vmlinuz-$CURRENT_KERNEL" ]]; then
        cp "/boot/vmlinuz-$CURRENT_KERNEL" "$BACKUP_DIR/"
        log_success "已备份内核镜像"
    fi
    
    if [[ -f "/boot/initrd.img-$CURRENT_KERNEL" ]]; then
        cp "/boot/initrd.img-$CURRENT_KERNEL" "$BACKUP_DIR/"
        log_success "已备份initrd镜像"
    fi
    
    if [[ -f "/boot/System.map-$CURRENT_KERNEL" ]]; then
        cp "/boot/System.map-$CURRENT_KERNEL" "$BACKUP_DIR/"
        log_success "已备份System.map"
    fi
    
    if [[ -f "/boot/config-$CURRENT_KERNEL" ]]; then
        cp "/boot/config-$CURRENT_KERNEL" "$BACKUP_DIR/"
        log_success "已备份内核配置"
    fi
    
    log_success "内核备份完成: $BACKUP_DIR"
}

# 安装新内核
install_kernel() {
    log_info "开始安装XanMod内核..."
    
    # 检查内核包是否存在
    KERNEL_PACKAGE=$(find . -name "kernel-*.tar.gz" | head -1)
    
    if [[ -z "$KERNEL_PACKAGE" ]]; then
        log_error "未找到内核安装包 (kernel-*.tar.gz)"
        echo "请确保内核包在当前目录中"
        exit 1
    fi
    
    log_info "找到内核包: $KERNEL_PACKAGE"
    
    # 解压内核包
    log_info "解压内核包..."
    tar -xzf "$KERNEL_PACKAGE"
    
    # 安装内核文件
    if [[ -d "boot" ]]; then
        log_info "安装内核文件到 /boot..."
        cp -r boot/* /boot/
        log_success "内核文件安装完成"
    else
        log_error "内核包中未找到boot目录"
        exit 1
    fi
    
    # 安装内核模块
    if [[ -d "lib/modules" ]]; then
        log_info "安装内核模块到 /lib/modules..."
        cp -r lib/modules/* /lib/modules/
        log_success "内核模块安装完成"
    else
        log_warning "内核包中未找到模块目录"
    fi
    
    # 设置文件权限
    chmod 644 /boot/vmlinuz-*
    chmod 644 /boot/System.map-*
    chmod 644 /boot/config-*
    
    log_success "文件权限设置完成"
}

# 生成initramfs
generate_initramfs() {
    log_info "生成initramfs..."
    
    # 获取新安装的内核版本
    NEW_KERNEL=$(ls /boot/vmlinuz-* | grep -v backup | sort -V | tail -1 | sed 's/.*vmlinuz-//')
    
    if [[ -z "$NEW_KERNEL" ]]; then
        log_error "无法确定新内核版本"
        exit 1
    fi
    
    log_info "为内核 $NEW_KERNEL 生成initramfs"
    
    # 根据系统类型生成initramfs
    case $ID in
        ubuntu|debian)
            update-initramfs -c -k "$NEW_KERNEL"
            ;;
        centos|rhel|fedora)
            dracut --force "/boot/initramfs-$NEW_KERNEL.img" "$NEW_KERNEL"
            ;;
        arch|manjaro)
            mkinitcpio -k "$NEW_KERNEL" -g "/boot/initramfs-$NEW_KERNEL.img"
            ;;
        *)
            log_warning "未知系统，尝试使用update-initramfs"
            update-initramfs -c -k "$NEW_KERNEL" || true
            ;;
    esac
    
    log_success "initramfs生成完成"
}

# 更新引导加载程序
update_bootloader() {
    log_info "更新引导加载程序配置..."
    
    # 执行引导加载程序更新命令
    if command -v $BOOTLOADER_CMD >/dev/null 2>&1; then
        $BOOTLOADER_CMD
        log_success "引导加载程序配置更新完成"
    else
        log_error "引导加载程序命令不存在: $BOOTLOADER_CMD"
        exit 1
    fi
}

# 设置默认启动内核
set_default_kernel() {
    log_info "设置新内核为默认启动项..."
    
    # 获取新内核的grub菜单项
    if [[ -f /boot/grub/grub.cfg ]]; then
        GRUB_CONFIG="/boot/grub/grub.cfg"
    elif [[ -f /boot/grub2/grub.cfg ]]; then
        GRUB_CONFIG="/boot/grub2/grub.cfg"
    else
        log_warning "未找到GRUB配置文件，跳过默认内核设置"
        return
    fi
    
    # 查找新内核的菜单项
    NEW_KERNEL_ENTRY=$(grep "menuentry.*$NEW_KERNEL" "$GRUB_CONFIG" | head -1 | sed "s/.*menuentry '\([^']*\)'.*/\1/")
    
    if [[ -n "$NEW_KERNEL_ENTRY" ]]; then
        log_info "找到内核菜单项: $NEW_KERNEL_ENTRY"
        
        # 更新GRUB默认项
        if [[ -f /etc/default/grub ]]; then
            sed -i "s/^GRUB_DEFAULT=.*/GRUB_DEFAULT=\"$NEW_KERNEL_ENTRY\"/" /etc/default/grub
            $BOOTLOADER_CMD
            log_success "已设置新内核为默认启动项"
        fi
    else
        log_warning "无法找到新内核的GRUB菜单项"
    fi
}

# 验证安装
verify_installation() {
    log_info "验证安装..."
    
    # 检查内核文件
    if [[ -f "/boot/vmlinuz-$NEW_KERNEL" ]]; then
        log_success "内核镜像安装成功"
    else
        log_error "内核镜像安装失败"
        return 1
    fi
    
    # 检查initramfs
    if [[ -f "/boot/initrd.img-$NEW_KERNEL" ]] || [[ -f "/boot/initramfs-$NEW_KERNEL.img" ]]; then
        log_success "initramfs生成成功"
    else
        log_warning "initramfs可能生成失败"
    fi
    
    # 检查模块目录
    if [[ -d "/lib/modules/$NEW_KERNEL" ]]; then
        log_success "内核模块安装成功"
    else
        log_warning "内核模块目录不存在"
    fi
    
    log_success "安装验证完成"
}

# 清理临时文件
cleanup() {
    log_info "清理临时文件..."
    
    # 删除解压的文件
    [[ -d "boot" ]] && rm -rf boot
    [[ -d "lib" ]] && rm -rf lib
    
    log_success "清理完成"
}

# 主函数
main() {
    echo "======================================"
    echo "    XanMod内核自动安装脚本"
    echo "======================================"
    echo
    
    check_root
    check_system
    
    echo
    read -p "是否继续安装XanMod内核？这将替换当前内核 [y/N]: " -n 1 -r
    echo
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        log_info "安装已取消"
        exit 0
    fi
    
    echo
    backup_current_kernel
    install_kernel
    generate_initramfs
    update_bootloader
    set_default_kernel
    verify_installation
    cleanup
    
    echo
    echo "======================================"
    log_success "XanMod内核安装完成！"
    echo "======================================"
    echo
    log_info "重启后将使用新内核启动"
    log_info "如果遇到问题，可以在GRUB菜单中选择原内核"
    log_info "原内核备份位置: $BACKUP_DIR"
    echo
    
    read -p "是否立即重启系统？ [y/N]: " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        log_info "正在重启系统..."
        reboot
    else
        log_info "请手动重启系统以使用新内核"
    fi
}

# 捕获中断信号
trap cleanup EXIT

# 运行主函数
main "$@"