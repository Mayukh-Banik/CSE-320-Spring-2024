/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include <errno.h>
#include <stdbool.h>
#include <helper.h>

void *sf_malloc(size_t size) 
{
    if (size <= 0)
    {
        sf_errno = size == 0 ? sf_errno : ENOMEM;
        return NULL;
    }
    sf_block* blockToReturn = NULL;
    size = makeNumberAligned(size + sizeof(sf_header));
    if (!isHeapInitialized())
    {
        if (!initializeHeap())
        {
            sf_errno = ENOMEM;
            return NULL;
        }
    }
    blockToReturn = handleQuickList(size);
    if (blockToReturn != NULL)
    {
        return blockToReturn->body.payload;
    }
    int minFreeListIndex = getFreeListIndex(size);
    while (minFreeListIndex < NUM_FREE_LISTS)
    {
        if (isThereAFreeBlockAtThatFreeList(minFreeListIndex))
        {
            break;
        }
        minFreeListIndex++;
    }
    if (minFreeListIndex == NUM_FREE_LISTS)
    {
        blockToReturn = handleNotEnoughSpaceInFreeLists(size);
        if (blockToReturn == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
        return blockToReturn->body.payload;
    }
    else
    {
        blockToReturn = popOffFreeList(minFreeListIndex);
        if (freeBlockCanBeSplit(blockToReturn, size))
        {
            blockToReturn = splitFreeBlock(blockToReturn, size);
            blockToReturn = makeFreeBlockAlloc(blockToReturn);
            return blockToReturn->body.payload;
        }
        else
        {
            blockToReturn = makeFreeBlockAlloc(blockToReturn);
            return blockToReturn->body.payload;
        }
    }
    sf_errno = ENOMEM;
    return NULL;
}

void sf_free(void *pp) 
{
    if (isPointerValidToBeFreed(pp) == false)
    {
        abort();
    }
    sf_block* block = (sf_block*) (pp - sizeof(sf_header) - sizeof(sf_footer));
    size_t size = unObfuscate(block->header) & ~0x7;
    int index = getQuickListIndex(size);
    if (index != -1)
    {
        insertAllocBlockIntoQuickList(block);
        return;
    }
    block = makeAllocBlockFree(block);
    insertFreeBlockIntoFreeLists(block);
    block = coalesceBlocks(block);
}

void *sf_realloc(void *pp, size_t rsize) 
{
    if (isPointerValidToBeFreed(pp) == false)
    {
        sf_errno = EINVAL;
        return NULL;
    }
    if (rsize == 0)
    {
        sf_free(pp);
        return NULL;
    }
    size_t totalSizeOfNewBlock = makeNumberAligned(rsize + sizeof(sf_header));
    sf_block* block = pp - sizeof(sf_header) - sizeof(sf_footer);
    size_t origSize = unObfuscate(block->header) & ~0x7;
    if (totalSizeOfNewBlock > origSize)
    {
        sf_block* temp = sf_malloc(rsize) - sizeof(sf_header) - sizeof(sf_footer);
        memcpy(temp->body.payload, pp, origSize);
        sf_free(pp);
        return temp->body.payload;
    }
    else
    {
        if ((origSize - totalSizeOfNewBlock) < 32)
        {
            return pp;
        }
        block->header = unObfuscate(block->header) & 0x7;
        block->header = block->header | totalSizeOfNewBlock;
        block->header = obfuscate(block->header);
        sf_block* temp = (void*)&block->header + totalSizeOfNewBlock - sizeof(sf_footer);
        temp->header = obfuscate((origSize - totalSizeOfNewBlock) | PREV_BLOCK_ALLOCATED | THIS_BLOCK_ALLOCATED);
        temp = makeAllocBlockFree(temp);
        insertFreeBlockIntoFreeLists(temp);
        temp = coalesceBlocks(temp);
        return block->body.payload;
    }
    return (void*) 0x0;
}

double sf_fragmentation() 
{
    return 0;
}
static double MAX_UTIL = 0;
double sf_utilization() 
{
    if (!isHeapInitialized())
    {
        return 0;
    }
    void* pointer = sf_mem_start() + 8;
    long double currUtil = 0;
    do
    {
        sf_header* head = pointer;
        if (unObfuscate(*head) & THIS_BLOCK_ALLOCATED)
        {
            currUtil = currUtil + (unObfuscate(*head) & ~0x7);
        }
        pointer = pointer + (unObfuscate(*head) & ~0x7);
    } while (pointer < (sf_mem_end() - 8));
    size_t currentHeapSize = sf_mem_end() - sf_mem_start();
    currUtil = currUtil / currentHeapSize;
    if (currUtil > MAX_UTIL)
    {
        MAX_UTIL = currUtil;
    }
    return MAX_UTIL;
}
