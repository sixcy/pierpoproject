#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* memory */
char memory[MEMORY_SIZE]; 

/* Structure declaration for a free block */
typedef struct free_block{
	int size; 
	struct free_block *next; 
} free_block_s, *free_block_t; 

/* Structure declaration for an occupied block */
typedef struct{
	int size; 
} busy_block_s, *busy_block_t; 


/* Pointer to the first free block in the memory */
free_block_t first_free; 


#define ULONG(x)((long unsigned int)(x))
#define max(x,y) (x>y?x:y)

#define WRITE_IN_MEMORY(type, address, value)(*((type*) address) = value)

void memory_init(void){
   	*((int*) memory) = MEMORY_SIZE;

   	WRITE_IN_MEMORY(int, memory, MEMORY_SIZE);
   	WRITE_IN_MEMORY(free_block_t, memory+sizeof(int), NULL);
   
   	first_free = (free_block_t) memory;
}

char *memory_alloc(int size){
	/* First Fit */

	free_block_t current_free;
	free_block_t previous;
	free_block_t new_free;

	current_free = first_free;
	previous = current_free;

	if (!first_free) {
		printf("first_free does not exist.\n");
		exit(EXIT_FAILURE);
	}

	/* Browse through the free_block list */
	while (current_free->size < size && current_free) {
		previous = current_free;
		current_free = current_free->next;
	}

	/* If we went through the whole list, then the memory is full */
	if (!current_free) {
		printf("Not enough memory space.\n");
		exit(EXIT_FAILURE);
	}

	/* We now allocate the block */

	/* PROBLEME POTENTIEL : SI ON A TOUT PILE LA PLACE */
	/* New pointer to the beginning of the new free block */
	new_free = (free_block_s*) (((char*)current_free) + size + sizeof(busy_block_s));
	/* Write the new size left in the structure */
   	WRITE_IN_MEMORY(int, new_free, current_free->size - size);
	/* Write the new next position in the structure */
   	WRITE_IN_MEMORY(free_block_t, ((char*)new_free)+sizeof(int), current_free->next);

	/* Now we have to replace the old free block by a busy one */
	current_free->size = size;

	/* previous -> new_free */
	/* Works even if we are on first_free */
	previous->next = new_free;

 	print_alloc_info((char*) current_free, current_free->size); 
/* 	print_alloc_info(addr, actual_size); 
 */

	return ((char*) current_free) + sizeof(busy_block_s);
}

void memory_free(char *p){
	print_free_info(p); 

	/* ... */
}


void print_info(void) {
	fprintf(stderr, "Memory : [%lu %lu] (%lu bytes)\n", (long unsigned int) 0, (long unsigned int) (memory+MEMORY_SIZE), (long unsigned int) (MEMORY_SIZE));
	fprintf(stderr, "Free block : %lu bytes; busy block : %lu bytes.\n", ULONG(sizeof(free_block_s)), ULONG(sizeof(busy_block_s))); 
}

void print_free_info(char *addr){
	if(addr)
		fprintf(stderr, "FREE  at : %lu \n", ULONG(addr - memory)); 
	else
		fprintf(stderr, "FREE  at : %lu \n", ULONG(0)); 
}

void print_alloc_info(char *addr, int size){
	if(addr){
		fprintf(stderr, "ALLOC at : %lu (%d byte(s))\n", 
				ULONG(addr - memory), size);
	}
	else{
		fprintf(stderr, "Warning, system is out of memory\n"); 
	}
}

void print_free_blocks(void) {
	free_block_t current; 
	fprintf(stderr, "Begin of free block list :\n"); 
	for(current = first_free; current != NULL; current = current->next)
		fprintf(stderr, "Free block at address %lu, size %u\n", ULONG((char*)current - memory), current->size);
}

char *heap_base(void) {
	return memory;
}


void *malloc(size_t size){
	static int init_flag = 0; 
	if(!init_flag){
		init_flag = 1; 
		memory_init(); 
		//print_info(); 
	}      
	return (void*)memory_alloc((size_t)size); 
}

void free(void *p){
	if (p == NULL) return;
	memory_free((char*)p); 
	print_free_blocks();
}

void *realloc(void *ptr, size_t size){
	if(ptr == NULL)
		return memory_alloc(size); 
	busy_block_t bb = ((busy_block_t)ptr) - 1; 
	printf("Reallocating %d bytes to %d\n", bb->size - (int)sizeof(busy_block_s), (int)size); 
	if(size <= bb->size - sizeof(busy_block_s))
		return ptr; 

	char *new = memory_alloc(size); 
	memcpy(new, (void*)(bb+1), bb->size - sizeof(busy_block_s) ); 
	memory_free((char*)(bb+1)); 
	return (void*)(new); 
}


#ifdef MAIN
int main(int argc, char **argv){

	/* The main can be changed, it is *not* involved in tests */
	memory_init();
	print_info(); 
	print_free_blocks();
	int i ; 
	for( i = 0; i < 10; i++){
		char *b = memory_alloc(rand()%8);
		memory_free(b); 
		print_free_blocks();
	}




	char * a = memory_alloc(15);
	a=realloc(a, 20); 
	memory_free(a);


	a = memory_alloc(10);
	memory_free(a);

	printf("%lu\n",(long unsigned int) (memory_alloc(9)));
	return EXIT_SUCCESS;
}
#endif 