#include <stdio.h>
#include <malloc.h>
#include <stdint.h>

typedef struct {
	void* ptr;
	uint64_t size;
	uint64_t num_frees;
	char alloc_location[64];
	char free_location[64];
} Allocation;
#define ALLOC_CHUNK 100
uint64_t max_mem = 0;
uint64_t mem_leaked = 0;
uint64_t num_allocations = 0;
uint64_t allocations_length = 0;
Allocation* allocations = NULL;

void* occulus_malloc(size_t size, const char* file, uint64_t line) {
	void* ptr = malloc(size);

	if (num_allocations >= allocations_length) {
		allocations_length += ALLOC_CHUNK;
		allocations = realloc(allocations, allocations_length * sizeof(Allocation));
	}
	allocations[num_allocations] = (Allocation){
		.ptr = ptr,
		.size = size,
		.num_frees = 0,
		.alloc_location = { 0 },
		.free_location = { 0 },
	};
	sprintf(allocations[num_allocations].alloc_location, "%s:%lu", file, line);
	num_allocations++;

	mem_leaked += size;
	max_mem = (mem_leaked > max_mem ? mem_leaked : max_mem);

	return ptr;
}

void* occulus_realloc(void* old_ptr, size_t size, const char* file, uint64_t line) {
	void* ptr = realloc(old_ptr, size);
	printf("%s:%lu> realloc(%lu) 0x%0lx->0x%0lx\n", file, line, size, (uint64_t)old_ptr, (uint64_t)ptr);
	return ptr;
}

void occulus_free(void* ptr, const char* file, uint64_t line) {
	int found = 0;
	for (uint64_t t = 0; t < num_allocations; t++) {
		if (allocations[t].ptr == ptr) {
			allocations[t].num_frees++;
			if (allocations[t].num_frees == 2) {
				printf("%s:%lu> memory freed more than once, 0x%0lx\n", file, line, (uint64_t)ptr);
				return;
			} else {
				sprintf(allocations[num_allocations].alloc_location, "%s:%lu", file, line);
			}

			mem_leaked -= allocations[t].size;

			found = 1;
			break;
		}
	}

	if (!found) {
		printf("%s:%lu> free on unrecognized memory, 0x%0lx\n", file, line, (uint64_t)ptr);
	}

	free(ptr);
}

void occulus_print() {
	printf("MEM_DEBUG>\nMax: %lukb\tLeaked: %lukb\n", max_mem / 1024, mem_leaked / 1024);
	for (uint64_t t = 0; t < num_allocations; t++) {
		if (allocations[t].num_frees == 0) {
			printf("Memory at %s never freed!\n", allocations[t].alloc_location);
		} else if (allocations[t].num_frees > 1) {
			printf("Memory at %s freed %lu times!\n", allocations[t].alloc_location, allocations[t].num_frees);
		}
	}
	printf("<MEM_DEBUG\n");
}
