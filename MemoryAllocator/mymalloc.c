#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#include <malloc.h> 
#include <stdio.h> 
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <debug.h> // definition of debug_printf
#include <sys/types.h>
#include <sys/mman.h>
#include <pthread.h>



//represents a block of memory
typedef struct block {
  size_t size;        // How many bytes beyond this block have been allocated in the heap
  struct block *next; // Where is the next block in your linked list
  int free;           // Is this memory free, i.e., available to give away?
  int debug;          // (optional) Perhaps you can embed other information--remember,
                      // you are the boss!
}block_t; //given ,, can be used to represent linked list

#define BLOCK_SIZE sizeof(block_t) // size of a block of memory
//initialized as a global variable so that the beginning of the linked list can be referenced from anywhere
block_t* GLOBAL_HEAD = NULL; //represents the head/beginning of the linked list
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // a mutex

//the size of a page that mmap's region size will align along
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
//4096


//recursively iterates through the list and finds the first block of memory that can fit the requested size 
block_t *find_helper(block_t *b, size_t s) {
  if (b == NULL) { //case where there is no block at this pointer
    return NULL;
  } else if (b->size >= s && b->free == 1) { //if the block is free and the size can accomodate the requested size of memory
    return b;
  } else {
    return find_helper(b->next, s); //iterate recursively
  }
}



//for a given requested size of memory, find the first block that is open in the list
//returns pointer to that block of memory
block_t *find(size_t s) {
  return find_helper(&GLOBAL_HEAD, s);
}


//gets the end of the list
//does this by getting the block that has no next block
block_t *end_block_helper(block_t *b) {
  if (b->next == NULL) {
    return b; //last block has a null next
  } else {
    return end_block_helper(b->next);//recursively iterate
  }
}



//be able to access the last block at the end of the free list
//returns the pointer
block_t *end_block() {
  return end_block_helper(&GLOBAL_HEAD);
}



//function that allocates requested number of pages using mmap
//takes in the number of pages needed and returns a pointer to the start of the newly added page/s
void *allocate_additional(int pages) {
  void *add = mmap(NULL, pages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (add == (void *) -1) {
    pthread_mutex_unlock(&mutex);
    return add; //the page is added
  }

  //page is added as a block in the list
  block_t *end = end_block(); //get the end of the list
  end->next = add; //newly added page should be at the end of the list
  end->next->free = 0; //not free
  end->next->size = pages * PAGE_SIZE - BLOCK_SIZE; 
  end->next->next = NULL; //newly added block should be the last block
  assert(end->next == add && end->next->free == 0 && end->next->size == pages * PAGE_SIZE - BLOCK_SIZE && end->next->next == NULL);
 
  pthread_mutex_unlock(&mutex);
  return (void *) add + BLOCK_SIZE;
}




//splits a block into two. If not, returns an allocated block
//takes in block that needs to be split and the size of the first half split
void *split_block(block_t *block, size_t s) {
  block_t initial; //block before split
  initial.size = block->size;
  initial.next = block->next;
  assert(initial.size == block->size && 
  initial.next ==  block->next); //the size of the block and its next block initally
 

 //the second half of the block which will not be used for allocation
  block_t *b = ((void *) block) + BLOCK_SIZE + s;
  b->free = 1; //it will be free since only the first half will be used for allocation
  b->next = initial.next; //next block should still stay the same
  b->size = initial.size - BLOCK_SIZE - s; //the size will be  the size left over based on the given size for the first half
  assert(b->free == 1 && 
  b->next == initial.next && b->size == initial.size - BLOCK_SIZE - s); //the size of the block and its next block initally


  block->free = 0; //the first half of the split block which has been used for allocation 
  block->next = b; //the next block should be the second half 
  block->size = s; //size of the block should be as requested
  assert(block->free == 0 && block->size == s);


  pthread_mutex_unlock(&mutex); //unlock so it is accessible 
  return ((void *) block) + BLOCK_SIZE;
}



//splits a page and adds the two new blocks to the list
//takes in the requested memory size
// returns a pointer to the first block that is added to the free list
void *split_page(size_t s) {
  //map a new page
  void *newp = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (newp == (void *) -1) { //if the page cannot be split
    pthread_mutex_unlock(&mutex); //the newly added page should be accessible now
    return newp; //return pointer to that page
  }

  // Split the newpage into two blocks and add at the end of the list
  block_t *end = end_block(); //get the last block at th end of the list
  end->next = newp; //the next block at the end of the list will be the new page
  end->next->free = 0; //the new page has been allocated
  end->next->size = s; //page will be of requested memory size
  assert(end->next == newp && end->next->free == 0 && end->next->size  == s);

  //now split that page
  block_t *split = ((void *) newp) + s + BLOCK_SIZE;
  end->next->next = split; 
  split->free = 1; //the second half of split will not be allocated 
  split->next = NULL; //should be the last block
  split->size = PAGE_SIZE - BLOCK_SIZE - BLOCK_SIZE - s; //should be the left over of the requested size
  assert(end->next->next == split && split->free == 1 && split->next == NULL && split->size == PAGE_SIZE - BLOCK_SIZE - BLOCK_SIZE - s); 
  
  pthread_mutex_unlock(&mutex); //both blocks should be accessible
  return (void *) newp + BLOCK_SIZE;
}



//takes in the size of requested memory and allocates a page that is either already in the list or adds a page and allocates that page
void *allocate_page(size_t s) {
  block_t *found = find(s); //goes through list and finds the first open block that will fit request

  // if no open block that can fit request is found
  if (found == NULL) {
    // check if the size requires the page to be split or not
    if (PAGE_SIZE - BLOCK_SIZE - s <= BLOCK_SIZE) {
      return allocate_additional(1); //allocates one page 
    } else { //allocate a page that is split to conserve space
      return split_page(s);
    }
  } else {
    //split the block if a block is found but the size is bigger than needed
    if ((int)(found->size - s - BLOCK_SIZE) >= (int)1) {
      return split_block(found, s); //returns the split block and the first half of the block's size
    } else {
      found->free = 0; //if a block is found and doesnt need to be split, mark as allocated
      assert(found->free == 0);
      pthread_mutex_unlock(&mutex);
      return ((void *) found) + BLOCK_SIZE;
    }
  }
}



//if too consecutive blocks are free, then add them to conserve space
void coalesce() {
  int coalesced; //initialize a counter of blocks that have been merged 
  block_t *current = &GLOBAL_HEAD; //start at the beginning of the list

  while (current->next != NULL) { //if there is a block right next to the current block we are at
    block_t *next = current->next; //iterate through the list, make the next block the current
    //if the current and next block are both free
    if (current->free == 1 && next->free == 1 && current->next == (void *) current + BLOCK_SIZE + current->size){ 
      current->next = next->next; //iterate
      current->size += next->size + BLOCK_SIZE; //coalesce, so when the two blocks come together the size will be the current block's size>
      coalesced++; //add to the counter which keeps track of the number of coalesced blocks
    } else { //if two consecutive blocks are not current, then iterate through the list
      current = current->next; //iterate
    }
  }
  debug_printf("Coalesced %d blocks.\n", coalesced);
}



//helper function to compute pages
//takes in the size of the memory request and returns the number of pages that would be needed to allocate that memory
int compute_pages(size_t s){
 int pages = (s + BLOCK_SIZE) / PAGE_SIZE; 
  if ((s + BLOCK_SIZE) % PAGE_SIZE > 0) { //when you cannot allocate one whole page and need more than one page
    pages++; //add to the counter that keeps track of how many pages will be required.
  }
return pages;
}


void *mymalloc(size_t s) {
  pthread_mutex_lock(&mutex);
  int pages = compute_pages(s);
  // if the request needs more than one page
  if (s + BLOCK_SIZE >= PAGE_SIZE) {
    // allocate how many ever more pages needed and return pointer 
    return allocate_additional(pages);
  } else { //if the request can be fit in one page size
    return allocate_page(s); //allocate to a block already in the list or add one block
  }
}


// takes in the number of elements to allocate and the storage size of those elements
//returns the pointer to allocated block of memory
void *mycalloc(size_t nmemb, size_t s) {
  size_t size = nmemb * s; //the total size required will be the number of elements times the required size
  void *ptr = mymalloc(size); //call malloc function on the current pointer given the size
  memset(ptr, 0, size); //initialize block to 0
  debug_printf("calloc %zu bytes\n", size);
  return ptr;
}


// Get the previous 
block_t *prev(block_t *b) {
  block_t *current = &GLOBAL_HEAD; //start at the head
  while (current->next != b) { //go upto the given block
    current = current->next; //iterate
  }
  return current;
}


//free memory by taking in the pointer to a memory address
void myfree(void *ptr) {
  assert(ptr != NULL);
  pthread_mutex_lock(&mutex);
  block_t *b = (block_t*)ptr-1; //get block 1 behind
  assert(b->free == 0);

  int size = b->size;
  if (size + BLOCK_SIZE < PAGE_SIZE) { //for a block of memory size less than the page size
    b->free = 1; //free
    assert(b->free == 1);
    coalesce(); //make sure to merge with consecutive free blocks if there are any
  } else { 
    block_t *previous = prev(b); //get the previous block
    previous->next = b->next; // merge to get rid of b
    munmap(b, size + BLOCK_SIZE); //unmap b
  }
  pthread_mutex_unlock(&mutex);
  debug_printf("Freed %zu bytes\n", size); 
} 

