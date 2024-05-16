#ifndef HELPER_H
#define HELPER_H


#include "huff.h"
#include <stdio.h>
// #include <stdlib.h>
#include <limits.h>

#include "global.h"
#include "huff.h"
#include "debug.h"

void printNode(NODE *node);

void printNodes();

void printSymbols();

/**
 * @details Sets every unsigned char value to 0x0 in current_block up to MAX_BLOCK_SIZE
*/
void zeroCurrentBlock();

/**
 * @details Reads every char from stdin using fgetc and places it into current_block
 * as an unsigned char, casted from an int
 * @param CURRENT_BLOCK_SIZE The upper bound to read from STDIN
 * @return {1, 0, -1} 1 if EOF is reached, 0 if CURRENT_BLOCK_SIZE was reached,
 *  -1 if STDIN fgetc error
*/
int readIntoCurrentBlock(int CURRENT_BLOCK_SIZE);

/**
 * Sets all Pointers in nodes to NULL, and weight/symbol to 0x0
 * 
 * @return Nothing
*/
void wipeNodes();

/**
 * @details Checks if a Node with given symbol is in the nodes array
 * @param c Character to check if a node exists with that symbol
 * @return {1, 0} 1 if a node exists with that symbol, 0 otherwise
*/
int isSymbolInNodesArray(short c);

/**
 * @details Checks if a Node with given symbol is in the nodes array
 * @param c Character to check if a node exists with that symbol
 * @return {NODE*, NULL} NODE* of node with given symbol, NULL otherwise
*/
NODE* findNodeinNodes(short c);

/**
 * @param nd NODE reference to pass, give NULL and nothing will happen
 * @param update Value to update weight of node
 * @return {1, 0} 1 if a node exists with that symbol, 0 otherwise
*/
int updateWeightOfNode(NODE* nd, int update);

NODE* createNodeFromSymbol(short symbol);

/**
 * Converts current_block chars to nodes
*/
void currBlockToNodes(int check);

/**
 * Makes all pointers in Symbol null
*/
void wipeAllNodeForSymbol();

int userPassedValidBlockSize(char* str);
int userBlockSize(char* str);
int isValidOption(char Option);

/**
 * Returns number of nodes currently in nodes array
*/
int returnNumOfNodes();

void initBuildHuffmanTree();

int split(NODE* nodeArray, int low, int high);

void sort(NODE* nodes, int low, int high);

void sortNodes(NODE* nodes, int maxSymbols);

void postOrderSetParent(NODE* node);

/**
 * Puts nodes into symbol array, postorder style
*/
void convertNodesToSymbol(NODE* node);

void zeroSymbols();

void makeWeight(NODE* node, int value);

void setWeightsAndTraverse(NODE* root, int parentWeight);

void huffmanTreeToBitOrderInPostOrderForm(NODE* node, unsigned int *bitAccumulator, int *bitCount);
void outputByte(unsigned int byte);
void padAndOutputLastByte(unsigned int bitAccumulator, int bitCount);
void outputShortBytes(short num);
void putCharactersInHuffmanTreeIntoStdoutInPostOrder(NODE* node);
void postOrderSetWeightsIfLeafOrInner(NODE* node);
void currentBlockToSTDOUT(int check);

int dReadNumNodesFromStartOfHuffmanEncoding();

int getBitAsIntFromByte(unsigned int index, unsigned char startingChar);

int buildTreeFromDescription(int nodesLength);

int setIndexOfNodesArray(unsigned int index, NODE tempNode);

void pPrintTree();
void printTree(NODE* node);

void dNodeLeafsToSymbols(NODE *root);
void dNodeLeafsToSymbolsHelper(NODE *root, int *index);
int dReadCharactersToSymbols();

int dPrintOutValues();
void swapNodesBasedOnIndex(int a, int b);
void printCurrentBlock(int index);
#endif