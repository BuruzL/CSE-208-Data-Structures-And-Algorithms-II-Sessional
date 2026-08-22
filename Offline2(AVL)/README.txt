CSE 208 - DSA II Sessional
Assignment 2: AVL Tree and Interval Scheduler
Student ID: 2405118


AVL TREE

Compilation:
g++ -std=c++17 -O2 AVLTree.cpp -o AVLTree

Run:
./AVLTree testcase_avl.txt output_avl.txt

Expected output file:
expected_output_avl.txt


INTERVAL SCHEDULER

Compilation:
g++ -std=c++17 -O2 IntervalScheduler.cpp -o IntervalScheduler

Run with basic testcase:
./IntervalScheduler testcase_basic_interval.txt output_basic.txt

Run with edge testcase:
./IntervalScheduler testcase_edge_interval.txt output_edge.txt

Run with large testcase:
./IntervalScheduler testcase_large_interval.txt output_large.txt

Expected output files:
expected_output_interval_basic.txt
expected_output_interval_edge.txt
expected_output_interval_large.txt


TIMING REPORT

Both programs print timing statistics to the console in this format:

operation,count,total_ns,average_ns

To save the timing statistics:

AVL Tree:
./AVLTree testcase_avl.txt output_avl.txt > avl_timing.txt

Interval Scheduler:
./IntervalScheduler testcase_large_interval.txt output_large.txt > interval_timing.txt

The timing results are provided in timing_report.txt.