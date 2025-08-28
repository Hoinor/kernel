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

void __attribute__((weak)) arch_report_meminfo(struct seq_file *m)
{
}

static void show_val_kb(struct seq_file *m, const char *s, unsigned long num)
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

	show_val_kb(m, "MemTotal:       ", i.totalram * 2);  /* XanMod: 内存总量放大2倍 */
	show_val_kb(m, "MemFree:        ", i.freeram * 2);   /* XanMod: 可用内存放大2倍 */
	show_val_kb(m, "MemAvailable:   ", available * 2);   /* XanMod: 可用内存放大2倍 */
	show_val_kb(m, "Buffers:        ", i.bufferram * 2); /* XanMod: 缓冲区放大2倍 */
	show_val_kb(m, "Cached:         ", cached * 2);      /* XanMod: 缓存放大2倍 */
	show_val_kb(m, "SwapCached:     ", total_swapcache_pages() * 2); /* XanMod: 交换缓存放大2倍 */
	show_val_kb(m, "Active:         ", (pages[LRU_ACTIVE_ANON] +
					   pages[LRU_ACTIVE_FILE]) * 2); /* XanMod: 活跃内存放大2倍 */
	show_val_kb(m, "Inactive:       ", (pages[LRU_INACTIVE_ANON] +
					   pages[LRU_INACTIVE_FILE]) * 2); /* XanMod: 非活跃内存放大2倍 */
	show_val_kb(m, "Active(anon):   ", pages[LRU_ACTIVE_ANON] * 2);   /* XanMod: 活跃匿名页放大2倍 */
	show_val_kb(m, "Inactive(anon): ", pages[LRU_INACTIVE_ANON] * 2); /* XanMod: 非活跃匿名页放大2倍 */
	show_val_kb(m, "Active(file):   ", pages[LRU_ACTIVE_FILE] * 2);   /* XanMod: 活跃文件页放大2倍 */
	show_val_kb(m, "Inactive(file): ", pages[LRU_INACTIVE_FILE] * 2); /* XanMod: 非活跃文件页放大2倍 */
	show_val_kb(m, "Unevictable:    ", pages[LRU_UNEVICTABLE] * 2);   /* XanMod: 不可回收页放大2倍 */
	show_val_kb(m, "Mlocked:        ", global_zone_page_state(NR_MLOCK) * 2); /* XanMod: 锁定内存放大2倍 */

#ifdef CONFIG_HIGHMEM
	show_val_kb(m, "HighTotal:      ", i.totalhigh * 2);           /* XanMod: 高端内存总量放大2倍 */
	show_val_kb(m, "HighFree:       ", i.freehigh * 2);            /* XanMod: 高端内存可用放大2倍 */
	show_val_kb(m, "LowTotal:       ", (i.totalram - i.totalhigh) * 2); /* XanMod: 低端内存总量放大2倍 */
	show_val_kb(m, "LowFree:        ", (i.freeram - i.freehigh) * 2);   /* XanMod: 低端内存可用放大2倍 */
#endif

#ifndef CONFIG_MMU
	show_val_kb(m, "MmapCopy:       ",
		    (unsigned long)atomic_long_read(&mmap_pages_allocated));
#endif

	show_val_kb(m, "SwapTotal:      ", i.totalswap * 2);  /* XanMod: 交换空间总量放大2倍 */
	show_val_kb(m, "SwapFree:       ", i.freeswap * 2);   /* XanMod: 交换空间可用放大2倍 */
#ifdef CONFIG_ZSWAP
	show_val_kb(m, "Zswap:          ", zswap_total_pages() * 2); /* XanMod: Zswap放大2倍 */
	seq_printf(m,  "Zswapped:       %8lu kB\n",
		   ((unsigned long)atomic_long_read(&zswap_stored_pages) <<
		   (PAGE_SHIFT - 10)) * 2); /* XanMod: Zswapped放大2倍 */
#endif
	show_val_kb(m, "Dirty:          ",
		    global_node_page_state(NR_FILE_DIRTY) * 2); /* XanMod: 脏页放大2倍 */
	show_val_kb(m, "Writeback:      ",
		    global_node_page_state(NR_WRITEBACK) * 2); /* XanMod: 回写页放大2倍 */
	show_val_kb(m, "AnonPages:      ",
		    global_node_page_state(NR_ANON_MAPPED) * 2); /* XanMod: 匿名页放大2倍 */
	show_val_kb(m, "Mapped:         ",
		    global_node_page_state(NR_FILE_MAPPED) * 2); /* XanMod: 映射页放大2倍 */
	show_val_kb(m, "Shmem:          ", i.sharedram * 2); /* XanMod: 共享内存放大2倍 */
	show_val_kb(m, "KReclaimable:   ", (sreclaimable +
		    global_node_page_state(NR_KERNEL_MISC_RECLAIMABLE)) * 2); /* XanMod: 可回收内核内存放大2倍 */
	show_val_kb(m, "Slab:           ", (sreclaimable + sunreclaim) * 2); /* XanMod: Slab内存放大2倍 */
	show_val_kb(m, "SReclaimable:   ", sreclaimable * 2); /* XanMod: 可回收Slab放大2倍 */
	show_val_kb(m, "SUnreclaim:     ", sunreclaim * 2);   /* XanMod: 不可回收Slab放大2倍 */
	seq_printf(m, "KernelStack:    %8lu kB\n",
		   global_node_page_state(NR_KERNEL_STACK_KB) * 2); /* XanMod: 内核栈放大2倍 */
#ifdef CONFIG_SHADOW_CALL_STACK
	seq_printf(m, "ShadowCallStack:%8lu kB\n",
		   global_node_page_state(NR_KERNEL_SCS_KB) * 2); /* XanMod: 影子调用栈放大2倍 */
#endif
	show_val_kb(m, "PageTables:     ",
		    global_node_page_state(NR_PAGETABLE) * 2); /* XanMod: 页表放大2倍 */
	show_val_kb(m, "SecPageTables:  ",
		    global_node_page_state(NR_SECONDARY_PAGETABLE) * 2); /* XanMod: 二级页表放大2倍 */

	show_val_kb(m, "NFS_Unstable:   ", 0);
	show_val_kb(m, "Bounce:         ", 0);
	show_val_kb(m, "WritebackTmp:   ",
		    global_node_page_state(NR_WRITEBACK_TEMP) * 2); /* XanMod: 临时回写放大2倍 */
	show_val_kb(m, "CommitLimit:    ", vm_commit_limit() * 2); /* XanMod: 提交限制放大2倍 */
	show_val_kb(m, "Committed_AS:   ", committed * 2); /* XanMod: 已提交内存放大2倍 */
	seq_printf(m, "VmallocTotal:   %8lu kB\n",
		   ((unsigned long)VMALLOC_TOTAL >> 10) * 2); /* XanMod: Vmalloc总量放大2倍 */
	show_val_kb(m, "VmallocUsed:    ", vmalloc_nr_pages() * 2); /* XanMod: Vmalloc使用放大2倍 */
	show_val_kb(m, "VmallocChunk:   ", 0ul);
	show_val_kb(m, "Percpu:         ", pcpu_nr_pages() * 2); /* XanMod: 每CPU内存放大2倍 */

	memtest_report_meminfo(m);

#ifdef CONFIG_MEMORY_FAILURE
	seq_printf(m, "HardwareCorrupted: %5lu kB\n",
		   atomic_long_read(&num_poisoned_pages) << (PAGE_SHIFT - 10));
#endif

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	show_val_kb(m, "AnonHugePages:  ",
		    global_node_page_state(NR_ANON_THPS));
	show_val_kb(m, "ShmemHugePages: ",
		    global_node_page_state(NR_SHMEM_THPS));
	show_val_kb(m, "ShmemPmdMapped: ",
		    global_node_page_state(NR_SHMEM_PMDMAPPED));
	show_val_kb(m, "FileHugePages:  ",
		    global_node_page_state(NR_FILE_THPS));
	show_val_kb(m, "FilePmdMapped:  ",
		    global_node_page_state(NR_FILE_PMDMAPPED));
#endif

#ifdef CONFIG_CMA
	show_val_kb(m, "CmaTotal:       ", totalcma_pages);
	show_val_kb(m, "CmaFree:        ",
		    global_zone_page_state(NR_FREE_CMA_PAGES));
#endif

#ifdef CONFIG_UNACCEPTED_MEMORY
	show_val_kb(m, "Unaccepted:     ",
		    global_zone_page_state(NR_UNACCEPTED));
#endif
	show_val_kb(m, "Balloon:        ",
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
