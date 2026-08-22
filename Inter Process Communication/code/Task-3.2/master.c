#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SHM_KEY 1234
#define MBOX_A_TO_B_KEY 5001
#define MBOX_B_TO_A_KEY 5002
#define END_MARKER -1
#define PGSIZE 4096

/**
 * Helper function to reverse a string.
 */
void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

/**
 * Custom implementation of itoa (integer to ASCII string).
 * Converts an integer 'n' into a string and stores it in 's'.
 */
void itoa(int n, char s[]) {
    int i = 0;
    int isNegative = 0;

    // Handle the case of 0 explicitly
    if (n == 0) {
        s[i++] = '0';
        s[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (n < 0) {
        isNegative = 1;
        n = -n;
    }

    // Generate digits in reverse order
    while (n != 0) {
        int rem = n % 10;
        s[i++] = rem + '0';
        n = n / 10;
    }

    // Append the negative sign if necessary
    if (isNegative) {
        s[i++] = '-';
    }

    s[i] = '\0'; // Null-terminate the string

    // Reverse the string to get the correct order
    reverse(s, i);
}

int main() {
    printf("Master: Starting intertwined path demonstration\n");

    // Create shared memory
    int shm_id = shm_create(SHM_KEY);
    if (shm_id < 0) {
        printf("Master: Failed to create shared memory\n");
        exit(1);
    }
    printf("Master: Created shared memory with ID %d\n", shm_id);

    // Get shared memory pointer
    int *shm = (int*)shm_get(SHM_KEY);
    if (shm == 0) {
        printf("Master: Failed to attach to shared memory\n");
        exit(1);
    }
    printf("Master: Attached to shared memory\n");

    // Initialize the intertwined paths
    // A's path: 0 -> 2 -> 4 -> 6 -> END
    // B's path: 1 -> 3 -> 5 -> 7 -> END
    shm[0] = 3; 
    shm[1] = 2; 
    shm[2] = 5; 
    shm[3] = 4; 
    shm[4] = 7; 
    shm[5] = 6; 
    shm[6] = END_MARKER;
    shm[7] = END_MARKER; 
    
    printf("Master: Initialized intertwined paths.\n");

    // Create mailboxes
    int mbox_a_to_b = mbox_create(MBOX_A_TO_B_KEY);
    int mbox_b_to_a = mbox_create(MBOX_B_TO_A_KEY);

    if (mbox_a_to_b < 0 || mbox_b_to_a < 0) {
        printf("Master: Failed to create mailboxes\n");
        shm_close(SHM_KEY);
        exit(1);
    }
    
    printf("Master: Created mailboxes:\n");
    printf("First_Process (A) will receive next address in its path from mailbox: %d\n", mbox_b_to_a);
    printf("First_Process (B) will receive next address in its path from mailbox: %d\n\n", mbox_a_to_b);

    // Buffers to hold string versions of integer IDs for exec
    char shm_key_str[12];
    char mbox_a_to_b_str[12];
    char mbox_b_to_a_str[12];
    
    // Convert integer IDs to strings
    itoa(SHM_KEY, shm_key_str);
    itoa(mbox_a_to_b, mbox_a_to_b_str);
    itoa(mbox_b_to_a, mbox_b_to_a_str);

    // Fork and exec process A (send_first)
    int pid_a = fork();
    if (pid_a == 0) {
        // Child A
        // Process A sends to mbox_a_to_b and receives from mbox_b_to_a
        // args: <shm_key> <my_mbox> <other_mbox> <start> <role>
        char *args[] = {"process", shm_key_str, mbox_b_to_a_str, mbox_a_to_b_str, "0", "send_first", 0};
        exec("process", args);
        printf("Master: Failed to exec process A\n");
        exit(1);
    }

    // Fork and exec process B (recv_first) 
    int pid_b = fork();
    if (pid_b == 0) {
        // Child B
        // Process B sends to mbox_b_to_a and receives from mbox_a_to_b
        // args: <shm_key> <my_mbox> <other_mbox> <start> <role>
        char *args[] = {"process", shm_key_str, mbox_a_to_b_str, mbox_b_to_a_str, "1", "recv_first", 0};
        exec("process", args);
        printf("Master: Failed to exec process B\n");
        exit(1);
    }

    // Wait for both children to complete
    int status;
    wait(&status);
    printf("Master: First child completed\n");
    wait(&status);
    printf("Master: Second child completed\n");

    // Cleanup
    shm_close(SHM_KEY);
    printf("Master: All processes completed successfully\n");

    exit(0);
}