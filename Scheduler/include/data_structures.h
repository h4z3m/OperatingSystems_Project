/**
 * @file data_structures.h
 * @author yogilany@gmail.com, h4z3m1z@gmail.com, ahmedtarek1754@gmail.com
 * @brief
 * @version 1.0
 * @date 2022-12-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

typedef short bool;
typedef struct _node
{
    void *dataPtr;
    struct _node *nextNode;
} Node;

typedef struct _prio_node
{
    void *dataPtr;
    int priority;
    struct _prio_node *nextNode;
} PrioNode;

typedef struct
{
    Node *front;
    Node *rear;
    unsigned capacity;

} Queue;

typedef struct
{
    PrioNode *front;
    PrioNode *rear;
    unsigned capacity;
} PriorityQueue;

typedef struct
{
    Node *front;
    Node *rear;
    unsigned capacity;

} CircularQueue;

typedef struct
{
    // we need an array of pointers of type queue
    Queue *Qptrs[11];
} MultiLevelQueue;

typedef struct treeNode
{
    int data;                /**< the size of the treeNode */
    int startIndx;           /**< the start of the memory piece */
    int endIndx;             /**< the end of the memory piece */
    struct treeNode *left;   /**< a treeNode representing the left subtree */
    struct treeNode *parent; /**< a treeNode to the parent; equal NULL if there's no parent */
    struct treeNode *right;  /**< a treeNode representing the right subtree */
} treeNode;

/* Queue */
Queue *createQueue();

bool isEmpty(Queue *q);
void enqueue(Queue *q, void *dataPtr);
bool peek(Queue *q, void **dataPtr);
bool dequeue(Queue *queue, void **out_data);
void destroyQueue(Queue *q);

/* Priority Queue */
PriorityQueue *createPriorityQueue();

bool priorityIsEmpty(PriorityQueue *q);
void enqueuePriority(PriorityQueue *q, void *dataPtr, int priority);
bool peekPriority(PriorityQueue *q, void **out_data);
bool dequeuePriority(PriorityQueue *q, void **out_data);
void removeNodePriority(PriorityQueue *q, void **dataToDelete);
void destroyPriorityQueue(PriorityQueue *q);

/* Multi level Queue*/
MultiLevelQueue *createMultiLevelQueue();

// enqueue item in certain level
void enqueueMultiLevel(MultiLevelQueue *q, void **dataPtr, int priority_level);
// dequeue item in chosen level
bool dequeueMultiLevel(MultiLevelQueue *q, void **out_data, int priority_level);
// this function returns the first occupied level in the queue , otherwise ; it returns -1
int multiLevelisEmpty(MultiLevelQueue *q);
// destroys multilevel by destroying each queue
void destroyMultiLevelQueue(MultiLevelQueue *q);

/************************************************* Buddy system tree *************************************************/

#define MemorySize 1024

/**
 * @brief
 *
 * @param n
 * @return unsigned int
 */
unsigned int nextPowerOf2(unsigned int n);

/**
 * @brief Create a treeNode give it's data
 *
 * @param data the size of the memory the treeNode represents.
 * @param parent a pointer to the parent.
 * @param start the start of the memory the treeNode represents.
 * @return treeNode* the created treeNode
 */
treeNode *newNode(int data, treeNode *parent, int start);

/**
 * @brief
 *
 * @param size
 * @param currentSize
 * @param grandParent
 * @param dir
 * @param start
 * @return treeNode*
 */
treeNode *allocateRoot(int size, int currentSize, treeNode *grandParent, char dir, int start);

/**
 * @brief
 *
 * @param root
 * @param size
 * @return treeNode*
 */
treeNode *allocateTree(treeNode *root, int size);

/**
 * @brief This function used to allocate a piece of memory in the buddy system
 * memory management
 *
 * @param size The desired memory size to allocate
 * @return treeNode* a treeNode pointer to the allocated piece of memory
 */
treeNode *Allocate(int size);

/**
 * @brief
 *
 * @param root
 * @return int
 */
int Deallocate_(treeNode *root);

/**
 * @brief
 *
 * @param root
 */
void Deallocate(treeNode *root);

#endif // DATA_STRUCTURES_H