# Deleted file recovery

* get all file handles on partition(crazy slow, this could be made async + thread pool for each directory. Supposedly even for a 1 threaded approach, the file handles wouldn't be in strictly increasing order of the memory location, so no loss here)

* sort the file handles(could be slow again, might have to look into a parallelized quicksort)

* use FSCTL_GET_RETRIEVAL_POINTERS to get the location on disk of these files

* obtain the empty space between two consecutive files and check it for deleted files

- some special considerations might exist for SSD

- partition needs to be taken offline first so that it's not modified while the program runs(doable through command prompt)

- Windows 10 often uses non-contiguous memory allocation(separate blocks scattered throughout the memory, this increases the complexity of finding the free memory
blocks and also of analyzing for deleted files)

how to parallelize it?
======================
1) get all file HANDLEs
2) for each file, 'map' it to the nr of free bytes since the previous file(can be parallelized): free_bytes_since_last[i]
3) based on above step, create a struct FreeMemoryBlock for each block between two consecutive files
   * the struct will contain beginning and ending memory offset, and the size of the block

4) call GetDiskFreeSpaceExW to get the amount of free memory

5) load balance the FreeMemoryBlock array based on the number of threads allocated and the size of each block
   (try to get a roughly equal number of bytes on each thread)

6) - look for file signatures in each FreeMemoryBlock
   - if not corrupted, recreate and save to a different partition

* set partition back online

