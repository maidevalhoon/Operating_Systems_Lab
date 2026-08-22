#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int
main(void)
{
  int key1 = 5678;
  int key2 = 1234;
  int a = mbox_create(key1);
  int b = mbox_create(key2);

  int pid = fork();

  if (pid == 0) {
    // child: receiver
    for(int i = 0; i < 10; i++) {
        int msg = -1;
        mbox_recv(a, &msg);
        int oo = i;
        mbox_send(b, oo);
    }
    exit(0);
  } else {
    int s = 100;
    for(int i = 0; i < 10; i++) {
      s++;
      mbox_send(a, s);
      int r = -1;
      mbox_recv(b, &r);
    }
    wait(0);
    exit(0);
  }
}