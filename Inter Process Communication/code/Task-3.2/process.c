#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define END_MARKER -1

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: process <shm_key> <my_mbox> <other_mbox> <start_index> <role>\n");
        exit(1);
    }
    
    int shm_key = atoi(argv[1]);
    int my_mbox = atoi(argv[2]);
    int other_mbox = atoi(argv[3]);
    int current_index = atoi(argv[4]);
    char *role = argv[5];
    
    char process_name = (strcmp(role, "send_first") == 0) ? 'A' : 'B';
    
    // Attach to shared memory
    int *shm = (int*)shm_get(shm_key);
    if (shm == 0) {
        printf("Process %c: Failed to attach to shared memory\n", process_name);
        exit(1);
    }
    
    int step = 0;
    
    // Main traversal loop
    while (1) {
        if (strcmp(role, "send_first") == 0) {
            // Send current position first, then receive
            if (mbox_send(other_mbox, current_index) < 0) {
                printf("Process %c: Failed to send message\n", process_name);
                break;
            }
            
            // Receive other's position
            int received_index;
            if (mbox_recv(my_mbox, &received_index) < 0) {
                printf("Process %c: Failed to receive message\n", process_name);
                break;
            }
            
            // Look up my next index using the received index
            int next_index = shm[received_index];
            
            if (next_index == END_MARKER) {
                break;
            }
            
            current_index = next_index;
            
        } else {
            // Receive first, then send current position
            int received_index;
            if (mbox_recv(my_mbox, &received_index) < 0) {
                printf("Process %c: Failed to receive message\n", process_name);
                break;
            }
            
            if (mbox_send(other_mbox, current_index) < 0) {
                printf("Process %c: Failed to send message\n", process_name);
                break;
            }
            
            // Look up my next index using the received index  
            int next_index = shm[received_index];
            
            if (next_index == END_MARKER) {
                break;
            }
            
            current_index = next_index;
        }
        
        step++;
        
        // Safety check to prevent infinite loops
        if (step > 10) {
            printf("Process %c: Too many steps, terminating\n", process_name);
            break;
        }
    }
    
    // Cleanup
    shm_close(shm_key);
    
    exit(0);
}