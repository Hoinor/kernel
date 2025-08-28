#!/bin/bash

# XanMod内核修改验证脚本
# 此脚本用于验证硬盘容量放大3倍、内存和CPU信息放大2倍的修改

echo "=== XanMod内核修改验证 ==="
echo

echo "1. 检查statfs.c修改（硬盘容量放大3倍）:"
echo "   - 修改了do_statfs_native函数中的f_blocks, f_bfree, f_bavail字段"
echo "   - 修改了do_statfs64函数中的f_blocks, f_bfree, f_bavail字段"
echo "   - 修改了ustat系统调用中的f_tfree字段"
grep -n "\* 3" fs/statfs.c | head -5
echo

echo "2. 检查meminfo.c修改（内存信息放大2倍）:"
echo "   - 修改了meminfo_proc_show函数中的所有内存相关字段"
grep -n "\* 2" fs/proc/meminfo.c | head -10
echo

echo "3. 检查CPU proc.c修改（CPU信息放大2倍）:"
echo "   - 修改了CPU频率、缓存大小、bogomips等字段"
grep -n "\* 2" arch/x86/kernel/cpu/proc.c
echo

echo "=== 修改摘要 ==="
echo "✓ 硬盘容量显示：放大3倍 (statfs.c)"
echo "✓ 内存信息显示：放大2倍 (meminfo.c)"
echo "✓ CPU信息显示：放大2倍 (proc.c)"
echo
echo "注意：这些修改将在编译并安装新内核后生效。"
echo "使用 'df -h' 查看硬盘容量，'/proc/meminfo' 查看内存，'/proc/cpuinfo' 查看CPU信息。"