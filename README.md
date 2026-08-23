# Understanding the Xv6 Architecture

This repository is a hands-on exploration of the **Xv6** operating system — a small, clean teaching OS originally built at MIT. The goal was never just to read about how operating systems work, but to actually get inside one and change things.

This work is part of **CS-343 (Operating Systems)**, a course at the **Indian Institute of Technology, Guwahati**. Each folder here corresponds to a topic from the course where we went beyond theory and wrote real kernel code.

---

## What we worked on

### [Thread and Synchronization](./Thread%20and%20Synchronization/)
Xv6 does not have threads by default, so we built them. Kernel-level threads that share an address space, along with spinlocks to keep shared data safe when multiple threads are running at the same time.

### [Scheduling](./Scheduling/)
The default Xv6 scheduler treats every process equally, regardless of how long it will run. We added system calls to expose process information and used that to implement a **Shortest Job First** scheduler and a **Hybrid** scheduling policy.

### [Memory Management](./Memory%20Management/)
Two things real operating systems do that Xv6 skips: **lazy memory allocation** (don't actually allocate until the process touches the memory) and **page swapping** (move pages to disk when RAM runs low, bring them back when needed).

### [Inter-Process Communication](./Inter%20Process%20Communication/)
Pipes only go so far. We added **shared memory** — letting multiple processes map the same physical page — and **mailboxes**, a blocking message-passing system for asynchronous communication between processes.

### [File System](./File%20System/)
Xv6's file system caps file size at around 268 KB. We raised that to roughly 64 MB by adding doubly-indirect block pointers to the inode structure. We also implemented **symbolic links**, a file type that holds a path instead of data, complete with loop detection for circular links.

---

## How to navigate this repo

Each folder has its own `Readme.md` that explains the implementation in detail. For the more involved modules, there is also a PDF lab report with analysis and results. Start there if you want to understand how something was built.

---

*This is a learning project. The code is meant to be read alongside the Xv6 source, not as a standalone system.*
