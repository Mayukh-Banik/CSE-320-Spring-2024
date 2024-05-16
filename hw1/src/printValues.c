#include "helper.h"

#include <stdio.h>

void printNodeHeader() 
{
    fprintf(stderr, "%-18s %-18s %-18s %-18s %-10s %-10s\n", "Index", "Left", "Right", "Parent", "Weight", "Symbol");
}

void printNode(NODE *node) 
{
    if (node == NULL) {
        fprintf(stderr, "%-10d Node is NULL.\n", 0);
        return;
    }
    fprintf(stderr, "%-18ld %-18ld %-18ld %-18ld %-10d %-10d\n",
           node - nodes,
           node->left - nodes,
           node->right - nodes,
           node->parent - nodes,
           node->weight,
           node->symbol);
}

void printNodes() 
{
    printNodeHeader();
    for (int i = 0; i < 24; i++) 
    {
        printNode(nodes+i);
    }
	fprintf(stderr, "\n");
    fflush(stderr);
}

void pPrintTree()
{
    printNodeHeader();
    printTree(nodes);
}

void printTree(NODE* node)
{
    if (node == NULL)
    {
        return;
    }
    printTree(node->left);
    printTree(node->right);
    printNode(node);
}

void printSymbols()
{
    for (int i = 0; i < 10; i++)
    {
        printNode(*(node_for_symbol + i));
    }
    fflush(stdout);
}

void printCurrentBlock(int index)
{
    unsigned char* x = current_block;
    for (int i = 0; i < index - 1; i++)
    {
        fprintf(stderr, "%c", *x);
        x++;
    }
    fprintf(stderr, "\n");
    fflush(stderr);
}