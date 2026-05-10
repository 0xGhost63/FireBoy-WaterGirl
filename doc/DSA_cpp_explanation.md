# `DSA.cpp` Detailed Explanation

**File**: `src/DSA.cpp`

Implements low-level custom Data Structures and Algorithms from scratch (avoiding standard libraries).

### Function-Level Breakdown

#### 1. Queue Functions (`queueInit`, `queueEnqueue`, `queueDequeue`)
- Implements a circular array queue.
- `queueEnqueue`: Inserts at the `rear` index, then increments `rear` via modulo `(rear + 1) % QUEUE_MAX` to wrap around.
- `queueDequeue`: Grabs the item at `front`, increments `front`, and decreases the `count`.
- Used for `EventQueue` (ensuring events fire chronologically) and `ConveyorQueue`.

#### 2. Doubly-Linked List (`listAppend`, `listNext`, `listPrev`, `listFree`)
- Used for mapping the game levels.
- `listAppend`: Dynamically creates a new `LevelNode`. Points its `prev` to the current `tail`. If the list isn't empty, updates the old `tail->next` to point to this new node. Sets `tail` to the new node.
- `listNext`: Checks if `current->next` exists. If so, `current = current->next` and returns true. This is the core mechanism for advancing the game upon beating a level.

#### 3. Binary Search Tree (`bstInsert`, `bstGet`, `bstFree`)
- Organizes the 2D tile map spatially for instant lookup.
- `bstInsert`: Recursively splits based on Row (R) and Column (C). If the inserted key is smaller, it goes to the `left` child; if larger, it goes to the `right` child.
- `bstGet`: Traverses the tree matching the exact Row and Col, returning `TILE_SOLID` or `TILE_EMPTY`. Physics heavily relies on this for instant collision checks.

#### 4. Quick Sort (`quickSort`, `partition`)
- Used to sort the Leaderboard from highest to lowest score.
- `partition`: Selects a pivot element. Moves all elements larger than the pivot to the left, and smaller to the right. Returns the pivot index.
- `quickSort`: Recursively calls itself on the left partition and right partition, resulting in an `O(n log n)` sorted array.

#### 5. Binary Search (`binarySearch`)
- Used to instantly find a player's leaderboard rank.
- Sets `low = 0` and `high = n - 1`. Checks the `mid` element.
- Since the array is descending, if `score > arr[mid].score`, it halves the search space by setting `high = mid - 1`. This finds the rank in `O(log n)` time.

#### 6. Min-Heap (`minHeapPush`, `minHeapPop`, `gemMinHeapFind`)
- A specialized tree where the root is always the absolute smallest element.
- `minHeapPush`: Inserts an element at the bottom, then "bubbles up" (swaps with its parent) until it obeys the rule that parents are smaller than children.
- `gemMinHeapFind`: The Game Engine uses this to iterate over all gems, calculate the strict geometric distance to the player, push them into the heap, and pop the absolute smallest distance to draw a Hint line.

#### 7. Hash Maps (`gateMapInsert`, `teleportMapGet`)
- Maps arbitrary integer IDs (like "Gate ID 42") to an array index (like index 0).
- Uses a hash function `(id % HASH_SIZE)` to find a bucket. Uses linked list chaining inside the bucket to handle collisions if multiple IDs hash to the same bucket.
