#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define MAX_SHM_REGIONS 64

struct shm_region {
  int key;
  void* pa;
  int ref_count;
  int active;
};

struct {
  struct spinlock lock;
  struct shm_region regions[MAX_SHM_REGIONS];
} shm_table;

void
shminit(void)
{
  initlock(&shm_table.lock, "shm_table");
  for(int i = 0; i < MAX_SHM_REGIONS; i++) {
    shm_table.regions[i].active = 0;
  }
}

int
shm_create(int key)
{
  struct shm_region *r = 0;
  void *pa;

  acquire(&shm_table.lock);
  
  // Check if key already exists
  for(int i = 0; i < MAX_SHM_REGIONS; i++) {
    if(shm_table.regions[i].active && shm_table.regions[i].key == key) {
      release(&shm_table.lock);
      return i;
    }
  }

  // Find free slot
  for(int i = 0; i < MAX_SHM_REGIONS; i++) {
    if(!shm_table.regions[i].active) {
      r = &shm_table.regions[i];
      break;
    }
  }

  if(r == 0) {
    release(&shm_table.lock);
    return -1;
  }

  // Allocate physical page
  pa = kalloc();
  memset(pa, 0, PGSIZE);

  if(pa == 0) {
    release(&shm_table.lock);
    return -1;
  }

  r->key = key;
  r->pa = pa;
  r->ref_count = 0;
  r->active = 1;

  release(&shm_table.lock);
  return r - shm_table.regions;
}

void*
shm_get(int key)
{
  struct shm_region *r = 0;
  struct proc *p = myproc();
  void *va;

  acquire(&shm_table.lock);
  
  // Find region by key
  for(int i = 0; i < MAX_SHM_REGIONS; i++) {
    if(shm_table.regions[i].active && shm_table.regions[i].key == key) {
      r = &shm_table.regions[i];
      break;
    }
  }

  if(r == 0) {
    release(&shm_table.lock);
    return 0;
  }

  // If this process already attached, return the same VA (avoid duplicate mapping)
  for (int i = 0; i < SHM_ATTACH_MAX; i++) {
    if (p->shm_attach[i].key == key) {
      va = (void*) p->shm_attach[i].va;
      release(&shm_table.lock);
      return va;
    }
  }

  r->ref_count++;
  release(&shm_table.lock);

  int slot = -1;
  for (int i = 0; i < SHM_ATTACH_MAX; i++) {
    if (p->shm_attach[i].key == 0){ 
        slot = i; break; 
    }
  }

  if (slot < 0) {
    // rollback ref_count
    acquire(&shm_table.lock);
    r->ref_count--;
    release(&shm_table.lock);
    return 0;
  }

  // Find free virtual address in user space
  va = (void*)PGROUNDUP(p->sz);
  if ((uint64)va + PGSIZE > MAXVA) {
    acquire(&shm_table.lock);
    r->ref_count--;
    release(&shm_table.lock);
    return 0;
  }

  // Map page into process address space
  if(mappages(p->pagetable, (uint64)va, PGSIZE, (uint64)r->pa, PTE_W|PTE_R|PTE_U) < 0) {
    acquire(&shm_table.lock);
    r->ref_count--;
    release(&shm_table.lock);
    return 0;
  }

  // record attachment in proc
  p->shm_attach[slot].key = key;
  p->shm_attach[slot].va  = (uint64)va;

  p->sz = (uint64)va + PGSIZE;
  return va;
}

int
shm_close(int key)
{
  struct shm_region *r = 0;
  struct proc *p = myproc();
  int slot = -1;

  // find the process attachment slot
  for (int i = 0; i < SHM_ATTACH_MAX; i++) {
    if (p->shm_attach[i].key == key){
        slot = i; 
        break; 
    }
  }
  if (slot < 0) {
    return -1;
  }

  uint64 va = p->shm_attach[slot].va;

  // Unmap the page from this process address space
  uvmunmap(p->pagetable, va, 1, 0);

  // clear the proc attachment slot
  p->shm_attach[slot].key = 0;
  p->shm_attach[slot].va  = 0;

  
  acquire(&shm_table.lock);
  for (int i = 0; i < MAX_SHM_REGIONS; i++) {
    if (shm_table.regions[i].active && shm_table.regions[i].key == key) {
      r = &shm_table.regions[i];
      break;
    }
  }

  if (r == 0) {
    release(&shm_table.lock);
    return -1;
  }

  r->ref_count--;
  if (r->ref_count <= 0) {
    kfree(r->pa);
    r->pa = 0;
    r->key = 0;
    r->ref_count = 0;
    r->active = 0;
  }

  release(&shm_table.lock);
  return 0;
}
