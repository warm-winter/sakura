#include <pmm.h>
#include <list.h>
#include <string.h>
#include <default_pmm.h>

/* 
��First-Fit�㷨�У�����������һ�����п���б���Ϊ�����б���
һ���յ��ڴ�����������ͻ������б�ɨ���һ���㹻��Ŀ�����������
�����ѡ�Ŀ������Ĵ�ö࣬ͨ���Ὣ���֣�
ʣ��Ŀ齫��Ϊ��һ�����п���ӵ��б��У�
���вμ���ά�����ݽṹ����C����������ԡ�һ���196~198ҳ��8.2�ڡ� 
*/
// LAB2 EXERCISE 1: YOUR CODE
// you should rewrite functions: `default_init`, `default_init_memmap`,
// `default_alloc_pages`, `default_free_pages`.
/*
 * Details of FFMA
 * (1) Preparation:
	Ϊ��ʵ��First-Fit�ڴ���䣨FFMA��������Ӧ��ʹ��һ���б�����������ڴ�顣
	��free_area_t���ṹ���ڹ�������ڴ�顣
	
	���ȣ���Ӧ����Ϥlist.h�еĽṹlist��
	�ṹlist��һ���򵥵�˫�������б�ʵ�֡�
	��Ӧ��֪�����ʹ��list_init��list_add��list_add_after����list_add_before��list_del��list_next��list_prev��
	
	��һ�����ֵķ�����ʹ�����º꽫һ���list�ṹת��Ϊ����Ľṹ������struct page����
	ʹ�����º꣺le2page����memlayout.h�У���(�������Ժ��lab��le2vma����vmm.h�У���le2proc��proc.h�У��ȡ� ��
	
	��2��default_init��
	����������ʹ��demo default_init��������ʼ��free_list��������nr_free������Ϊ0��
	free_list���ڼ�¼�����ڴ�顣nr_free�ǿ����ڴ���������
	
	(3) `default_init_memmap`:
	����ͼ: `kern_init` --> `pmm_init` --> `page_init` --> `init_memmap` --> `pmm_manager` --> `init_memmap`.
	�˺������ڳ�ʼ�����п飨ʹ�ò���'addr_base`��`page_number`����
	Ϊ�˳�ʼ��һ�����п飬����Ӧ�ó�ʼ��������ɿ��е�ÿ��ҳ�棨��memlayout.h�ж��壩��
	�ó��������
	�������á�p->flags���ġ�PG_property��λ������ζ�Ŵ�ҳ��Ч��
	�ں�����pmm_init������pmm.c�У��У���P->flags���ġ�PG_reserved��λ�Ѿ����ú��ˡ�
	
	- �����ҳ�ǿ���ҳ�����Ҳ��ǿ��п�ĵ�һҳ����p->property��Ӧ����Ϊ0��
	- �����ҳ�ǿ���ҳ�������ǿ��п�ĵ�һҳ����p->property��Ӧ����Ϊ���е���ҳ����
	- `p->ref`Ӧ����0����Ϊ����'p'�����ɵģ�û�����á�
	
	Ȼ�����ǿ���ʹ�á�p->page_link������ҳ�����ӵ���free_list��
	�����磺list_add_before��&free_list��&��p->page_link�������� 
	
	�������Ӧ�ø��¿����ڴ����ܺͣ�nr_free+=n��
	
	(4) `default_alloc_pages`:
	�ڿ����б���������һ�����п飨���С>=n�������������ҵ��Ŀ飬���˿�ĵ�ַ����Ϊ��malloc������ĵ�ַ��
 *  (4.1)
 *      ������Ӧ�����������������б�:
 *          list_entry_t le = &free_list;
 *          while((le=list_next(le)) != &free_list) {
 *          ...
        (4.1.1)
		��whileѭ���У���ȡ�ṹpage������顰p->property������¼�˿��еĿ���ҳ����>=n��
                struct Page *p = le2page(le, page_link);
                if(p->property >= n){ ...
 
        (4.1.2)
		��������ҵ����'p'������ζ�������ҵ���һ����С>=n�Ŀ��п飬
		���ĵ�һ��'n'ҳ���Ա�malloce������ҳ��ĳЩ��־λӦ����Ϊ����PG_reserved=1����`PG_property=0`��
		Ȼ��ȡ��ҳ���롰free_list�������ӡ�
		
 *          (4.1.2.1)
 *              �����p->property>n������Ӧ���¼���˿��п��ʣ��ҳ����
			�����磺 `le2page(le,page_link))->property
 *          = p->property - n;`)
 *          (4.1.3)
 *              ���¼���`nr_free`�����п��ÿ�����ಿ�ֵı�ţ���
 *          (4.1.4)
 *              return `p`.
 *      (4.2)
 *          ����Ҳ�����С���ڵ���n�Ŀ��п飬�򷵻�NULL��
 * (5) `default_free_pages`:
		��ҳ���������ӵ������б��У����ҿ��Խ�С�Ŀ��п�ϲ�Ϊ��顣
 *  (5.1)
		������ȡ��Ļ���ַ���ڿ����б�����������ȷλ�ã���ַ�ӵ͵��ߣ�������ҳ�档
		������ʹ��'list_next`��`le2page`��`list_add_before`��
 *  (5.2)
 *      ����ҳ����ֶΣ����硰p->ref���͡�p->flags����PageProperty�� 
 *  (5.3)
 *      ���Ժϲ��ϵͻ�ϸߵ�ַ�Ŀ顣ע�⣺�⽫��ȷ�ظ���ĳЩҳ�ġ�p->property���� 
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
default_init_memmap(struct Page *base, size_t n) {//����ַ��ҳ����
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
		assert(PageReserved(p));			//ȷ�����Ա�����������
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    nr_free += n;
    list_add(&free_list, &(base->page_link));	//��ַ�ӵ͵������У�list_add_before����ַ�Ӹߵ������� �������
}

static struct Page *
default_alloc_pages(size_t n) {
    assert(n > 0);				//����Ĵ�С�������0
    if (n > nr_free) {			//������������ã����䲻�ˣ�����
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {//˫������û�ص����
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {					//�ҵ��ĵ�һ���������Ҫ�ģ��������ˣ�����
            page = p;
            break;
        }
    }
	
    if (page != NULL) {							//�ҵ�
         if (page->property > n) {
            struct Page *p = page + n;			//Ҫ�Ŀ�
            p->property = page->property - n;	
			SetPageProperty(p);
            list_add(&free_list, &(p->page_link));
		}
		list_del(&(page->page_link));			//Ӧ������������Ȼ���ٴ�������������
		
        nr_free -= n;							//����n�������п������-n
        ClearPageProperty(page);
    }
    return page;
}

static void
default_free_pages(struct Page *base, size_t n) {
    assert(n > 0);								
    struct Page *p = base;
    for (; p != base + n; p ++) {//�������
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

