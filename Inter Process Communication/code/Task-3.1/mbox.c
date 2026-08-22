#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define MAX_MAILBOXES 64
#define MBOX_SIZE 16

struct mailbox {
  int key;
  int active;
  int messages[MBOX_SIZE];
  int head;
  int tail;
  int count;
  struct spinlock lock;
};

struct {
  struct spinlock lock;
  struct mailbox mboxes[MAX_MAILBOXES];
} mbox_table;

void
mboxinit(void)
{
  initlock(&mbox_table.lock, "mbox_table");
  for(int i = 0; i < MAX_MAILBOXES; i++) {
    mbox_table.mboxes[i].active = 0;
    initlock(&mbox_table.mboxes[i].lock, "mailbox");
  }
}

int
mbox_create(int key)
{
  struct mailbox *mb = 0;

  acquire(&mbox_table.lock);
  
  // Check if key already exists
  for(int i = 0; i < MAX_MAILBOXES; i++) {
    if(mbox_table.mboxes[i].active && mbox_table.mboxes[i].key == key) {
      release(&mbox_table.lock);
      return i;
    }
  }

  // Find free slot
  for(int i = 0; i < MAX_MAILBOXES; i++) {
    if(!mbox_table.mboxes[i].active) {
      mb = &mbox_table.mboxes[i];
      break;
    }
  }

  if(mb == 0) {
    release(&mbox_table.lock);
    return -1;
  }

  mb->key = key;
  mb->active = 1;
  mb->head = 0;
  mb->tail = 0;
  mb->count = 0;

  release(&mbox_table.lock);
  return mb - mbox_table.mboxes;
}

int
mbox_send(int mbox_id, int msg)
{
  struct mailbox *mb;

  if(mbox_id < 0 || mbox_id >= MAX_MAILBOXES)
    return -1;

  mb = &mbox_table.mboxes[mbox_id];
  if(!mb->active)
    return -1;

  acquire(&mb->lock);
  
  // Block while mailbox is full
  while(mb->count == MBOX_SIZE) {
    sleep(mb, &mb->lock);
  }

  mb->messages[mb->tail] = msg;
  mb->tail = (mb->tail + 1) % MBOX_SIZE;
  mb->count++;
  
  printf("mbox_send: %d to mbox %d\n", msg, mbox_id);
  wakeup(mb);


  release(&mb->lock);
  return 0;
}

int
mbox_recv(int mbox_id, int *msg)
{
  struct mailbox *mb;
  struct proc *p = myproc();

  if(mbox_id < 0 || mbox_id >= MAX_MAILBOXES)
    return -1;

  mb = &mbox_table.mboxes[mbox_id];
  if(!mb->active)
    return -1;

  acquire(&mb->lock);
  
  // Block while mailbox is empty
  while(mb->count == 0) {
    sleep(mb, &mb->lock);
  }

  int message = mb->messages[mb->head];
  mb->head = (mb->head + 1) % MBOX_SIZE;
  mb->count--;

  printf("mbox_recv: %d from mbox %d\n", message, mbox_id);
  // Copy message to user space
  if(copyout(p->pagetable, (uint64)msg, (char*)&message, sizeof(int)) < 0) {
    release(&mb->lock);
    return -1;
  }

  wakeup(mb);


  release(&mb->lock);
  return 0;
}