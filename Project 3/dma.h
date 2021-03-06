#ifndef DMA_H
#define DMA_H

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int dma_init (int m);
void *dma_alloc (int size);
void dma_free (void *p);
void dma_print_page(int pno);
void dma_print_bitmap();
void dma_print_blocks();
int dma_give_intfrag();

#endif // DMA_H
