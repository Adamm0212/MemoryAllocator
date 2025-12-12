#include <unistd.h>
#include <stdio.h>

struct header {
    size_t size;
    int free;
    struct header* next;
};

#define MIN_SPLIT_SIZE 8

struct header* free_list = NULL;

void* my_malloc(size_t size) {
    struct header* current = free_list;
    struct header* prev = NULL;

    while (current) {
        if (current->free == 1 && current->size >= size) {

            if (current->size >= size + sizeof(struct header) + MIN_SPLIT_SIZE) {
                struct header* new_block = (struct header*)((char*)current + sizeof(struct header) + size);
                new_block->size = current->size - size - sizeof(struct header);
                new_block->free = 1;
                new_block->next = current->next;

                current->size = size;
                current->free = 0;
                current->next = NULL;

                // Link previous block with next block
                // If block we allocate is the head
                if (prev == NULL) {
                    free_list = new_block;
                }
                else {
                    prev->next = new_block;
                }

                return (void*)((char*)current + sizeof(struct header));
            }
            else {
                current->free = 0;
                if (prev == NULL) {
                    free_list = current->next;
                }
                else {
                    prev->next = current->next;
                }
                current->next = NULL;
                return (void*)((char*)current + sizeof(struct header));
            }
        }
        prev = current;
        current = current->next;
    }

    // No suitable block, request more memory

    // Sbrk increases heap by n bytes. Block is a pointer to the old memory start
    void* block = sbrk(sizeof(struct header) + size);
    if (block == (void*)-1) return NULL; // sbrk failed

    // Make it a block
    struct header* h = (struct header*)block;
    h->size = size;
    h->free = 0;
    h->next = NULL;

    return (void*)((char*)h + sizeof(struct header));
}

void my_free(void *user_ptr) {
    if (!user_ptr) return; // ignore NULL

    struct header* h = (struct header*)((char*)user_ptr - sizeof(struct header));

    h->free = 1;
    h->next = free_list;
    free_list = h;
}

int main() {
    int* array1 = (int*)my_malloc(1000 * sizeof(int));
    int* array2 = (int*)my_malloc(50 * sizeof(int));

    for (int i = 0; i < 100; i++) array1[i] = i;
    for (int i = 0; i < 50; i++) array2[i] = i * 2;

    // Free the first block
    my_free(array1);

    // Allocate a smaller block, should reuse the freed memory
    int* array3 = (int*)my_malloc(1 * sizeof(int));
    int* array4 = (int*)my_malloc(2 * sizeof(int));
    printf("array1 = %p\narray2 = %p\narray3 = %p\narray4 = %p\n", array1, array2, array3, array4);
    return 0;
}