#include "dma.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <math.h>

int seg_size;
int bitmap_size, map_size;
int reserved;
void *cptr;

pthread_mutex_t lock;

int dma_init(int m)
{
    if (m < 14 || m > 22)
    {
        printf("Unaccepted Range!");
        return -1;
    }

    seg_size = pow(2, m);

    void *p = mmap(NULL, (size_t)seg_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (p == MAP_FAILED)
    {
        printf("Mapping Failed\n");
        return -1;
    }

    bitmap_size = seg_size / 64;
    reserved = 256;

    char *temp;
    temp = (char *)p;

    map_size = (seg_size - bitmap_size - reserved) / 8;

    for (int i = 0; i < map_size; i++)
    {
        temp[i] = 1;
    }

    cptr = temp;
    return 0;
}

void *dma_alloc(int size)
{
    pthread_mutex_lock(&lock);

    int count, reserved_size, start_ptr;
    int firstTime = 1;
    int occupied = 0;

    count = 0;

    char *status = NULL;
    char *temp = cptr;

    if (size % 16 != 0)
    {
        int dividing = size / 16;
        dividing++;
        reserved_size = dividing * 16;
    }

    int i;

    if (size <= seg_size)
    {

        for (i = 0; i < map_size - 1; i = i + 2)
        {

            if (temp[i] == 0)
            {
                if (temp[i + 1] == 1)
                {
                    if (count == reserved_size)
                    {
                        temp[start_ptr] = 0;
                        for (int x = start_ptr + 2; x < count; x++)
                        {
                            temp[x] = 0;
                        }
                        status = &((temp + ((bitmap_size + reserved) * 4))[start_ptr]);
                        printf("%p\n", status);
                        return status;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            else
            {
                if (temp[i + 1] == 1)
                {
                    if (firstTime)
                    {
                        start_ptr = i;
                        firstTime = 0;
                        occupied = 0;
                    }
                    else if (count == reserved_size)
                    {
                        temp[start_ptr] = 0;
                        for (int x = start_ptr + 2; x < count; x++)
                        {
                            temp[x] = 0;
                        }
                        status = &((temp + ((bitmap_size + reserved) * 4))[start_ptr]);
                        printf("%p\n", status);
                        return status;
                    }
                    count++;
                }
            }
        }
    }

    pthread_mutex_unlock(&lock);
    temp[start_ptr] = 0;
    for (int x = start_ptr + 2; x < count; x++)
    {
        temp[x] = 0;
    }
    status = &((temp + ((bitmap_size + reserved) * 4))[start_ptr]);
    printf("%p\n", status);
    return status;
}

void dma_free(void *p)
{
    pthread_mutex_lock(&lock);
    char *temp = cptr;
    int i;
    int start = 0;
    char *cstart;
    int x;
    char *travel = p;

    /* for (i = atoi(temp); i < (atoi(temp) + bitmap_size); i++)
    {
        if (temp == &travel[start])
        {
            if (start == 0)
            {
                cstart = temp;
            }
            start++;
            if (travel[start] == '\0')
            {
                break;
            }
        }
        else
        {
            start = 0;
            cstart = NULL;
        }
    }

    for (x = atoi(cstart); x < (atoi(cstart) + start); x++)
    {
        *cstart = *cstart | 0xff;
    } */
    pthread_mutex_unlock(&lock);
}

void dma_print_page(int pno)
{
    pthread_mutex_lock(&lock);
    char *temp = cptr;
    int i;
    int check = 1;
    char *cstart;

    for (i = atoi(temp); i < (atoi(temp) + bitmap_size); i++)
    {
        if (check == pno)
        {
            cstart = temp;
        }
        check++;
    }

    int x;

    for (x = atoi(cstart); x < (atoi(cstart) + bitmap_size); x++)
    {
        printf("%x", cstart[x] & 0xff);

        if (i % 64 == 0)
        {
            printf("\n");
        }
    }
    pthread_mutex_unlock(&lock);
}

void dma_print_bitmap()
{
    pthread_mutex_lock(&lock);
    int i;
    int mask;
    int size = seg_size / 8;
    char *temp = NULL;

    for (i = size; i >= 0; i--)
    {

        if (i % 8 == 0)
        {
            if (i % 64 == 0)
            {
                printf("\n");
            }
            else
            {
                printf(" ");
            }
        }

        mask = 0xff << i;

        if (*temp & mask)
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
        temp++;
    }
    pthread_mutex_unlock(&lock);
}

void dma_print_blocks()
{
    pthread_mutex_lock(&lock);
    int mask;
    int i;
    char *temp = cptr;
    for (i = atoi(temp); i < (atoi(temp) + bitmap_size); i++)
    {
        mask = 0xff << i;

        if (*temp & mask)
        {
            printf("F");
        }
        else
        {
            printf("A");
        }
        printf("%x", temp[i] & 0xff);
        printf("%lx (%lu)\n", sizeof(i) & 0xff, sizeof(i));
        printf("\n");
    }
    pthread_mutex_unlock(&lock);
}

int dma_give_intfrag()
{
    pthread_mutex_lock(&lock);
    int mask;
    int i;
    int empty = 0;
    char *temp = cptr;
    int max = 0;
    for (i = atoi(temp); i < (atoi(temp) + bitmap_size); i++)
    {
        mask = 0xff << i;

        if (*temp & mask)
        {
            empty++;
            if (max < sizeof(i))
            {
                max = sizeof(i);
            }
        }
    }
    pthread_mutex_unlock(&lock);
    return ((empty - max) / empty) * 100;
}
