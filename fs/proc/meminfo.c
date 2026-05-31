// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/memblock.h>
#include <linux/proc_fs.h>
#include <linux/percpu.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <linux/vmalloc.h>
#ifdef CONFIG_CMA
#include <linux/cma.h>
#endif
#include <linux/zswap.h>
#include <asm/page.h>
#include "internal.h"

/*
 * XanMod 自定义: 内存显示放大倍数
 * 设置为 2 表示所有内存值显示为实际值的 2 倍
 */
#define XANMOD_MEM_MULTIPLIER 2

void __attribute__((weak)) arch_report_meminfo(struct seq_file *m)
{
}

/*
 * 优化的内存值显示函数
 * 应用 XanMod 内存放大倍数
 */
static inline void show_val_kb_scaled(struct seq_file *m, const char *s,
				      unsigned long num)
{
	seq_put_decimal_ull_width(m, s,
				   (num << (PAGE_SHIFT - 10)) * XANMOD_MEM_MULTIPLIER,
				   8);
	seq_write(m, " kB\n", 4);
}

/*
 * 原始的内存值显示函数（不放大）
 * 用于不需要放大的特殊值
 */
static inline void show_val_kb(struct seq_file *m, const char *s,
			       unsigned long num)
{
	seq_put_decimal_ull_width(m, s, num << (PAGE_SHIFT - 10), 8);
	seq_write(m, " kB\n", 4);
}

static int meminfo_proc_show(struct seq_file *m, void *v)
{
	struct sysinfo i;
	unsigned long committed;
	long cached;
	long available;
	unsigned long pages[NR_LRU_LISTS];
	unsigned long sreclaimable, sunreclaim;
	int lru;

	si_meminfo(&i);
	si_swapinfo(&i);
	committed = vm_memory_committed();

	cached = global_node_page_state(NR_FILE_PAGES) -
			total_swapcache_pages() - i.bufferram;
	if (cached < 0)
		cached = 0;

	for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_node_page_state(NR_LRU_BASE + lru);

	available = si_mem_available();
	sreclaimable = global_node_page_state_pages(NR_SLAB_RECLAIMABLE_B);
	sunreclaim = global_node_page_state_pages(NR_SLAB_UNRECLAIMABLE_B);

	/* 基础内存信息 - 全部放大 */
	show_val_kb_scaled(m, "MemTotal:       ", i.totalram);
	show_val_kb_scaled(m, "MemFree:        ", i.freeram);
	show_val_kb_scaled(m, "MemAvailable:   ", available);
	show_val_kb_scaled(m, "Buffers:        ", i.bufferram);
	show_val_kb_scaled(m, "Cached:         ", cached);
	show_val_kb_scaled(m, "SwapCached:     ", total_swapcache_pages());

	/* LRU 页面统计 - 全部放大 */
	show_val_kb_scaled(m, "Active:         ",
			   pages[LRU_ACTIVE_ANON] + pages[LRU_ACTIVE_FILE]);
	show_val_kb_scaled(m, "Inactive:       ",
			   pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE]);
	show_val_kb_scaled(m, "Active(anon):   ", pages[LRU_ACTIVE_ANON]);
	show_val_kb_scaled(m, "Inactive(anon): ", pages[LRU_INACTIVE_ANON]);
	show_val_kb_scaled(m, "Active(file):   ", pages[LRU_ACTIVE_FILE]);
	show_val_kb_scaled(m, "Inactive(file): ", pages[LRU_INACTIVE_FILE]);
	show_val_kb_scaled(m, "Unevictable:    ", pages[LRU_UNEVICTABLE]);
	show_val_kb_scaled(m, "Mlocked:        ", global_zone_page_state(NR_MLOCK));

#ifdef CONFIG_HIGHMEM
	show_val_kb_scaled(m, "HighTotal:      ", i.totalhigh);
	show_val_kb_scaled(m, "HighFree:       ", i.freehigh);
	show_val_kb_scaled(m, "LowTotal:       ", i.totalram - i.totalhigh);
	show_val_kb_scaled(m, "LowFree:        ", i.freeram - i.freehigh);
#endif

#ifndef CONFIG_MMU
	show_val_kb_scaled(m, "MmapCopy:       ",
			   (unsigned long)atomic_long_read(&mmap_pages_allocated));
#endif

	/* 交换空间信息 - 全部放大 */
	show_val_kb_scaled(m, "SwapTotal:      ", i.totalswap);
	show_val_kb_scaled(m, "SwapFree:       ", i.freeswap);

#ifdef CONFIG_ZSWAP
	show_val_kb_scaled(m, "Zswap:          ", zswap_total_pages());
	seq_printf(m, "Zswapped:       %8lu kB\n",
		   ((unsigned long)atomic_long_read(&zswap_stored_pages) <<
		   (PAGE_SHIFT - 10)) * XANMOD_MEM_MULTIPLIER);
#endif

	/* 页面状态统计 - 全部放大 */
	show_val_kb_scaled(m, "Dirty:          ",
			   global_node_page_state(NR_FILE_DIRTY));
	show_val_kb_scaled(m, "Writeback:      ",
			   global_node_page_state(NR_WRITEBACK));
	show_val_kb_scaled(m, "AnonPages:      ",
			   global_node_page_state(NR_ANON_MAPPED));
	show_val_kb_scaled(m, "Mapped:         ",
			   global_node_page_state(NR_FILE_MAPPED));
	show_val_kb_scaled(m, "Shmem:          ", i.sharedram);

	/* 内核内存统计 - 全部放大 */
	show_val_kb_scaled(m, "KReclaimable:   ",
			   sreclaimable +
			   global_node_page_state(NR_KERNEL_MISC_RECLAIMABLE));
	show_val_kb_scaled(m, "Slab:           ", sreclaimable + sunreclaim);
	show_val_kb_scaled(m, "SReclaimable:   ", sreclaimable);
	show_val_kb_scaled(m, "SUnreclaim:     ", sunreclaim);

	seq_printf(m, "KernelStack:    %8lu kB\n",
		   global_node_page_state(NR_KERNEL_STACK_KB) * XANMOD_MEM_MULTIPLIER);

#ifdef CONFIG_SHADOW_CALL_STACK
	seq_printf(m, "ShadowCallStack:%8lu kB\n",
		   global_node_page_state(NR_KERNEL_SCS_KB) * XANMOD_MEM_MULTIPLIER);
#endif

	show_val_kb_scaled(m, "PageTables:     ",
			   global_node_page_state(NR_PAGETABLE));
	show_val_kb_scaled(m, "SecPageTables:  ",
			   global_node_page_state(NR_SECONDARY_PAGETABLE));

	/* 已废弃的字段 - 保持为 0 */
	show_val_kb(m, "NFS_Unstable:   ", 0);
	show_val_kb(m, "Bounce:         ", 0);

	show_val_kb_scaled(m, "WritebackTmp:   ",
			   global_node_page_state(NR_WRITEBACK_TEMP));
	show_val_kb_scaled(m, "CommitLimit:    ", vm_commit_limit());
	show_val_kb_scaled(m, "Committed_AS:   ", committed);

	seq_printf(m, "VmallocTotal:   %8lu kB\n",
		   ((unsigned long)VMALLOC_TOTAL >> 10) * XANMOD_MEM_MULTIPLIER);
	show_val_kb_scaled(m, "VmallocUsed:    ", vmalloc_nr_pages());
	show_val_kb(m, "VmallocChunk:   ", 0ul);
	show_val_kb_scaled(m, "Percpu:         ", pcpu_nr_pages());

	memtest_report_meminfo(m);

#ifdef CONFIG_MEMORY_FAILURE
	/* 硬件损坏页面不放大 - 这是真实的硬件错误 */
	seq_printf(m, "HardwareCorrupted: %5lu kB\n",
		   atomic_long_read(&num_poisoned_pages) << (PAGE_SHIFT - 10));
#endif

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	/* 大页统计 - 全部放大 */
	show_val_kb_scaled(m, "AnonHugePages:  ",
			   global_node_page_state(NR_ANON_THPS));
	show_val_kb_scaled(m, "ShmemHugePages: ",
			   global_node_page_state(NR_SHMEM_THPS));
	show_val_kb_scaled(m, "ShmemPmdMapped: ",
			   global_node_page_state(NR_SHMEM_PMDMAPPED));
	show_val_kb_scaled(m, "FileHugePages:  ",
			   global_node_page_state(NR_FILE_THPS));
	show_val_kb_scaled(m, "FilePmdMapped:  ",
			   global_node_page_state(NR_FILE_PMDMAPPED));
#endif

#ifdef CONFIG_CMA
	/* CMA 内存 - 全部放大 */
	show_val_kb_scaled(m, "CmaTotal:       ", totalcma_pages);
	show_val_kb_scaled(m, "CmaFree:        ",
			   global_zone_page_state(NR_FREE_CMA_PAGES));
#endif

#ifdef CONFIG_UNACCEPTED_MEMORY
	show_val_kb_scaled(m, "Unaccepted:     ",
			   global_zone_page_state(NR_UNACCEPTED));
#endif

	show_val_kb_scaled(m, "Balloon:        ",
			   global_node_page_state(NR_BALLOON_PAGES));

	hugetlb_report_meminfo(m);

	arch_report_meminfo(m);

	return 0;
}

static int __init proc_meminfo_init(void)
{
	struct proc_dir_entry *pde;

	pde = proc_create_single("meminfo", 0, NULL, meminfo_proc_show);
	pde_make_permanent(pde);
	return 0;
}
fs_initcall(proc_meminfo_init);
