# File System

This section aims to expand the capabilities and overcome the limitations of the standard xv6 file system architecture.

## What is Implemented

### 1. Doubly-Indirect Block Support
An enhancement to the file system's inode structure to significantly increase the maximum allowable file size.
- The standard xv6 inode limits file size to approximately 268 KB using direct and singly-indirect pointers.
- This implementation modifies the inode to support a doubly-indirect block pointer, increasing the maximum addressable blocks.
- Elevates the maximum file size limit from 268 KB to approximately 64 MB, adapting the block mapping and file truncation logic to handle the new hierarchical structure.

### 2. Symbolic Links (Soft Links)
The introduction of a new file type that points to another file or directory by its path.
- Overcomes the limitations of hard links, which cannot point to directories or span across different file systems.
- Implements a new system call to create symbolic links and modifies the path resolution process to correctly follow these links.
- Includes mechanisms to detect and prevent infinite recursion caused by circular symbolic links.

For detailed explanations of the structural changes to inodes, recursive block freeing logic, and the symbolic link path resolution implementation, please see the included Report.
