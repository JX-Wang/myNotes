Linux内核的内存管理机制(一)之 内核申请内存的机制和接口
===

本文属于内核学习系列中一段,主要描述Linux内核申请内存时,会借助于什么样的机制和接口来实现.

<!-- TOC -->

- [基本概念](#基本概念)
	- [页](#页)
	- [区](#区)
- [内核用来管理内存的接口](#内核用来管理内存的接口)
	- [获取页大小的内存](#获取页大小的内存)
		- [gfp_mask](#gfp_mask)
		- [获得页](#获得页)
		- [获得内容为０的页](#获得内容为０的页)
		- [释放页](#释放页)
		- [使用例子](#使用例子)
	- [获取一定字节大小的内存](#获取一定字节大小的内存)
		- [分配物理地址连续的内存,kmalloc,kfree](#分配物理地址连续的内存kmallockfree)
		- [分配物理地址不一定连续的内存,ｖmalloc,vfree](#分配物理地址不一定连续的内存ｖmallocvfree)
- [内核用来管理内存的策略](#内核用来管理内存的策略)
	- [伙伴系统](#伙伴系统)
		- [分配块](#分配块)
		- [释放块](#释放块)
	- [slab分配器](#slab分配器)
		- [层次结构](#层次结构)
		- [使用接口](#使用接口)
- [reference](#reference)

<!-- /TOC -->

## 基本概念

### 页

Linux内核需要为每一个物理页分配一个struct page结构体,无论这个页是否空闲.

struct page定义在内核代码include/linux/mm.h中,每个struct page都对应一个物理页!4GB内存的linux中,大概占用几十MB空间.

```c
/*
 * 每个物理页都必须在内核内存中对应一个struct page,无论这页是否被使用
 */
struct page {
	page_flags_t flags;		/* 页的状态,包括:脏页,正在被写回,激活状态等提供原子性的操作*/
	atomic_t _count;		/* 引用计数,有多少进程,用于page_count()函数. */
	atomic_t _mapcount;		
	unsigned long private;		
	struct address_space *mapping;	/* 属于哪个地址空间 */
	pgoff_t index;			
	struct list_head lru;		
		
#if defined(WANT_PAGE_VIRTUAL)
	void *virtual;			/* Kernel virtual address (NULL if
					   not kmapped, ie. highmem) */
#endif /* WANT_PAGE_VIRTUAL */
};
```

### 区

物理页并不是平等的存在,而是存在某些页仅用于一些特殊用途.比如DMA.所以Linux使用区这个概念,根据用途划分物理页.

include/linux/mmzone.h中有这几种区:
```c
#define ZONE_DMA		0	/* 有些IO设备只能使用固定物理内存作为DMA */
#define ZONE_NORMAL		1	/* 一般性的内存区,可以随意使用 */
#define ZONE_HIGHMEM		2	/* 高端内存区,这个区里的页和物理页的对应关系是不固定的,方便内核访问更多的内存 */
/*还有更多,不再列出*/
```	

区并不只是个概念,而是具有自己的数据结构.

```c
struct zone {
	/* Fields commonly accessed by the page allocator */
	unsigned long		free_pages;
	unsigned long		pages_min, pages_low, pages_high;
	/*
	 * We don't know if the memory that we're going to allocate will be freeable
	 * or/and it will be released eventually, so to avoid totally wasting several
	 * GB of ram we must reserve some of the lower zone memory (otherwise we risk
	 * to run OOM on the lower zones despite there's tons of freeable ram
	 * on the higher zones). This array is recalculated at runtime if the
	 * sysctl_lowmem_reserve_ratio sysctl changes.
	 */
	unsigned long		lowmem_reserve[MAX_NR_ZONES];

	struct per_cpu_pageset	pageset[NR_CPUS];

	/*
	 * free areas of different sizes
	 */
	spinlock_t		lock;
	struct free_area	free_area[MAX_ORDER];


	ZONE_PADDING(_pad1_)

	/* Fields commonly accessed by the page reclaim scanner */
	spinlock_t		lru_lock;	
	struct list_head	active_list;
	struct list_head	inactive_list;
	unsigned long		nr_scan_active;
	unsigned long		nr_scan_inactive;
	unsigned long		nr_active;
	unsigned long		nr_inactive;
	unsigned long		pages_scanned;	   /* since last reclaim */
	int			all_unreclaimable; /* All pages pinned */

	/*
	 * prev_priority holds the scanning priority for this zone.  It is
	 * defined as the scanning priority at which we achieved our reclaim
	 * target at the previous try_to_free_pages() or balance_pgdat()
	 * invokation.
	 *
	 * We use prev_priority as a measure of how much stress page reclaim is
	 * under - it drives the swappiness decision: whether to unmap mapped
	 * pages.
	 *
	 * temp_priority is used to remember the scanning priority at which
	 * this zone was successfully refilled to free_pages == pages_high.
	 *
	 * Access to both these fields is quite racy even on uniprocessor.  But
	 * it is expected to average out OK.
	 */
	int temp_priority;
	int prev_priority;


	ZONE_PADDING(_pad2_)
	
	wait_queue_head_t	* wait_table;
	unsigned long		wait_table_size;
	unsigned long		wait_table_bits;

	/*
	 * Discontig memory support fields.
	 */
	struct pglist_data	*zone_pgdat;
	struct page		*zone_mem_map;/* 这个区的页数组的首地址,是mem_map的一部分 */
	/* zone_start_pfn == zone_start_paddr >> PAGE_SHIFT */
	unsigned long		zone_start_pfn;

	unsigned long		spanned_pages;	/* total size, including holes */
	unsigned long		present_pages;	/* amount of memory (excluding holes) */

	/*
	 * rarely used fields:
	 */
	char			*name;
} ____cacheline_maxaligned_in_smp;
```

## 内核用来管理内存的接口

### 获取页大小的内存

#### gfp_mask

内核分配内存常常要加上一些说明,比如内存用途等.gfp_mask就是用来表示这些用途的,定义有:
```c
#define __GFP_WAIT	0x10	/* Can wait and reschedule? */
#define __GFP_HIGH	0x20	/* Should access emergency pools? */
#define __GFP_IO	0x40	/* Can start physical IO? */
#define __GFP_FS	0x80	/* Can call down to low-level FS? */
#define __GFP_COLD	0x100	/* Cache-cold page required */
#define __GFP_NOWARN	0x200	/* Suppress page allocation failure warning */
#define __GFP_REPEAT	0x400	/* Retry the allocation.  Might fail */
#define __GFP_NOFAIL	0x800	/* Retry for ever.  Cannot fail */
#define __GFP_NORETRY	0x1000	/* Do not retry.  Might fail */
#define __GFP_NO_GROW	0x2000	/* Slab internal usage */
#define __GFP_COMP	0x4000	/* Add compound page metadata */
#define __GFP_ZERO	0x8000	/* Return zeroed page on success */

#define __GFP_BITS_SHIFT 16	/* Room for 16 __GFP_FOO bits */
#define __GFP_BITS_MASK ((1 << __GFP_BITS_SHIFT) - 1)

/* if you forget to add the bitmask here kernel will crash, period */
#define GFP_LEVEL_MASK (__GFP_WAIT|__GFP_HIGH|__GFP_IO|__GFP_FS| \
			__GFP_COLD|__GFP_NOWARN|__GFP_REPEAT| \
			__GFP_NOFAIL|__GFP_NORETRY|__GFP_NO_GROW|__GFP_COMP)

#define GFP_ATOMIC	(__GFP_HIGH)
#define GFP_NOIO	(__GFP_WAIT)
#define GFP_NOFS	(__GFP_WAIT | __GFP_IO)
#define GFP_KERNEL	(__GFP_WAIT | __GFP_IO | __GFP_FS)
#define GFP_USER	(__GFP_WAIT | __GFP_IO | __GFP_FS)
#define GFP_HIGHUSER	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HIGHMEM)
```

#### 获得页

内核提供这样一些接口，让用户获得页．

```c
struct page* alloc_pages(gfp_mask, order) /* 请求分配一系列页 */
struct page* alloc_page(gfp_mask)		  /* 请求分配一个页 */
```

其中，alloc_pages是最核心的,它会分配2的order次方个连续的物理页.这种接口语义和伙伴系统有关.

#### 获得内容为０的页

页上面也会有数据残留，linux提供一些函数来获取一个初始化为0的页.他们在为操作系统用户分配内存时特别有用.

```c
unsigned long get_zeroed_page(unsigned int gfp_mask);
```

#### 释放页

```c
void __free_pages(struct page *page, unsigned int order);
void free_pages(unsigned long addr,unsigned int order);
void free_page(unsigned long addr);
```

内核对于这些函数不做任何防御,完全信赖内核开发者.

#### 使用例子

```c
unsigned long page;
page=alloc_pages(GFP_KERNEL,3);/*为内核分配２的３次方个连续页*/
if(!page){
	/* 分配失败 */

	/*......
	......*/
}

/*
	使用之
*/

free_pages(page,3);/*参数一定不要传错,内核不做任何检查!*/
```

### 获取一定字节大小的内存 

#### 分配物理地址连续的内存,kmalloc,kfree

如果仅仅想为一个变量获取内存,比如struct page,就不需要获取一整页.内核中常使用kmalloc来获取小内存,这个接口的功能类似与用户空间中的malloc.

```c
void * kmalloc (size_t size,gfp_t flags);
void kfree(const void* ptr);
```

使用例子:
```c
//内核分配一块内存放page结构体,再释放
struct page* ptr;
ptr=kmalloc(sizeof(struct page),GFP_KERNEL);
kfree(ptr);
```

#### 分配物理地址不一定连续的内存,ｖmalloc,vfree

vmalloc分配虚拟内存空间上连续的内存,不保证在物理地址也连续.vmalloc通过更新页表来实现这一点.



## 内核用来管理内存的策略

上面主要介绍接口,下面说说策略.策略问题关系到如何分配内存,使得速度快并且消耗小.著名的策略有伙伴系统等.

### 伙伴系统

伙伴系统是个很有效的策略,一定程度上解决了外碎片的问题.

伙伴系统中,以order描述一块连续内存的大小,每一块连续内存具有2的order次方个页.伙伴系统维护11个块链表,每个块链表内都是固定大小的空闲块,每个块的order分别为0,1,2,3...10.页的分配只按照最合适的order大小来分配,这样会导致内碎片问题.但是,内碎片往往是由于占用内存太小不足以覆盖页块,这样的内存分配在内核中一般使用其他机制,比如slab.所以伙伴系统专门用于服务大量内存分配.

#### 分配块

alloc_page()函数内部使用了伙伴系统分配空闲块的策略.

对于指定的块order,记为b.  
* 伙伴系统首先查找order为b的空闲块链表,如果有空闲的块,则直接分配之.  
* 否则,查找order为b+1的空闲块链表.如果存在空闲块,则把它一分为二,一般用来分配,一般插入到order为b的空闲块链表.
* 否则,迭代查找order为b+2,b+3,..的空闲块链表.如果在某次迭代查找到了空闲块,则把它拆分后插入更小order的空闲块链表中,最终满足分配需求.
* 如果最终没有查找到,返回错误信息.

#### 释放块

free_page()函数内部使用释放块策略.

释放order为b的块时:

* 伙伴系统计算出这个块的伙伴块.
* 如果伙伴块不是空闲的,把原块插入空闲快链表.
* 否则,合并原块和伙伴块,形成新的块.
* 继续尝试合并新块,直到无法继续合并.

伙伴块的定义如下:
* 两个块是连续的
* 两个块大小相同
* 第一个块的首物理地址是2b乘2的12次方的倍数.


<!-- TODO: 伙伴系统和页表机制是不是有冲突? -->

### slab分配器

linux内核中的slab就相当于一个内存池系统.[官方文档](https://www.kernel.org/doc/gorman/html/understand/understand011.html)有更详细的描述.


#### 层次结构

本来,内核的内存管理不在乎分配出去的内存用于什么用途,但是slab机制引入了对用途的考虑.它把内存进一步划分,不同部分专门用来分配给一类特定的对象(比如struct page,struct inode).这样一来,很多**新对象的内存申请**都可以直接使用**废弃掉的老对象的内存**,slab机制只要维护这样的对象信息,就**不需要重新查找内存**.这样也可以一定程度避免内存碎片的问题.

**这样的设计之所以可行,是因为内核经常性的创建和销毁统一类型的对象**.想一想确实如此，内核经常创建```struct task_struct```,```struct spinlock```,```struct inode```，文件描述副等结构．

slab分配器维护一个缓存(cache)链表.每个缓存含有很多slab,每个slab只包含一类对象.一个slab通常对应一个物理页,这个页专门用来存放一类对象.slab具有三种状态:满,空,半满.他们的层次结构是这样的:  

![slab structure](slab-structure.png)

slab的定义如下:

```c
struct slab {
	struct list_head	list;
	unsigned long		colouroff;
	void			*s_mem;		
	unsigned int		inuse;		/* 这个slab中已经分配了多少对象 */
	kmem_bufctl_t		free;		/* 指向第一个空闲的对象,如果有的话 */
};
```

#### 使用接口

* 高速缓存的创建，撤销

```c
/*
 * name:		高速缓存的名字
 * size:		高速缓存内每个对象的大小
 * align:		slab内第一个对象的偏移量,用于对齐
 * flags:		一些选项
 * ctor:		构造函数
 * dtor:		析构函数
 */
kmem_cache_t *
kmem_cache_create (const char *name, size_t size, size_t align,
	unsigned long flags, void (*ctor)(void*, kmem_cache_t *, unsigned long),
	void (*dtor)(void*, kmem_cache_t *, unsigned long))

int kmem_cache_destroy (kmem_cache_t * cachep);

```

* 高速缓存中对象的分配,释放

```c
void * kmem_cache_alloc (kmem_cache_t *cachep, int flags);
void kmem_cache_free (kmem_cache_t *cachep, void *objp);
```

## reference

* [Slab Allocator](https://www.kernel.org/doc/gorman/html/understand/understand011.html)

* linux-2.6.11.1

* Linux内核设计与实现,第三版  
