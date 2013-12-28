lwIP Optimization
====================

This is a course project code repo, in which we profile lwIP and find its
bottlenecks to do optimization.

Two optimization has been made to lwIP to make it more efficient:

1. Use hash table to replace the linear linked list to speed up the pcb lookup.
2. Use assembly language to rewrite the computation of checksum algorithm.
