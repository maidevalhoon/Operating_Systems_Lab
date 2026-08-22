# Inter-Process Communication

This section explores how independent processes can communicate and synchronize their actions. By default, xv6 only supports basic IPC through pipes, which are limited to unidirectional byte streams between related processes. This project introduces more sophisticated IPC paradigms to the xv6 kernel.

## What is Implemented

### 1. Shared Memory
A subsystem that allows multiple processes to map the same physical memory page into their respective virtual address spaces.
- Enables direct, high-speed data access and exchange without the overhead of kernel-mediated copying.
- Managed via a centralized table with reference counting and appropriate locking mechanisms for safe concurrent access.

### 2. Mailboxes
A structured message-passing mechanism for asynchronous communication.
- Provides a reliable way for processes to send and receive discrete messages using a bounded circular buffer.
- Features blocking behavior, ensuring that sending processes wait if a mailbox is full, and receiving processes wait if a mailbox is empty.

### 3. The Intertwined Memory Challenge
An application-level demonstration utilizing the newly implemented IPC primitives.
- Multiple worker processes coordinate to traverse a shared path.
- Demonstrates process cooperation, using shared memory for data representation and mailboxes for strict synchronization and turn-taking.

For detailed information regarding the implementation of the Shared Memory Manager, Mailbox Manager, and synchronization locks, please consult the included Report.
