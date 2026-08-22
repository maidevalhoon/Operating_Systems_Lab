// user/symlinktest.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(void)
{
  int fd;
  char buf[200];
  int n;

  printf("Starting symlinktest...\n\n");

  // create a regular file t.txt
  printf("--- Test 1: Basic File and Symlink Creation ---\n");
  printf("Attempting to create and write to a regular file 't.txt'...\n");
  fd = open("t.txt", O_CREATE | O_RDWR);
  if(fd < 0){
    printf("ERROR: Failed to create t.txt\n");
    exit(1);
  }
  write(fd, "hello\n", 6);
  close(fd);
  printf("SUCCESS: 't.txt' created and written to successfully.\n\n");

  // create a symlink link1 -> t.txt
  printf("Attempting to create a symbolic link 'link1' that points to 't.txt'...\n");
  if (symlink("t.txt", "link1") < 0) {
    printf("ERROR: symlink 'link1' -> 't.txt' failed\n");
    exit(1);
  }
  printf("SUCCESS: Symbolic link 'link1' created.\n\n");

  // open link1 (should follow to t.txt)
  printf("--- Test 2: Following a Symbolic Link ---\n");
  printf("Opening 'link1' without O_NOFOLLOW. This should open the target file 't.txt'.\n");
  fd = open("link1", O_RDONLY);
  if (fd < 0) {
    printf("ERROR: open('link1') failed\n");
    exit(1);
  }
  n = read(fd, buf, sizeof(buf)-1);
  if(n < 0) {
    printf("ERROR: read from 'link1' failed\n");
    exit(1);
  }
  buf[n] = 0;
  printf("SUCCESS: Read from followed link 'link1'. Content: '%s'", buf);
  close(fd);
  printf("------------------------------------------------\n\n");


  // open link1 with O_NOFOLLOW should open the symlink file itself
  printf("--- Test 3: Opening a Symlink with O_NOFOLLOW ---\n");
  printf("Opening 'link1' with O_NOFOLLOW. This should open the link itself, not the target.\n");
  fd = open("link1", O_RDONLY | O_NOFOLLOW);
  if (fd < 0) {
    printf("ERROR: open('link1', O_NOFOLLOW) failed\n");
    exit(1);
  }
  printf("Reading the content of the symlink file...\n");
  n = read(fd, buf, sizeof(buf)-1);
  if(n < 0) {
    printf("ERROR: read from 'link1' with O_NOFOLLOW failed\n");
    exit(1);
  }
  buf[n] = 0;
  printf("SUCCESS: The content of the symlink is its target path: '%s'\n", buf);
  close(fd);
  printf("--------------------------------------------------\n\n");

  // create a symlink loop: a -> b, b -> a
  printf("--- Test 4: Symbolic Link Loop Detection ---\n");
  printf("Creating a symlink loop: 'a' -> 'b'...\n");
  if (symlink("b", "a") < 0) {
    printf("ERROR: symlink 'a' -> 'b' failed\n");
    exit(1);
  }
  printf("Creating a symlink loop: 'b' -> 'a'...\n");
  if (symlink("a", "b") < 0) {
    printf("ERROR: symlink 'b' -> 'a' failed\n");
    exit(1);
  }
  printf("Link loop 'a' <-> 'b' created successfully.\n\n");


  // Try to open "a" (should fail due to recursion limit)
  printf("Attempting to open 'a'. The kernel should detect the loop and prevent opening.\n");
  fd = open("a", O_RDONLY);
  if (fd >= 0) {
    printf("FAILURE: Opened looped symlink 'a' unexpectedly. This should not happen.\n");
    exit(1);
  } else {
    printf("SUCCESS: As expected, open('a') failed, preventing an infinite loop.\n");
  }
  printf("-------------------------------------------\n\n");

  printf("symlinktest finished successfully!\n");
  exit(0);
}