// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define PGCNT (1L << 15)

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run
{
  struct run *next;
};

struct
{
  struct spinlock lock;
  struct run *freelist;
  uint64 refcnt[PGCNT];
} kmem;

void kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void *)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

uint64 incRefcnt(void *pa)
{
  acquire(&kmem.lock);
  int pageidx = ((uint64)pa - (uint64)end) >> PGSHIFT;
  printf(" 1 %d %d %d/%d\n", (uint64)pa, (uint64)end, pageidx, PGCNT);

  if (pageidx < 0 || pageidx >= PGCNT)
  {
    printf(" 1 pageidx error : %d %d\n", pageidx, PGCNT);
    printf(" %d %d %d\n", (uint64)pa, (uint64)end, pageidx);

    release(&kmem.lock);

    return 0;
  }
  kmem.refcnt[pageidx]++;
  printf(" 1 pageidx = %d / refcnt : %d\n", pageidx, kmem.refcnt[pageidx]);
  release(&kmem.lock);
  return 0;
}

uint64 decRefcnt(void *pa)
{
  acquire(&kmem.lock);
  int pageidx = ((uint64)pa - (uint64)end) >> PGSHIFT;
  printf("-1 %d %d %d/%d\n", (uint64)pa, (uint64)end, pageidx, PGCNT);

  if (pageidx < 0 || pageidx >= PGCNT)
  {
    printf("-1 pageidx error : %d %d\n", pageidx, PGCNT);
    printf(" %d %d %d\n", (uint64)pa, (uint64)end, pageidx);

    release(&kmem.lock);

    return 0;
  }
  if (kmem.refcnt[pageidx] == 0)
  {
    printf("already freed\n");
    release(&kmem.lock);
    return -1;
  }
  kmem.refcnt[pageidx]--;
  printf("-1 pageidx = %d / refcnt : %d\n", pageidx, kmem.refcnt[pageidx]);
  release(&kmem.lock);
  return kmem.refcnt[pageidx];
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  int ref = decRefcnt(pa);
  release(&kmem.lock);
  if (ref > 0)
    return;

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r)
    kmem.freelist = r->next;
  int pageidx = ((uint64)r - (uint64)end) >> PGSHIFT;
  if (pageidx >= 0 && pageidx < PGCNT)
    kmem.refcnt[pageidx] = 1;
  release(&kmem.lock);

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}
