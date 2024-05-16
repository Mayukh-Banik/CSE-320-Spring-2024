#include "helper.h"
#include <stdio.h>

int dReadNumNodesFromStartOfHuffmanEncoding()
{
    int Nnodes = 0x0;
    {
        int part1 = fgetc(stdin);
        if (part1 == EOF)
        {
            return -1;
        }
        // fprintf(stderr, "Part1: %d\n", part1);
        int part2 = fgetc(stdin);
        if (part2 == EOF)
        {
            return -1;
        }
        // fprintf(stderr, "Part2: %d\n", part2);
        Nnodes = (part1 << 8) | part2;
    }
    // fprintf(stderr, "Nnodes: %d\n", Nnodes);
    // fflush(stderr);

    return Nnodes;
}

int getBitAsIntFromByte(unsigned int index, unsigned char startingChar)
{
    if (index > 7)
    {
        return -1; 
    }
    unsigned char bit = (startingChar >> (8 - index - 1)) & 0x1;
    return (int) bit; 
}

int buildTreeFromDescription(int nodesLength)
{
    if (nodesLength <= 0)
    {
        return -1;
    }
    int numberOfBitsRead = 0x0;
    int c = 0x0;
    int currentBit = 0x0;
    int stackIndex = 0x0;
    int freeNodeIndex = 2 * MAX_SYMBOLS - 1;
    while (numberOfBitsRead < nodesLength)
    {
        if (numberOfBitsRead % 8 == 0)
        {
            if ((c = fgetc(stdin)) == EOF)
            {
                return -1;
            }
        }
        currentBit = getBitAsIntFromByte((unsigned int) numberOfBitsRead % 8, c);
        // fprintf(stderr, "Current Bit: %d, Bit Read: %d\n", currentBit, numberOfBitsRead);
        if (currentBit == 0x0)
        {
            NODE temp = {.left = NULL, .right = NULL, .parent = NULL, 
                .weight = 0x0, .symbol = 0x0};
            setIndexOfNodesArray(stackIndex, temp);
            stackIndex++;
        }
        else
        {
            freeNodeIndex--;
            int indexOfRightChild = freeNodeIndex;
            freeNodeIndex--;
            int indexOfLeftChild= freeNodeIndex;
            swapNodesBasedOnIndex(stackIndex - 1, indexOfRightChild);
            swapNodesBasedOnIndex(stackIndex - 2, indexOfLeftChild);
            // printNode(nodes + indexOfLeftChild);
            // printNode(nodes + indexOfRightChild);

            NODE temp = {.left = nodes + indexOfLeftChild, .right = nodes + indexOfRightChild,
                .parent = NULL, .weight = 0x1, .symbol = 0x0};
            stackIndex--;
            // stackIndex--;
            setIndexOfNodesArray(stackIndex - 1, temp);
        }

        numberOfBitsRead++;
    }
    // pPrintTree();
    return 0;
}

int setIndexOfNodesArray(unsigned int index, NODE tempNode)
{
    if (index > 2 * MAX_SYMBOLS - 1)
    {
        return -1;
    }
    NODE* nodesPointer = nodes + index;
    nodesPointer->left = tempNode.left;
    nodesPointer->right = tempNode.right;
    nodesPointer->parent = tempNode.parent;
    nodesPointer->weight = tempNode.weight;
    nodesPointer->symbol = tempNode.symbol;
    // printNode(nodesPointer);
    return 0;
}

void dNodeLeafsToSymbols(NODE *root) 
{
    int index = 0; 
    dNodeLeafsToSymbolsHelper(root, &index);
}

void dNodeLeafsToSymbolsHelper(NODE *root, int *index) 
{
    if (root == NULL) 
    {
        return; 
    }
    dNodeLeafsToSymbolsHelper(root->left, index);
    dNodeLeafsToSymbolsHelper(root->right, index);
    if (root->left == NULL && root->right == NULL) 
    {
        *(node_for_symbol + *index) = root;
        (*index)++;
    }
}

int dReadCharactersToSymbols()
{
    int currChar = 0x0;
    NODE** pointer = node_for_symbol;
    while (*pointer != NULL)
    {
        if ((currChar = fgetc(stdin)) == EOF)
        {
            return -1;
        }
        if ((unsigned char) currChar == 0xFF)
        {
            if ((currChar = fgetc(stdin)) == EOF)
            {
                return -1;
            }
            if (currChar == 0x0)
            {
                (*pointer)->symbol = -1;
            }
            else
            {
                ungetc(currChar, stdin);
                (*pointer)->symbol = 0xFF;
            }
        }
        else
        {
            (*pointer)->symbol = currChar;
        }
        pointer++;
    }
    return 0;
}

int dPrintOutValues()
{
    int currChar = 0x0;
    NODE* currNode = nodes;
    int currentBit = 0x0;
    unsigned int count = 0x0;
    unsigned int MAX = 0x0;
    while (MAX < UINT_MAX)
    {
        if (count % 8 == 0)
        {
            count = 0;
            if ((currChar = fgetc(stdin)) == EOF)
            {
                return -1;
            }
            MAX++;
        }
        currentBit = getBitAsIntFromByte(count % 8, currChar);
        if (currentBit == 0)
        {
            currNode = currNode->left;
            if (currNode->left == NULL && currNode->right == NULL)
            {
                if (currNode->symbol == -1)
                {
                    return 0;
                }
                fputc(currNode->symbol, stdout);
                currNode = nodes;
            }
        }
        if (currentBit == 1)
        {
            currNode = currNode->right;
            if (currNode->left == NULL && currNode->right == NULL)
            {
                if (currNode->symbol == -1)
                {
                    return 0;
                }
                fputc(currNode->symbol, stdout);
                currNode = nodes;
            }
        }
        count++;
    }
    return -1;
}