//2022csb1104
//Prasun Sarkar

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int t; // minimum degree (global)

typedef struct btNode {
    int numKeys;
    int isLeaf;
    int *keys;
    struct btNode **children;
} btNode;

typedef struct BTree {
    btNode *root;
} BTree;

// Prototypes
btNode *allocateNode(void);
void createBTree(BTree *tree);
void splitChild(btNode *parent, int index);
void insertNonFull(btNode *node, int key);
void insert(BTree *tree, int key);
bool search(btNode *node, int key);
void inOrderTraversal(btNode *node);
int findPredecessor(btNode *node, int index);
int findSuccessor(btNode *node, int index);
void deleteFromNonLeaf(btNode *node, int index);
void deleteFromLeaf(btNode *node, int index);
void borrowFromPrevious(btNode *node, int index);
void borrowFromNext(btNode *node, int index);
void mergeNodes(btNode *node, int index);
void deleteKey(btNode *node, int key);
void fillChild(btNode *node, int index);
void bTreeDelete(BTree *tree, int key);
int findMinimum(btNode *node);

// Allocate and initialize a new node
btNode *allocateNode(void) {
    btNode *node = (btNode *)malloc(sizeof(btNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->keys = (int *)malloc((2 * t - 1) * sizeof(int));
    node->children = (btNode **)malloc((2 * t) * sizeof(btNode *));
    if (!node->keys || !node->children) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->numKeys = 0;
    node->isLeaf = 1;
    for (int i = 0; i < 2 * t; i++) node->children[i] = NULL;
    return node;
}

void createBTree(BTree *tree) {
    btNode *rootNode = allocateNode();
    tree->root = rootNode;
}

void splitChild(btNode *parent, int index) {
    btNode *oldChild = parent->children[index];
    btNode *newChild = allocateNode();
    newChild->isLeaf = oldChild->isLeaf;
    newChild->numKeys = t - 1;

    // Copy keys
    for (int i = 0; i < t - 1; i++) {
        newChild->keys[i] = oldChild->keys[i + t];
    }

    // Copy children if not leaf
    if (!oldChild->isLeaf) {
        for (int i = 0; i < t; i++) {
            newChild->children[i] = oldChild->children[i + t];
            oldChild->children[i + t] = NULL; // detach
        }
    }

    oldChild->numKeys = t - 1;

    // Shift parent's children and keys to make space
    for (int i = parent->numKeys; i >= index + 1; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[index + 1] = newChild;

    for (int i = parent->numKeys - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }

    parent->keys[index] = oldChild->keys[t - 1];
    parent->numKeys++;
}

void insertNonFull(btNode *node, int key) {
    int i = node->numKeys - 1;

    if (node->isLeaf) {
        // Move keys to make room
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->numKeys++;
    } else {
        while (i >= 0 && key < node->keys[i]) i--;
        i++;
        if (node->children[i]->numKeys == 2 * t - 1) {
            splitChild(node, i);
            if (key > node->keys[i]) i++;
        }
        insertNonFull(node->children[i], key);
    }
}

void insert(BTree *tree, int key) {
    btNode *root = tree->root;
    if (root->numKeys == 2 * t - 1) {
        btNode *newRoot = allocateNode();
        newRoot->isLeaf = 0;
        newRoot->children[0] = root;
        tree->root = newRoot;
        splitChild(newRoot, 0);
        insertNonFull(newRoot, key);
    } else {
        insertNonFull(root, key);
    }
}

bool search(btNode *node, int key) {
    int i = 0;
    while (i < node->numKeys && key > node->keys[i]) i++;
    if (i < node->numKeys && key == node->keys[i]) return true;
    if (node->isLeaf) return false;
    return search(node->children[i], key);
}

void inOrderTraversal(btNode *node) {
    if (!node) return;
    int i;
    for (i = 0; i < node->numKeys; i++) {
        if (!node->isLeaf) inOrderTraversal(node->children[i]);
        printf("%d ", node->keys[i]);
    }
    if (!node->isLeaf) inOrderTraversal(node->children[i]);
}

int findPredecessor(btNode *node, int index) {
    btNode *current = node->children[index];
    while (!current->isLeaf) current = current->children[current->numKeys];
    return current->keys[current->numKeys - 1];
}

int findSuccessor(btNode *node, int index) {
    btNode *current = node->children[index + 1];
    while (!current->isLeaf) current = current->children[0];
    return current->keys[0];
}

void deleteFromNonLeaf(btNode *node, int index) {
    int key = node->keys[index];
    if (node->children[index]->numKeys >= t) {
        int pred = findPredecessor(node, index);
        node->keys[index] = pred;
        deleteKey(node->children[index], pred);
    } else if (node->children[index + 1]->numKeys >= t) {
        int succ = findSuccessor(node, index);
        node->keys[index] = succ;
        deleteKey(node->children[index + 1], succ);
    } else {
        mergeNodes(node, index);
        deleteKey(node->children[index], key);
    }
}

void deleteFromLeaf(btNode *node, int index) {
    for (int i = index + 1; i < node->numKeys; i++) node->keys[i - 1] = node->keys[i];
    node->numKeys--;
}

void borrowFromPrevious(btNode *node, int index) {
    btNode *child = node->children[index];
    btNode *sibling = node->children[index - 1];

    // Shift child's keys and children to the right
    for (int i = child->numKeys - 1; i >= 0; i--) child->keys[i + 1] = child->keys[i];
    if (!child->isLeaf) {
        for (int i = child->numKeys; i >= 0; i--) child->children[i + 1] = child->children[i];
    }

    child->keys[0] = node->keys[index - 1];
    if (!child->isLeaf) child->children[0] = sibling->children[sibling->numKeys];

    node->keys[index - 1] = sibling->keys[sibling->numKeys - 1];
    child->numKeys++;
    sibling->numKeys--;
}

void borrowFromNext(btNode *node, int index) {
    btNode *child = node->children[index];
    btNode *sibling = node->children[index + 1];

    child->keys[child->numKeys] = node->keys[index];
    if (!child->isLeaf) child->children[child->numKeys + 1] = sibling->children[0];

    node->keys[index] = sibling->keys[0];
    for (int i = 1; i < sibling->numKeys; i++) sibling->keys[i - 1] = sibling->keys[i];
    if (!sibling->isLeaf) {
        for (int i = 1; i <= sibling->numKeys; i++) sibling->children[i - 1] = sibling->children[i];
    }

    child->numKeys++;
    sibling->numKeys--;
}

void mergeNodes(btNode *node, int index) {
    btNode *child = node->children[index];
    btNode *sibling = node->children[index + 1];

    // Pull down the key from current node into child
    child->keys[t - 1] = node->keys[index];

    // Copy keys from sibling to child
    for (int i = 0; i < sibling->numKeys; i++) child->keys[i + t] = sibling->keys[i];

    // Copy children from sibling
    if (!child->isLeaf) {
        for (int i = 0; i <= sibling->numKeys; i++) {
            child->children[i + t] = sibling->children[i];
            sibling->children[i] = NULL; // detach
        }
    }

    // Shift keys and children of current node
    for (int i = index + 1; i < node->numKeys; i++) node->keys[i - 1] = node->keys[i];
    for (int i = index + 2; i <= node->numKeys; i++) node->children[i - 1] = node->children[i];

    child->numKeys += sibling->numKeys + 1;
    node->numKeys--;

    // Free sibling
    if (sibling) {
        free(sibling->keys);
        free(sibling->children);
        free(sibling);
    }
}

void deleteKey(btNode *node, int key) {
    int index = 0;
    while (index < node->numKeys && key > node->keys[index]) index++;

    if (index < node->numKeys && key == node->keys[index]) {
        if (node->isLeaf) deleteFromLeaf(node, index);
        else deleteFromNonLeaf(node, index);
    } else {
        if (node->isLeaf) {
            printf("The key %d is not present in the tree.\n", key);
            return;
        }
        bool flag = (index == node->numKeys);
        if (node->children[index]->numKeys < t) fillChild(node, index);
        // After fill, if we merged with previous child, the target child index might change
        if (flag && index > node->numKeys) deleteKey(node->children[index - 1], key);
        else deleteKey(node->children[index], key);
    }
}

void fillChild(btNode *node, int index) {
    if (index != 0 && node->children[index - 1]->numKeys >= t) borrowFromPrevious(node, index);
    else if (index != node->numKeys && node->children[index + 1]->numKeys >= t) borrowFromNext(node, index);
    else {
        if (index != node->numKeys) mergeNodes(node, index);
        else mergeNodes(node, index - 1);
    }
}

void bTreeDelete(BTree *tree, int key) {
    if (!tree->root) return;
    deleteKey(tree->root, key);

    // If root's keys become 0, make its first child the new root
    if (tree->root->numKeys == 0) {
        btNode *oldRoot = tree->root;
        btNode *newRoot = oldRoot->isLeaf ? NULL : oldRoot->children[0];
        if (newRoot) {
            tree->root = newRoot;
        } else {
            // Tree is now empty: keep a single empty root node
            tree->root = allocateNode();
        }
        // Free the old root
        if (oldRoot) {
            free(oldRoot->keys);
            free(oldRoot->children);
            free(oldRoot);
        }
    }
}

int findMinimum(btNode *node) {
    if (!node) return -1; // empty
    btNode *current = node;
    while (!current->isLeaf) current = current->children[0];
    if (current->numKeys > 0) return current->keys[0];
    return -1;
}

// Simple cleanup: free entire tree (post-order)
void freeNode(btNode *node) {
    if (!node) return;
    if (!node->isLeaf) {
        for (int i = 0; i <= node->numKeys; i++) freeNode(node->children[i]);
    }
    free(node->keys);
    free(node->children);
    free(node);
}

int main() {
    printf("Enter the minimum degree of the B-tree 't' (>=2): ");
    if (scanf("%d", &t) != 1 || t < 2) {
        printf("Invalid degree. t must be integer >= 2.\n");
        return 1;
    }

    int app;
    printf("Write '1' for First approach and '2' for Second approach : ");
    if (scanf("%d", &app) != 1) return 1;

    BTree tree;
    createBTree(&tree);

    if (app == 2) {
        int choice, key, rt;
        while (1) {
            printf("\nChoose an option:\n");
            printf("1. Insert\n2. Search\n3. Delete\n4. Print Inorder\n5. Find Minimum\n6. Exit\n");
            printf("Enter your choice: ");
            if (scanf("%d", &choice) != 1) break;
            switch (choice) {
                case 1:
                    printf("Enter the number of elements you want to enter: ");
                    if (scanf("%d", &rt) != 1) break;
                    printf("Enter %d integers:\n", rt);
                    for (int i = 0; i < rt; i++) {
                        if (scanf("%d", &key) != 1) break;
                        insert(&tree, key);
                    }
                    break;
                case 2:
                    printf("Enter the key to search: ");
                    if (scanf("%d", &key) != 1) break;
                    if (search(tree.root, key)) printf("Key %d is present in the tree.\n", key);
                    else printf("Key %d is not present in the tree.\n", key);
                    break;
                case 3:
                    printf("Enter the key to delete: ");
                    if (scanf("%d", &key) != 1) break;
                    bTreeDelete(&tree, key);
                    break;
                case 4:
                    printf("Inorder Traversal: ");
                    inOrderTraversal(tree.root);
                    printf("\n");
                    break;
                case 5:
                    printf("Minimum Key: %d\n", findMinimum(tree.root));
                    break;
                case 6:
                    // free memory before exit
                    freeNode(tree.root);
                    return 0;
                default:
                    printf("Invalid choice. Try again.\n");
            }
        }
    } else if (app == 1) {
        int n, ele;
        printf("Enter 'n' : ");
        if (scanf("%d", &n) != 1) return 1;
        for (int i = n; i > 0; i--) insert(&tree, i);
        printf("Inorder traversal is: ");
        inOrderTraversal(tree.root);
        printf("\nEnter an element to search: ");
        if (scanf("%d", &ele) != 1) return 1;
        if (search(tree.root, ele)) printf("Element found\n");
        else printf("No element exist\n");
        freeNode(tree.root);
    } else {
        printf("Invalid input\n");
    }

    return 0;
}

 
