#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main() {
  int key = 100;

  if (shm_create(key) < 0) {
    printf("shmtest: create failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid == 0) {
    // Child
    int *arr_alias = (int*) shm_get(key);
    
    if ((uint64)arr_alias == (uint64)0) {
      printf("child get fail\n");
      exit(1);
    }

    pause(100); // allow parent to modify
    printf("child saw: %d\n", arr_alias[0]);

    arr_alias[1] = 4;
    shm_close(key);
    exit(0);

  } else if (pid > 0) {
    // Parent
    int *arr = (int*) shm_get(key);
    if ((uint64)arr == (uint64)0) {
        printf("shmtest: first get failed\n");
        exit(1);
    }
    arr[0] = 12;
    wait(0);
    printf("parent after child: %d\n", arr[1]);

    shm_close(key);
  } else {
    printf("fork failed\n");
  }

  exit(0);
}
