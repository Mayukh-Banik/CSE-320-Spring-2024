#include "sfmm.h"
#include <errno.h>
#include <stdbool.h>
#include "debug.h"

#ifndef HELPER_H
#define HELPER_H

/**
 * @brief Checks if the heap has been allocated space
 * 
 * @return true If the heap has been set
 * @return false If the heap hasn't been set
 */
bool isHeapInitialized();

/**
 * @brief Handles all logic for initializing the heap on first malloc call.
 * If growing the heap has an error that occurs, immediately returns without 
 * setting sf_errno under the assumption that it is set by the function.
 * Initializes Free Block Lists, sets initial Prologue and Epilogue, and 
 * 
 * @return true If no error occurs
 * @return false If error occurs
 */
bool initializeHeap();

/**
 * @brief  Xors input
 * 
 * @param val To be Xord
 * @return size_t Xord Value
 */
size_t obfuscate(size_t val);

/**
 * @brief  Xors input
 * 
 * @param val To be Xord
 * @return size_t Xord Value
 */
size_t unObfuscate(size_t val);

/**
 * @brief Follows instructions, essentially its name
 * 
 */
void initializeFreeBlocks();

/**
 * @brief Get the Free List Index object
 * 
 * @param size Of block to find index in the free list of 
 * @return int -1 on error or [0,9] for index.
 */
int getFreeListIndex(size_t size);

/**
 * @brief Set the Epilogue object, assumes previous block is unallocated and 
 * that it is marked as being allocated
 * 
 */
void setEpilogue();

/**
 * @brief Set the Prolouge object
 * 
 */
void setProlouge();

/**
 * @brief Creates the initial free block after the creation of the heap
 * 
 * @return sf_block* Mem address of the initial free block always sf_mem_start() + 32
 */
sf_block* makeInitialFreeBlock();

/**
 * @brief Set the Footer object according to the size of the block paased
 * 
 * @param block The free block to have its footer set
 */
sf_block* setFooter(sf_block* block);

/**
 * @brief Inserts a block into free lists, doesn't convert blocks, 
 * assumes pre processing on previous and next blocks in memory etc
 * 
 * @param block To be placed into free block
 * 
 * @returns Block inserted, NUll on error
 */
sf_block* insertFreeBlockIntoFreeLists(sf_block* block);

/**
 * @brief Makes size the aligned value
 * 
 * @param size 
 * @return size_t 
 */
size_t makeNumberAligned(size_t size);

/**
 * @brief Get the Quick List Index object
 * 
 * @param size Of quick list object
 * @return int Index or -1 if invalid
 */
int getQuickListIndex(size_t size);

/**
 * @brief Checks if the quick list at that index has any blocks in it.
 * 
 * @param index Of quick lists to check+..
 * +
 * @return true If there is a block
 * @return false If there are no blocks in that index
 */
bool isThereABlockInQuickListIndex(int index);

/**
 * @brief Removes first quick list block at that index
 * 
 * @param index Of quick lists to removed
 * @return sf_block* Of memory array, only its body.next is set to NULL nothing else is changed
 */
sf_block* popOffQuickList(int index);

/**
 * @brief Sets quick block bit to 0, this is obfuscationg safe
 * 
 * @param block To have quick block bit set to 0
 * @return sf_block* that was passed in, can be ignored.
 */
sf_block* makeQuickBlockAlloc(sf_block* block);

/**
 * @brief Checks if there is an available free block at the index passed to it
 * 
 * @param index Of free list to check
 * @return true If there is a block to be used there
 * @return false If there isn't a block to be used there
 */
bool isThereAFreeBlockAtThatFreeList(int index);

/**
 * @brief Checks if the given free block can be split
 * into two smaller blocks based on the size of the requested
 * block size. Eg. Can a free block with size of 100 be split into 
 * 2 blocks when 
 * 
 * @param block To be checked if it can be split
 * @param size Of the new block to be created, this is without the size of
 * the header to be included, eg the minimum requested size can be 32, so pass in 
 * 32 not 40. 
 * @return true If the block can be split   
 * @return false If it cannot
 */
bool freeBlockCanBeSplit(sf_block* block, size_t size);

/**
 * @brief Splits two free blocks. Makes them both free blocks. 
 * Inserts the second block. Does not insert the first block which is returned.
 * 
 * @param block 
 * @param size 
 * @return sf_block* 
 */
sf_block* splitFreeBlock(sf_block* block, size_t size);

/**
 * @brief Removes free block from list and sets its next and prev to NULL
 * Currently does not check if the block is marked as being free.
 * 
 * @param block To pop off
 * @return sf_block* That was popped off
 */
sf_block* removeBlockFromFreeList(sf_block* block);

/**
 * @brief Makes a free block allocated. Marks other pieces of memory as well.
 * 
 * @param block To make allocated
 * @return sf_block* That was marked as allocated.
 */
sf_block* makeFreeBlockAlloc(sf_block* block);

/**
 * @brief TODO
 * Set the Prev Alloc Bit Next Block object
 * 
 * @param block with to set the next block in memory as allocated
 * @param bit 1 to set PREV_BLOCK ALLOCATED bit as 1, 0 to set that bit to 0.
 */
void setPrevAllocBitNextBlock(sf_block* block, int bit);

/**
 * @brief Pops first free block from a list, assumes that there
 * is at least one block that isn't the header in there
 * 
 * @param index Of free lists to pop off
 * @return sf_block* That was popped off
 */
sf_block* popOffFreeList(int index);

/**
 * @brief Handles the quick list allocation part for sf_malloc
 * 
 * @param size Of block to be allocated
 * @return sf_block* To be returned to user. NULL if none available.
 */
sf_block* handleQuickList(size_t size);

/**
 * @brief Checks if the block passed is a free block
 * 
 * @param block To check if its freed
 * @return true If its a free block
 * @return false If its a quick/allocated block
 */
bool isBlockFree(sf_block* block);

/**
 * @brief If there isn't a block in the free lists for a block of the specified size it
 *  Grows the heap
 *  Makes a free block with all the new space
 *  Coalesces and removes the previous free block in memory, if able
 *  Inserts given block into the free lists
 *  Checks if the size is at least the size specified and if it is, returns. Does not pop off this 
 *  block from the quick list
 * 
 *  Assumes that it has already been determined that there isn't enough space.
 * 
 * @param size Of user requested memory
 * @return sf_block* To return
 */
sf_block* handleNotEnoughSpaceInFreeLists(size_t size);

/**
 * @brief Checks if the previos block in memory is free or not
 * 
 * @param block To check the previous of
 * @return true If its free
 * @return false If its not
 */
bool isPrevBlocFree(sf_block* block);

/**
 * @brief Get the Prev Block object, assuming the block in question is a free block
 * 
 * @param block 
 * @return sf_block* 
 */
sf_block* getPrevBlock(sf_block* block);

/**
 * @brief Removes both blocks from free lists if they're in one
 * Then merges them, does not insert back into free list
 * 
 * @param first 
 * @param second 
 * @return sf_block* 
 */
sf_block* coalesceTwoFreeBlocks(sf_block* first, sf_block* second);

/**
 * @brief Checks if the pointer the user passed to sf_free is valid
 * 
 * @param pointer 
 * @return true 
 * @return false 
 */
bool isPointerValidToBeFreed(void* pointer);

/**
 * @brief Given a sf_block pointer, it iteratively coalesces free
 * blocks starting from the previous block, eg if block sequence 1, 2, 3, 4, 5 occurs
 * and block 3 is freed, asssuming every block in that sequence is freed, it will 
 * coalesce with 2 & 3, then coalesce with 1, then coalesce with 4, then 5.
 * This will remove all blocks from free lists if they are in a free list, then make one 
 * super block which will not be inserted into a free list.
 * 
 * @note Order of coalescing: [1,2,3,4,5] -> [1,2+3,4,5] -> [1+2+3,4,5] -> [1+2+3+4,5] -> etc
 * @note block Must be in a free list
 * 
 * @param block To start the coalescing sequence
 * @return sf_block* That was coalesced, must be inserted manually.
 */
sf_block* coalesceBlocks(sf_block* block);

/**
 * @brief Get the Next Block object
 * 
 * @param block 
 * @return sf_block* 
 */
sf_block* getNextBlock(sf_block* block);

/**
 * @brief Makes an allocated block a quick block
 * 
 * @param block To mark quick
 * @return sf_block* That was passed
 */
sf_block* makeAllocBlockQuick(sf_block* block);

/**
 * @brief Takes in a block marked as allocated, and inserts it into a 
 * quick list. If quick list capacity is hit, starts that process.
 * 
 * @param block To be inserted
 */
sf_block* insertAllocBlockIntoQuickList(sf_block* block);

/**
 * @brief Just inserts a block into a quick list, no error checking
 * 
 * @param block 
 * @return sf_block* 
 */
sf_block* insertQuickList(sf_block* block, int index);

sf_block* makeQuickBlockFree(sf_block* block);

sf_block* makeAllocBlockFree(sf_block* block);

bool isNextBlockFree(sf_block* block);
































#endif