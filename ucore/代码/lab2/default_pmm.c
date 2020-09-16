#include <pmm.h>
#include <list.h>
#include <string.h>
#include <default_pmm.h>

/* 
在First-Fit算法中，分配器保存一个空闲块的列表（称为空闲列表）。
一旦收到内存分配请求，它就会沿着列表扫描第一个足够大的块来满足请求。
如果所选的块比请求的大得多，通常会将其拆分，
剩余的块将作为另一个空闲块添加到列表中，
拜托参见阎维民《数据结构——C程序设计语言》一书第196~198页第8.2节。 
*/
// LAB2 EXERCISE 1: YOUR CODE
// you should rewrite functions: `default_init`, `default_init_memmap`,
// `default_alloc_pages`, `default_free_pages`.
/*
 * Details of FFMA
 * (1) Preparation:
	为了实现First-Fit内存分配（FFMA），我们应该使用一个列表来管理空闲内存块。
	“free_area_t”结构用于管理可用内存块。
	
	首先，您应该熟悉list.h中的结构list。
	结构list是一个简单的双向链表列表实现。
	你应该知道如何使用list_init、list_add（list_add_after）、list_add_before、list_del、list_next、list_prev。
	
	有一个棘手的方法是使用以下宏将一般的list结构转换为特殊的结构（例如struct page），
	使用以下宏：le2page（在memlayout.h中），(并且在以后的lab中le2vma（在vmm.h中）、le2proc（proc.h中）等。 ）
	
	（2）default_init：
	您可以重新使用demo default_init函数来初始化free_list，并将“nr_free”设置为0。
	free_list用于记录可用内存块。nr_free是可用内存块的总数。
	
	(3) `default_init_memmap`:
	调用图: `kern_init` --> `pmm_init` --> `page_init` --> `init_memmap` --> `pmm_manager` --> `init_memmap`.
	此函数用于初始化空闲块（使用参数'addr_base`，`page_number`）。
	为了初始化一个空闲块，首先应该初始化这个自由块中的每个页面（在memlayout.h中定义）。
	该程序包括：
	正在设置“p->flags”的“PG_property”位，这意味着此页有效。
	在函数“pmm_init”（在pmm.c中）中，“P->flags”的“PG_reserved”位已经设置好了。
	
	- 如果此页是空闲页，并且不是空闲块的第一页，则“p->property”应设置为0。
	- 如果此页是空闲页，并且是空闲块的第一页，则“p->property”应设置为块中的总页数。
	- `p->ref`应该是0，因为现在'p'是自由的，没有引用。
	
	然后，我们可以使用“p->page_link”将此页面链接到“free_list”
	（例如：list_add_before（&free_list，&（p->page_link））；） 
	
	最后，我们应该更新空闲内存块的总和：nr_free+=n。
	
	(4) `default_alloc_pages`:
	在空闲列表中搜索第一个空闲块（块大小>=n）并重新排序找到的块，将此块的地址返回为“malloc”所需的地址。
 *  (4.1)
 *      所以你应该像这样搜索空闲列表:
 *          list_entry_t le = &free_list;
 *          while((le=list_next(le)) != &free_list) {
 *          ...
        (4.1.1)
		在while循环中，获取结构page，并检查“p->property”（记录此块中的可用页数）>=n。
                struct Page *p = le2page(le, page_link);
                if(p->property >= n){ ...
 
        (4.1.2)
		如果我们找到这个'p'，这意味着我们找到了一个大小>=n的空闲块，
		它的第一个'n'页可以被malloce处理。此页的某些标志位应设置为：“PG_reserved=1”，`PG_property=0`。
		然后，取消页面与“free_list”的链接。
		
 *          (4.1.2.1)
 *              如果“p->property>n”，则应重新计算此空闲块的剩余页数。
			（例如： `le2page(le,page_link))->property
 *          = p->property - n;`)
 *          (4.1.3)
 *              重新计算`nr_free`（所有可用块的其余部分的编号）。
 *          (4.1.4)
 *              return `p`.
 *      (4.2)
 *          如果找不到大小大于等于n的空闲块，则返回NULL。
 * (5) `default_free_pages`:
		将页面重新链接到空闲列表中，并且可以将小的空闲块合并为大块。
 *  (5.1)
		根据提取块的基地址，在空闲列表中搜索其正确位置（地址从低到高），插入页面。
		（可以使用'list_next`、`le2page`、`list_add_before`）
 *  (5.2)
 *      重置页面的字段，例如“p->ref”和“p->flags”（PageProperty） 
 *  (5.3)
 *      尝试合并较低或较高地址的块。注意：这将正确地更改某些页的“p->property”。 
 */
free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

static void
default_init(void) {
    list_init(&free_list);
    nr_free = 0;
}

static void
default_init_memmap(struct Page *base, size_t n) {//基地址，页面数
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
		assert(PageReserved(p));			//确保可以被保留，可用
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    nr_free += n;
    list_add(&free_list, &(base->page_link));	//地址从低到高排列；list_add_before：地址从高到地排列 有序就行
}

static struct Page *
default_alloc_pages(size_t n) {
    assert(n > 0);				//分配的大小必须大于0
    if (n > nr_free) {			//如果超过最大可用，分配不了，返回
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {//双向链表，没回到起点
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {					//找到的第一个块大于需要的，就是它了，跳出
            page = p;
            break;
        }
    }
	
    if (page != NULL) {							//找到
         if (page->property > n) {
            struct Page *p = page + n;			//要的块
            p->property = page->property - n;	
			SetPageProperty(p);
            list_add(&free_list, &(p->page_link));
		}
		list_del(&(page->page_link));			//应该先做操作，然后再从链表上拿下了
		
        nr_free -= n;							//分配n个，空闲块的总数-n
        ClearPageProperty(page);
    }
    return page;
}

static void
default_free_pages(struct Page *base, size_t n) {
    assert(n > 0);								
    struct Page *p = base;
    for (; p != base + n; p ++) {//清理操作
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    list_entry_t *le = list_next(&free_list);
    while (le != &free_list) {
        p = le2page(le, page_link);
        le = list_next(le);
        if (base + base->property == p) {
            base->property += p->property;
            ClearPageProperty(p);
            list_del(&(p->page_link));
        }
        else if (p + p->property == base) {
            p->property += base->property;
            ClearPageProperty(base);
            base = p;
            list_del(&(p->page_link));
        }
    }
    nr_free += n;
    le = list_next(&free_list);
	while(le != &free_list){
		p = le2page(le,page_link);
		if(base + base->property <= p){
			assert(base + base->property != p);
			break;
		}
		le = list_next(le);
	}
	list_add_before(le,&(base->page_link));
}

static size_t
default_nr_free_pages(void) {
    return nr_free;
}

static void
basic_check(void) {
    struct Page *p0, *p1, *p2;
    p0 = p1 = p2 = NULL;
    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(p0 != p1 && p0 != p2 && p1 != p2);
    assert(page_ref(p0) == 0 && page_ref(p1) == 0 && page_ref(p2) == 0);

    assert(page2pa(p0) < npage * PGSIZE);
    assert(page2pa(p1) < npage * PGSIZE);
    assert(page2pa(p2) < npage * PGSIZE);

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));

    unsigned int nr_free_store = nr_free;
    nr_free = 0;

    assert(alloc_page() == NULL);

    free_page(p0);
    free_page(p1);
    free_page(p2);
    assert(nr_free == 3);

    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(alloc_page() == NULL);

    free_page(p0);
    assert(!list_empty(&free_list));

    struct Page *p;
    assert((p = alloc_page()) == p0);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    free_list = free_list_store;
    nr_free = nr_free_store;

    free_page(p);
    free_page(p1);
    free_page(p2);
}

// LAB2: below code is used to check the first fit allocation algorithm (your EXERCISE 1) 
// NOTICE: You SHOULD NOT CHANGE basic_check, default_check functions!
static void
default_check(void) {
    int count = 0, total = 0;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        assert(PageProperty(p));
        count ++, total += p->property;
    }
    assert(total == nr_free_pages());

    basic_check();

    struct Page *p0 = alloc_pages(5), *p1, *p2;
    assert(p0 != NULL);
    assert(!PageProperty(p0));

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));
    assert(alloc_page() == NULL);

    unsigned int nr_free_store = nr_free;
    nr_free = 0;

    free_pages(p0 + 2, 3);
    assert(alloc_pages(4) == NULL);
    assert(PageProperty(p0 + 2) && p0[2].property == 3);
    assert((p1 = alloc_pages(3)) != NULL);
    assert(alloc_page() == NULL);
    assert(p0 + 2 == p1);

    p2 = p0 + 1;
    free_page(p0);
    free_pages(p1, 3);
    assert(PageProperty(p0) && p0->property == 1);
    assert(PageProperty(p1) && p1->property == 3);

    assert((p0 = alloc_page()) == p2 - 1);
    free_page(p0);
    assert((p0 = alloc_pages(2)) == p2 + 1);

    free_pages(p0, 2);
    free_page(p2);

    assert((p0 = alloc_pages(5)) != NULL);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    nr_free = nr_free_store;

    free_list = free_list_store;
    free_pages(p0, 5);

    le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        assert(le->next->prev == le && le->prev->next == le);
        struct Page *p = le2page(le, page_link);
        count --, total -= p->property;
    }
    assert(count == 0);
    assert(total == 0);
}

const struct pmm_manager default_pmm_manager = {
    .name = "default_pmm_manager",
    .init = default_init,
    .init_memmap = default_init_memmap,
    .alloc_pages = default_alloc_pages,
    .free_pages = default_free_pages,
    .nr_free_pages = default_nr_free_pages,
    .check = default_check,
};

