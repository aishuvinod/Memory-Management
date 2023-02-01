#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vect.h"

/** Main data structure for the vector. */
struct vect {
  char **data;             /* Array containing the actual data. */
  unsigned int size;       /* Number of items currently in the vector. */
  unsigned int capacity;   /* Maximum number of items the vector can hold before growing. */
};

/** Construct a new empty vector. */
vect_t *vect_new() {

  vect_t *v = malloc(sizeof(vect_t)); //creates a vect_t pointer
  v->size = 0;//sets the value of the size prop of the vect struct pointed to by v to 0
  v->capacity = VECT_INITIAL_CAPACITY;// sets the value of the capacity prop of the vect struct pointed to by v to VECT_INITIAL_CAPACITY
  v->data = (char **) malloc(VECT_INITIAL_CAPACITY * sizeof(char *));//sets the value of the data prop of the  vect struct pointed to by v to an allocated block of memory of size VECT_INITIAL_CAPACITY * sizeof(char *)

  return v;
}

/** Delete the vector, freeing all memory it occupies. */
void vect_delete(vect_t *v) {

  for (int i = 0; i < v->size; i = i + 1) {//goes through all the elements of data
    char *toDel = v->data[i];//gets the pointer of the element in the ith poistion
    free(toDel);//free the pointer
  }

  free(v->data);//after that is done, free the allocated block of memory for the data vector
  free(v);//free the vector pointer

}

/** Get the element at the given index. */
const char *vect_get(vect_t *v, unsigned int idx) {
  assert(v != NULL);
  assert(idx < v->size);

  /* [TODO] Complete the function */
  if (idx >= 0){ // if the id is greater than or equal to 0
    const char *toGet = v->data[idx];//get the pointer at the idxth index and store it as a const (cant be changed)
    return toGet; //return the const toGet
  }
}

/** Get a copy of the element at the given index. The caller is responsible
 *  for freeing the memory occupied by the copy. */
char *vect_get_copy(vect_t *v, unsigned int idx) {
  assert(v != NULL);
  assert(idx < v->size);

  /* [TODO] Complete the function */
  if (idx >= 0) {//if the idx is greater than or equal to 0
    char *toCopy = v->data[idx];//get the pointer to the element to copy
    char *copy = malloc(strlen(toCopy)+1);//allocate memory for a new pointer
    strcpy(copy, toCopy);//copy the contents of toCopy to the copy pointer
    return copy;//return the copy pointer
  }
}
/** Set the element at the given index. */
void vect_set(vect_t *v, unsigned int idx, const char *elt) {
  assert(v != NULL);
  assert(idx < v->size);
  free(v->data[idx]);//first, free the memory that exists in the idxth index
  char *toAdd = malloc(strlen(elt) + 1);//allocate memory for the new string to store
  strcpy(toAdd, elt);//copy the contents of elt into toAdd

  /* [TODO] Complete the function */
  v->data[idx] = toAdd;//store the toAdd pointer in the array
}

/** Add an element to the back of the vector. */
void vect_add(vect_t *v, const char *elt) {
  assert(v != NULL);

  if (v->size == v-> capacity) {//if the size is equal to the capacity...
    if (v->capacity <= VECT_MAX_CAPACITY) { // if the capacity is less than the max capacity allowed, then we can expand
      v->data = realloc(v->data, (v->capacity * VECT_GROWTH_FACTOR)*sizeof(char *));//reallocate the memory by doubling the size of the memory block and copying over all of its contents to the new block of memory
      v->capacity = VECT_GROWTH_FACTOR * v->capacity;//doubles the capacity of the vector struct pointed to by v
    }
  }
  if (v->size < VECT_MAX_CAPACITY) {//if the size if less than the maximum possible capacity, only then you can add
    v->data[v->size] = malloc((strlen(elt) + 1));//allocates memory of size strlen(elt) + 1 to v->data[v->size]
    strcpy(v->data[v->size],elt);//copies the contents of elt into v->data[v->size]
    v->size = v->size + 1;//increments the size of the vector by one
  }
}

/** Remove the last element from the vector. */
void vect_remove_last(vect_t *v) {
  assert(v != NULL);
  if (v->size > 0) {
    free(v->data[v->size - 1]);//frees the pointer stored at the end of the array
    v->size = v->size - 1;//decrements the size of the array by one
  }

}

/** The number of items currently in the vector. */
unsigned int vect_size(vect_t *v) {
  assert(v != NULL);
  return v->size; //returns the size of the vector pointed to by v
}

/** The maximum number of items the vector can hold before it has to grow. */
unsigned int vect_current_capacity(vect_t *v) {
  assert(v != NULL);
  return v->capacity;//returns the capacity of the vector pointed to by v
}
