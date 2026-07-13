# Simple File System Simulation

A menu-driven C program that simulates basic file system operations — create, write, read, delete, rename, and list — using an in-memory file table plus real disk I/O (`fopen`, `fwrite`, `fread`, `remove`, `rename`).

## Build

```bash
gcc -Wall -Wextra -o filesystem_sim filesystem_sim.c
```

## Run

```bash
./filesystem_sim
```

## Menu

1. Create File  
2. Write to File  
3. Read File  
4. Delete File  
5. List Files  
6. Rename File  
7. Exit  

## Skills demonstrated

File I/O, structs, pointers, error handling, and dynamic memory allocation.
