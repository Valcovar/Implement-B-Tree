# Implement-B-Tree
A B-Tree is a self-balancing tree data structure that maintains sorted data and allows searches, sequential access, insertions, and deletions in logarithmic time. It's widely used in databases and file systems due to its ability to efficiently manage large amounts of data on disk.

Covers all core B-Tree operations.
Separates logic clearly into functions.
Supports two modes: batch insert (Approach 1) and interactive menu (Approach 2).
Uses dynamic memory allocation properly.

⚠️ Issues and Suggestions
 Global variable t should not be global
You’re using int t; globally. It would be better to:
Pass t as a parameter to relevant functions,
Or include it in the BTree struct.
