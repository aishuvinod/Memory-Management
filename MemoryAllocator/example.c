#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Include any other headers we need here

typedef struct block {
  size_t size;        // How many bytes beyond this block have been allocated in the heap
  struct block *next; // Where is the next block in your linked list
  int free;           // Is this memory free, i.e., available to give away?
                      // (optional) Perhaps you can embed other information--remember,
                      // you are the boss!

}block_t;

 #define BLOCK_SIZE sizeof(block_t)
 block_t* searchAvailable(block_t* block, size_t s);
 block_t* create_space(block_t* block, size_t s);
 static block_t *init(size_t s);
 block_t* GLOBAL_START = NULL;
 static block_t *start = 0;
 block_t *create = 0;

// NOTE: You should NOT include <stdlib.h> in your final implementation

#include <debug.h> // definition of debug_printf

static block_t *init(size_t s) {
   start = sbrk(s + BLOCK_SIZE);
   start->size = s;
   start->free = 0;
   start->next = NULL;

   create = start;
   return start;
}

void *mymalloc(size_t s) {
  block_t *p;

 // check to make sure they are actually requesting memory greater than 0
 // cannot allocate negative memory!
 if (s <= 0) {
  return NULL;
}

 // if this is the first call we do not need to create more space
 if(start == 0) {
    return (void*)(init(s) + 1);
  }
  // if not first call search to see if any block in our list can handle our request
   else {
    p = searchAvailable(start, s);
  }

 // if searchAvailable returns NULL, no available space is found & request more memory
 if(p == GLOBAL_START) {
   p = create_space(start, s);
   if (!p) {
     return NULL;
}
 }
  // update pointer to be set to the begining of new block
  debug_printf("malloc %zu bytes\n", s);
  return(p + 1);
}


// sees if the current list of blocks can handle our size request
block_t* searchAvailable( block_t* block, size_t s) {
    while(block != GLOBAL_START) {
      // see if our current block can handle our request
      // if it can we set it to not be free (as it has been used)
      if (block->size >= s && block->free == 1) {
       block->free = 0;
       return block;
} else {
    // move onto next block in list
    block = block->next;
 }
}
// if no block in list can handle our request we return NULL;
return GLOBAL_START;
}

// allocates more space in our malloc if requested/needed 
 block_t* create_space (block_t* block, size_t s) {
  block_t *current;
  // get current pointer
  block = sbrk(0);
  current = sbrk(s + BLOCK_SIZE);

  // makes sure create was created to be at our current pointer
  assert((void*)block == current);

  // checks to see if sbrk failed
  if (current == (void*) -1) {
   return NULL;
}
// adds our new block to the end of our linked list
current->size = s;
current->next = NULL;
current->free = 0;

create->next = current;
create = current;
return current;
}


void *mycalloc(size_t nmemb, size_t s) {
  size_t size = nmemb * s;
  void *p = mymalloc(size);

  // initializes new blocks to 0
  memset(p, 0, size);
  if (!p) {
    // We are out of memory
    // if we get NULL back from malloc
  }
  debug_printf("calloc %zu bytes\n", s);

  return p;
}

// frees the memory of the requested block
void myfree(void *ptr) {

  if (!ptr) {
    return;
}

// move to the begining of our block (current pointer should point to end)
block_t* block_ptr = (block_t*)ptr - 1;

// check to make sure the given block is not already free
assert(block_ptr->free == 0);

// dictates the block as now free
block_ptr->free = 1;
debug_printf("Freed some memory\n");
}
