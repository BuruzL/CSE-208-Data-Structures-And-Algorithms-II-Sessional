BINOMIAL MIN HEAP AND VISUALIZER

Student Name: Lamia Buruz
Student ID: 2405118

FILES
BinomialHeap.cpp - Contains Part A and Part B implementation.
input.txt - Contains the input commands.
output.txt - Contains the required Part A output.

COMPILATION
g++ BinomialHeap.cpp -o heap

RUN COMMAND
Windows:
.\heap.exe


FILE INPUT AND OUTPUT
The program automatically reads commands from input.txt.
All required Part A output is displayed in the terminal and also
written to output.txt.

SUPPORTED PART A COMMANDS
I h x     Insert x into heap h.
F h       Find and return the minimum key.
E h       Extract and return the minimum key.
D h x y   Decrease key x to y.
R h x     Remove key x.
U h1 h2   Union Hh1 and Hh2, store the result in Hh1 and empty Hh2.
P h       Print the heap in the required format.

PART B COMMANDS
V h       Visualize heap h.
W h1 h2   Visualize the Union operation.

The V command displays every Binomial Tree separately and shows
the actual parent-child relationships.

The W command shows:
1. Both heaps before Union.
2. Every Bk + Bk -> Bk+1 link.
3. The final heap after Union.

OWN USEFUL FEATURE
The visualizer displays the degree beside every node. This helps
the user understand and verify the structure of each Binomial Tree.

IMPORTANT
The Part B visualization is printed only in the terminal. It is not
written to output.txt so that the normal Part A output remains suitable
for automatic checking.