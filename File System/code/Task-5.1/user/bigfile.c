#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// --- Configuration ---
// Use const for typed constants for better type-safety.
const char* FILENAME = "bigfile";
const int BLOCK_SIZE = 1024;
const int NUM_BLOCKS = 3000;
const int PROGRESS_INTERVAL = 500;

/**
 * @brief Generates a deterministic data pattern for a given block.
 * This helper function eliminates code duplication between write and verify.
 * @param buffer The buffer to fill with data.
 * @param block_num The block number, used to create a unique pattern.
 */
void generate_block_data(char* buffer, int block_num) {
  for (int i = 0; i < BLOCK_SIZE; i++) {
    // The pattern is a simple function of the block number and byte index.
    buffer[i] = (block_num + i) % 256;
  }
}

/**
 * @brief Creates and writes a large file with a predictable pattern.
 */
void write_bigfile() {
  int fd = open(FILENAME, O_CREATE | O_WRONLY);
  if (fd < 0) {
    // Print errors to standard error (file descriptor 2).
    fprintf(2, "bigfile: failed to create file %s\n", FILENAME);
    exit(1);
  }

  printf("bigfile: writing %d blocks to %s...\n", NUM_BLOCKS, FILENAME);

  char buffer[BLOCK_SIZE];
  for (int i = 0; i < NUM_BLOCKS; i++) {
    // Generate the data for the current block.
    generate_block_data(buffer, i);

    if (write(fd, buffer, BLOCK_SIZE) != BLOCK_SIZE) {
      fprintf(2, "bigfile: write failed at block %d\n", i);
      close(fd);
      exit(1);
    }

    if (i > 0 && i % PROGRESS_INTERVAL == 0) {
      printf("bigfile: written %d/%d blocks\n", i, NUM_BLOCKS);
    }
  }

  printf("bigfile: successfully wrote %d blocks\n", NUM_BLOCKS);
  close(fd);
}

/**
 * @brief Reads the large file and verifies its contents against the expected pattern.
 */
void verify_bigfile() {
  int fd = open(FILENAME, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "bigfile: failed to open file %s for reading\n", FILENAME);
    exit(1);
  }

  printf("bigfile: verifying %d blocks in %s...\n", NUM_BLOCKS, FILENAME);

  char read_buffer[BLOCK_SIZE];
  char expected_buffer[BLOCK_SIZE];
  for (int i = 0; i < NUM_BLOCKS; i++) {
    // Generate the expected data for this block.
    generate_block_data(expected_buffer, i);

    int bytes_read = read(fd, read_buffer, BLOCK_SIZE);
    if (bytes_read != BLOCK_SIZE) {
      fprintf(2, "bigfile: read failed at block %d (read %d bytes)\n", i, bytes_read);
      close(fd);
      exit(1);
    }

    if (memcmp(read_buffer, expected_buffer, BLOCK_SIZE) != 0) {
      fprintf(2, "bigfile: data mismatch at block %d\n", i);
      close(fd);
      exit(1);
    }

    if (i > 0 && i % PROGRESS_INTERVAL == 0) {
      printf("bigfile: verified %d/%d blocks\n", i, NUM_BLOCKS);
    }
  }

  printf("bigfile: successfully verified %d blocks\n", NUM_BLOCKS);
  close(fd);
}

int main(void) {
  printf("bigfile: starting file I/O test\n");

  write_bigfile();
  verify_bigfile();

  printf("bigfile: test completed successfully\n");
  exit(0);
}