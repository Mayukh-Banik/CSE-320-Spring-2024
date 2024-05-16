#include "helper.h"
#include <stdio.h>

void zeroCurrentBlock()
{
    for (unsigned char* ptr = current_block; ptr < current_block + MAX_BLOCK_SIZE; ptr++) 
    {
        *ptr = 0x0;
    }
}

void wipeNodes()
{
    NODE* temp = nodes;
    for (int i = 0; i < 2 * MAX_SYMBOLS - 1; i++)
    {
        temp->left = NULL;
        temp->right = NULL;
        temp->parent = NULL;
        temp->weight = 0x0;
        temp->symbol = 0x0;
        temp++;
    }
}

void wipeAllNodeForSymbol()
{
    int i = 0;
    NODE** x = node_for_symbol;
    while (i < MAX_SYMBOLS)
    {
        *x = NULL;
        x++;
        i++;
    }
}

void zeroSymbols()
{
    for (int i = 0; i < MAX_SYMBOLS; i++)
    {
        (*(node_for_symbol + i)) = NULL;
    }
}