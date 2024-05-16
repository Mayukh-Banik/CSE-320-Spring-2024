#include "helper.h"
#include "sfmm.h"

#define MINIMUM_BLOCK_SIZE 32
#define ALIGNMENT 16

bool isHeapInitialized()
{
    return !(sf_mem_start() == sf_mem_end());
}

bool initializeHeap()
{
    if (sf_mem_grow() == NULL)
    {
        return false;
    }
    initializeFreeBlocks();
    setProlouge();
    setEpilogue();
    sf_block* initFreeBlock = makeInitialFreeBlock();
    return !(insertFreeBlockIntoFreeLists(initFreeBlock) == NULL);
}

void initializeFreeBlocks()
{
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }
}

int getFreeListIndex(size_t size)
{
    int M = 32;
    if (size < 0)
    {
        return -1;
    }
    if (size <= 32)
    {
        return 0;
    }
    if (size > M && size <= 2 * M)
    {
        return 1;
    }
    else if (size > 2 * M && size <= 4 * M)
    {
        return 2;
    }
    else if (size > 4 * M && size <= 8 * M)
    {
        return 3;
    }
    else if (size > 8 * M && size <= 16 * M)
    {
        return 4;
    }
    else if (size > 16 * M && size <= 32 * M)
    {
        return 5;
    }
    else if (size > 32 * M && size <= 64 * M)
    {
        return 6;
    }
    else if (size > 64 * M && size <= 128 * M)
    {
        return 7;
    }
    else if (size > 128 * M && size <= 256 * M)
    {
        return 8;
    }
    else 
    {
        return 9;
    }
}

size_t obfuscate(size_t val)
{
    return val ^ MAGIC;
}

size_t unObfuscate(size_t val)
{
    return val ^ MAGIC;
}

void setEpilogue()
{
    sf_header* block = sf_mem_end() - 8;
    *block = obfuscate(THIS_BLOCK_ALLOCATED);
}

void setProlouge()
{
    sf_block* block = sf_mem_start();
    block->prev_footer = 0x0;
    block->header = obfuscate(MINIMUM_BLOCK_SIZE | PREV_BLOCK_ALLOCATED | THIS_BLOCK_ALLOCATED);
}

sf_block* makeInitialFreeBlock()
{
    sf_block* block = sf_mem_start() + MINIMUM_BLOCK_SIZE;
    size_t y = sf_mem_end() - sizeof(sf_header) - (void*)&block->header;
    block->header = obfuscate(y + PREV_BLOCK_ALLOCATED);
    block->body.links.next = NULL;
    block->body.links.prev = NULL;
    setFooter(block);
    return block;
}

sf_block* setFooter(sf_block* block)
{
    if (block == NULL)
    {
        return NULL;
    }
    size_t size = unObfuscate(block->header) & ~0x7;
    sf_footer* foot = (sf_footer*)((char*)&(block->header) + size - sizeof(sf_footer));
    if ((void*)foot > sf_mem_end() || (void*)foot < sf_mem_start())
    {
        return NULL;
    }
    *foot = block->header;
    return block;
}

sf_block* insertFreeBlockIntoFreeLists(sf_block* block)
{
    if (block == NULL)
    {
        return NULL;
    }
    size_t blockSize = unObfuscate(block->header) & ~0x7;
    int index = getFreeListIndex(blockSize);
    if (index == -1)
    {
        return NULL;
    }
    if (sf_free_list_heads[index].body.links.next == &sf_free_list_heads[index])
    {
        sf_free_list_heads[index].body.links.next = block;
        sf_free_list_heads[index].body.links.prev = block;
        block->body.links.next = &sf_free_list_heads[index];
        block->body.links.prev = &sf_free_list_heads[index];
    }
    else
    {
        block->body.links.next = sf_free_list_heads[index].body.links.next;
        block->body.links.prev = &sf_free_list_heads[index];
        sf_free_list_heads[index].body.links.next->body.links.prev = block;
        sf_free_list_heads[index].body.links.next = block;
    }
    return block;
}

size_t makeNumberAligned(size_t size)
{
    return size <= MINIMUM_BLOCK_SIZE ? MINIMUM_BLOCK_SIZE : ((size + 15) / 16) * 16;
}

int getQuickListIndex(size_t size)
{
    for (int i = 0; i < NUM_QUICK_LISTS; i++)
    {
        if (size == (MINIMUM_BLOCK_SIZE + (i * ALIGNMENT)))
        {
            return i;
        }
    }
    return -1;
}

sf_block* popOffQuickList(int index)
{
    if (index < 0 || index >= NUM_QUICK_LISTS ||
        sf_quick_lists[index].length == 0 || sf_quick_lists[index].first == NULL)
    {
        return NULL;
    }
    sf_block* block = sf_quick_lists[index].first;
    sf_quick_lists[index].first = block->body.links.next;
    sf_quick_lists[index].length = sf_quick_lists[index].length - 1;
    block->body.links.next = NULL;
    return block;
}

sf_block* makeQuickBlockAlloc(sf_block* block)
{
    if (block == NULL)
    {
        return NULL;
    }
    block->header = obfuscate(unObfuscate(block->header) & ~IN_QUICK_LIST);
    return block;
}

bool isThereAFreeBlockAtThatFreeList(int index)
{
    if (index >= NUM_FREE_LISTS || index < 0)
    {
        return false;
    }
    if (sf_free_list_heads[index].body.links.next != &sf_free_list_heads[index] &&
        sf_free_list_heads[index].body.links.prev != &sf_free_list_heads[index])
    {
        return true;
    }
    return false;
}

bool freeBlockCanBeSplit(sf_block* block, size_t size)
{
    if (block == NULL)
    {
        return false;
    }
    size_t originalBlockSize = unObfuscate(block->header) & ~0x7;
    return !((originalBlockSize - size) < MINIMUM_BLOCK_SIZE);
}

sf_block* splitFreeBlock(sf_block* block, size_t size) 
{
    if (block == NULL)
    {
        sf_errno = EINVAL;
        return NULL;
    }
    size_t origSize = unObfuscate(block->header) & ~0x7;
    sf_block* remainder = (void*)block + size;
    remainder->header = origSize - size;
    remainder->header = remainder->header & ~THIS_BLOCK_ALLOCATED;
    remainder->header = remainder->header & ~IN_QUICK_LIST;
    remainder->header = obfuscate(remainder->header);
    setFooter(remainder);
    insertFreeBlockIntoFreeLists(remainder);
    block->header = unObfuscate(block->header) & 0x7;
    block->header = size | block->header;
    block->header = block->header & ~THIS_BLOCK_ALLOCATED;
    block->header = block->header & ~IN_QUICK_LIST;
    block->header = obfuscate(block->header);
    setFooter(block);
    return block;
}

sf_block* removeBlockFromFreeList(sf_block* block)
{
    if (block == NULL)
    {
        return NULL;
    }
    int index = getFreeListIndex(unObfuscate(block->header) & ~0x7);
    sf_block* temp = &sf_free_list_heads[index];
    do
    {
        if (temp == block)
        {
            temp->body.links.prev->body.links.next = temp->body.links.next;
            temp->body.links.next->body.links.prev = temp->body.links.prev;
            temp->body.links.next = NULL;
            temp->body.links.prev = NULL;
            return block;
        }
        temp = temp->body.links.next;
    } 
    while (temp != &sf_free_list_heads[index]);
    return NULL;
}

sf_block* makeFreeBlockAlloc(sf_block* block)
{
    if (block == NULL)
    {
        return NULL;
    }
    block->header = unObfuscate(block->header);
    block->header = block->header | THIS_BLOCK_ALLOCATED;
    block->header = obfuscate(block->header);
    setPrevAllocBitNextBlock(block, 1);
    return block;
}

void setPrevAllocBitNextBlock(sf_block* block, int bit)
{
    if (block == NULL)
    {
        return;
    }
    size_t origBlockSize = unObfuscate(block->header) & ~0x7;
    sf_block* newBlock = (sf_block*) ((void*)block + origBlockSize);
    sf_header bb = unObfuscate(newBlock->header);
    bb = bit == 0 ? bb & ~PREV_BLOCK_ALLOCATED : bb | PREV_BLOCK_ALLOCATED;
    newBlock->header = obfuscate(bb);
    if (isBlockFree(newBlock))
    {
        setFooter(newBlock);
    }
}

sf_block* popOffFreeList(int index)
{
    if (index < 0 || index >= NUM_FREE_LISTS)
    {
        return NULL;
    }
    sf_block* temp = &sf_free_list_heads[index];
    sf_block* block = temp->body.links.next;
    temp->body.links.next = block->body.links.next;
    block->body.links.next->body.links.prev = temp;
    block->body.links.next = NULL;
    block->body.links.prev = NULL;
    return block;
}

sf_block* handleQuickList(size_t size)
{
    int index = getQuickListIndex(size);
    if (index != -1)
    {
        sf_block* block = popOffQuickList(index);
        if (block == NULL)
        {
            return NULL;
        }
        else
        {
            return makeQuickBlockAlloc(block);
        }
    }
    return NULL;
}

bool isBlockFree(sf_block* block)
{
    if (block == NULL)
    {
        return false;
    }
    sf_header head = unObfuscate(block->header);
    if (head & THIS_BLOCK_ALLOCATED)
    {
        return false;
    }
    return true;
}

sf_block* handleNotEnoughSpaceInFreeLists(size_t size)
{
    sf_block* block = (sf_block*) (sf_mem_end() - sizeof(sf_header) - sizeof(sf_footer));
    if (isPrevBlocFree(block))
    {
    }
    else
    {
        if (sf_mem_grow() == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
        setEpilogue();
        block->header = unObfuscate(block->header);
        block->header = block->header & 0x7;
        block->header = block->header + PAGE_SZ;
        block->header = block->header & ~THIS_BLOCK_ALLOCATED;
        block->header = obfuscate(block->header);
        setFooter(block);
        insertFreeBlockIntoFreeLists(block);
        if ((unObfuscate(block->header) & ~0x7) >= size)
        {
            if (freeBlockCanBeSplit(block, size))
            {
                block = removeBlockFromFreeList(block);
                block = splitFreeBlock(block, size);
                block = makeFreeBlockAlloc(block);
                return block;
            }
            block = removeBlockFromFreeList(block);
            block = makeFreeBlockAlloc(block);
            return block;
        }
        block = (sf_block*) (sf_mem_end() - sizeof(sf_header) - sizeof(sf_footer));
    }
    block = getPrevBlock(block);
    size_t sizeOfTemp = unObfuscate(block->header) & ~0x7;
    size_t sizeNeeded = size - sizeOfTemp;
    sizeNeeded = (sizeNeeded + PAGE_SZ - 1) / PAGE_SZ;
    while (sizeNeeded > 0)
    {
        if (sf_mem_grow() == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
        block = removeBlockFromFreeList(block);
        block->header = unObfuscate(block->header);
        block->header = block->header + PAGE_SZ;
        block->header = obfuscate(block->header);
        setFooter(block);
        if (insertFreeBlockIntoFreeLists(block) == NULL)
        {
            sf_errno = EINVAL;
            return NULL;
        }
        setEpilogue();
        sizeNeeded--;
    }
    block = removeBlockFromFreeList(block);
    if (freeBlockCanBeSplit(block, size))
    {
        block = splitFreeBlock(block, size);
        block = makeFreeBlockAlloc(block);
        return block;
    }
    makeFreeBlockAlloc(block);
    return block;
}

bool isPrevBlocFree(sf_block* block)
{
    if (block == NULL)
    {
        return NULL;
    }
    sf_header header = unObfuscate(block->header);
    if (header & PREV_BLOCK_ALLOCATED)
    {
        return false;
    }
    return true;
}

sf_block* getPrevBlock(sf_block* block)
{
    if (block == NULL || ((void*)block < sf_mem_start()) || ((void*)block > sf_mem_end()))
    {
        return NULL;
    }
    size_t prev_block_size = unObfuscate(block->prev_footer) & ~0x7;
    sf_block* prev_block = (sf_block*)((char*)block - prev_block_size);
    return prev_block;
}

sf_block* coalesceTwoFreeBlocks(sf_block* first, sf_block* second)
{
    if (first == NULL || second == NULL)
    {
        return NULL;
    }
    removeBlockFromFreeList(first);
    removeBlockFromFreeList(second);
    first->header = unObfuscate(first->header);
    second->header = unObfuscate(second->header);
    size_t firstSize = first->header & ~0x7;
    size_t secondSize = second->header & ~0x7;
    first->header = first->header & 0x7;
    firstSize = firstSize + secondSize;
    first->header = first->header | firstSize;
    first->header = obfuscate(first->header);
    setFooter(first);
    return first;
}

bool isPointerValidToBeFreed(void* pointer)
{
    if (pointer == NULL)
    {
        return false;
    }
    if ((size_t) pointer % 16 != 0)
    {
        return false;
    }
    sf_block* block = pointer - sizeof(sf_header) - sizeof(sf_footer);
    sf_header header = unObfuscate(block->header);
    size_t size = header & ~0x7;
    if (size < 32)
    {
        return false;
    }
    if (size % 16 != 0)
    {
        return false;
    }
    if ( (void*)&block->header < (sf_mem_start() + 40))
    {
        return false;
    }
    void* footer = ((char *)&(block->header) + size - sizeof(sf_footer));
    if (footer > (sf_mem_end() - sizeof(sf_header)))
    {
        return false;
    }
    if (header & IN_QUICK_LIST)
    {
        return false;
    }
    if ((header & THIS_BLOCK_ALLOCATED) == 0)
    {
        return false;
    }
    if ((header & PREV_BLOCK_ALLOCATED) == 0)
    {
        sf_footer foot= unObfuscate(block->prev_footer);
        if (foot & THIS_BLOCK_ALLOCATED)
        {
            return false;
        }
    }
    return true;
}

sf_block* coalesceBlocks(sf_block* block)
{
    while (isPrevBlocFree(block))
    {
        sf_block* prev = getPrevBlock(block);
        block = coalesceTwoFreeBlocks(prev, block);
        block = insertFreeBlockIntoFreeLists(block);
    }
    while (isNextBlockFree(block))
    {
        sf_block* next = getNextBlock(block);
        block = coalesceTwoFreeBlocks(block, next);
        block = insertFreeBlockIntoFreeLists(block);
    }
    return block;
}

sf_block* getNextBlock(sf_block* block)
{
    if (block == NULL)
    {
        return NULL;
    }
    size_t body_size = unObfuscate(block->header) & ~0x7;
    sf_block* bb = (sf_block*)((char*)&(block->header) + body_size - sizeof(sf_footer));
    if (bb == NULL || (void*)bb > sf_mem_end() || (void*)bb < sf_mem_start())
    {
        return NULL;
    }
    return bb;
}

sf_block* makeAllocBlockQuick(sf_block* block)
{
    block->header = unObfuscate(block->header);
    block->header = block->header | IN_QUICK_LIST;
    block->header = obfuscate(block->header);
    return block;
}

sf_block* insertAllocBlockIntoQuickList(sf_block* block)
{
    size_t size = unObfuscate(block->header) & ~0x7;
    int index = getQuickListIndex(size);
    if (index < 0 || index > (NUM_QUICK_LISTS - 1))
    {
        return NULL;
    }
    if (sf_quick_lists[index].length >= QUICK_LIST_MAX)
    {
        // Pop all and coalesce
        while (sf_quick_lists[index].length != 0)
        {
            sf_block* temp = popOffQuickList(index);
            temp = makeQuickBlockFree(block);
            temp = insertFreeBlockIntoFreeLists(block);
            temp = coalesceBlocks(block);
            temp = insertFreeBlockIntoFreeLists(temp);
        }
    }
    block = makeAllocBlockQuick(block);
    insertQuickList(block, index);
    return block;
}

sf_block* insertQuickList(sf_block* block, int index)
{
    block->body.links.next = sf_quick_lists[index].first;
    sf_quick_lists[index].length = sf_quick_lists[index].length + 1;
    sf_quick_lists[index].first = block;
    return block;
}

sf_block* makeQuickBlockFree(sf_block* block)
{
    block->header = unObfuscate(block->header);
    block->header = block->header & ~THIS_BLOCK_ALLOCATED;
    block->header = block->header & ~IN_QUICK_LIST;
    block->header = obfuscate(block->header);
    setFooter(block);
    sf_block* temp = getNextBlock(block);
    temp->header = unObfuscate(temp->header) & ~PREV_BLOCK_ALLOCATED;
    temp->header = obfuscate(temp->header);
    if (isBlockFree(temp))
    {
        setFooter(temp);
    }
    return block;
}

sf_block* makeAllocBlockFree(sf_block* block)
{
    block->header = unObfuscate(block->header);
    block->header = block->header & ~THIS_BLOCK_ALLOCATED;
    block->header = obfuscate(block->header);
    setFooter(block);
    sf_block* temp = getNextBlock(block);
    temp->header = unObfuscate(temp->header) & ~PREV_BLOCK_ALLOCATED;
    temp->header = obfuscate(temp->header);
    if (isBlockFree(temp))
    {
        setFooter(temp);
    }
    return block;
}

bool isNextBlockFree(sf_block* block)
{
    if (block == NULL)
    {
        return false;
    }
    size_t body_size = unObfuscate(block->header) & ~0x7;
    sf_block* bb = (sf_block*)((char*)&(block->header) + body_size - sizeof(sf_footer));
    if (bb == NULL || (void*)bb > sf_mem_end())
    {
        sf_errno = EINVAL;
        return false;
    }
    sf_header head = unObfuscate(bb->header);
    return (head & THIS_BLOCK_ALLOCATED) == 0;
}





















#undef MINIMUM_BLOCK_SIZE
#undef ALIGNMENT