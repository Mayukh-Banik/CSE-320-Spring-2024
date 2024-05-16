#include "helper.h"
#include <stdio.h>

int readIntoCurrentBlock(int CURRENT_BLOCK_SIZE)
{
    int c = 0x0;
    // void *tPointer = current_block;
    int i = 0;
    for (i = 0; i < CURRENT_BLOCK_SIZE; i++)
    {
        c = fgetc(stdin);
        if (c == EOF) 
        {
            if (feof(stdin)) 
            {
				return i + 1;
            } 
			else if (ferror(stdin)) 
			{
				return -1;
            }
            break;
        }
        *(current_block + i) = c;
		// *tPointer = c;
        // tPointer++;
    }
    // debug("Last Character Read: %d\n", c);
    // fprintf(stdout, "Last Character Read: %d\n", c);
    // fflush(stdout);
    return i + 1;
}

int isSymbolInNodesArray(short c)
{
    NODE* temp = nodes;
    for (int i = 0; i < 2 * MAX_SYMBOLS - 1; i++)
    {
        if (temp->symbol == c)
        {
            return 1;
        }
        temp++;
    }
    return 0;
}

NODE* findNodeinNodes(short c)
{
    NODE* temp = nodes;
    for (int i = 0; i < 2 * MAX_SYMBOLS - 1; i++)
    {
        if (temp->symbol == c)
        {
            return temp;
        }
        temp++;
    }
    return NULL;
}

int updateWeightOfNode(NODE* nd, int update)
{
    if (nd == NULL || nd->symbol == -1)
    {
        return 0;
    }
    nd->weight = nd->weight + update;
    return 1;
}

NODE* createNodeFromSymbol(short symbol)
{
    NODE* temp = nodes;
    for (int i = 0; i < 2 * MAX_SYMBOLS - 1; i++)
    {
        if (temp->symbol == 0x0)
        {
            temp->symbol = symbol;
            if (updateWeightOfNode(temp, 1) == 0)
            {
                return NULL;
            }
            return temp;
        }
        temp++;
    }
    return NULL;
}
// add check
void currBlockToNodes(int check)
{
    // unsigned char* temp = current_block;
    int i = 0;
    int c = *current_block;
    while (i < check)
    {
        if (!isSymbolInNodesArray((short) c))
        {
            createNodeFromSymbol((short) c);
        }
        else
        {
            updateWeightOfNode(findNodeinNodes((short) c), 1);
        }
        // temp++;
        c = *(current_block + i);
        i++;
    }
    NODE* end = createNodeFromSymbol((short) 0x0);
    end->weight = 0x0;
    end->symbol = -1;
}

int returnNumOfNodes()
{
    int nNodes = 0;
    NODE* x = nodes;
    for (int i = 0; i < 2*MAX_SYMBOLS - 1; i++)
    {
        if (x->weight == 0x0 && x->symbol == 0x0)
        {

        }
        else
        {
            nNodes = nNodes + 1;
        }
        x++;
    }
    return nNodes;
}

void swapNodesBasedOnIndex(int a, int b)
{
    NODE* nodeA = nodes + a;
    NODE* nodeB = nodes + b;
    NODE temp = {.parent = nodeB->parent, .left = nodeB->left, .right = nodeB->right,
                    .weight = nodeB->weight, .symbol = nodeB->symbol};
    nodeB->left = nodeA->left;
    nodeB->right = nodeA->right;
    nodeB->parent = nodeA->parent;
    nodeB->weight = nodeA->weight;
    nodeB->symbol = nodeA->symbol;
    nodeA->left = temp.left;
    nodeA->right = temp.right;
    nodeA->parent = temp.parent;
    nodeA->weight = temp.weight;
    nodeA->symbol = temp.symbol;
}

void shiftEveryOtherNode(int N)
{
	for (int i = 1; i < N - 1; i++)
	{
		swapNodesBasedOnIndex(i, i+1);
	}
    return;
}

int split(NODE* nodeArray, int low, int high) 
{
    NODE pivot = *(nodeArray + high);
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) 
	{
        if ((*(nodeArray + j)).weight < pivot.weight) 
		{
            i++; 
            NODE temp = *(nodeArray + i);
            *(nodeArray + i) = *(nodeArray + j);
            *(nodeArray + j) = temp;
        }
    }
    NODE temp = *(nodeArray + i + 1); 
    *(nodeArray + i + 1) = *(nodeArray + high);
    *(nodeArray + high) = temp;
    return (i + 1);
}

void sort(NODE* nodes, int low, int high) 
{
    if (low < high) 
    {
        if ((*(nodes + low)).weight == 0 && (*(nodes + low)).symbol == 0) 
        {
            return;
        }
        int pi = split(nodes, low, high);
        sort(nodes, low, pi - 1);
        sort(nodes, pi + 1, high);
    }
}

void sortNodes(NODE* nodes, int maxSymbols) 
{
    int n = 0;
    while (n < 2 * maxSymbols - 1 && !((*(nodes + n)).weight == 0 && (*(nodes + n)).symbol == 0)) 
	{
        n++;
    }
    sort(nodes, 0, n - 1);
}

void initBuildHuffmanTree()
{
    int N = num_nodes;
	sortNodes(nodes, MAX_SYMBOLS);
    while (1)
    {
		int a = (2*N) - 2;
		int b = (2*N) - 3;
        swapNodesBasedOnIndex(0, a);
        swapNodesBasedOnIndex(1, b);
        nodes->right = nodes + a;
        nodes->left = nodes + b;
        nodes->weight = (nodes + a)->weight + (nodes + b)->weight;
        shiftEveryOtherNode(N);
		N--;
		if (N < 2)
		{
			nodes->symbol = 0;
			break;
		}
        sortNodes(nodes, N);
    }
	num_nodes = num_nodes * 2 - 1;
    nodes->parent = NULL;
    postOrderSetParent(nodes);
    convertNodesToSymbol(nodes);
}

void postOrderSetParent(NODE* node)
{
    if (node == NULL || (node->symbol == 0x0 && node->weight == 0x0))
    {
        return;
    }
    if (node->left != NULL && (node->left->symbol != 0x0 || node->left->weight != 0x0))
    {
        node->left->parent = node;
        postOrderSetParent(node->left);
    }
    if (node->right != NULL && (node->right->symbol != 0x0 || node->right->weight != 0x0))
    {
        node->right->parent = node;
        postOrderSetParent(node->right);
    }
    return;
}

int firstEmptyValueInSymbols()
{
    for (int i = 0; i < MAX_SYMBOLS; i++)
    {
        if ((*(node_for_symbol + i)) == NULL)
        {
            return i;
        }
    }
    return -1;
}

void convertNodesToSymbol(NODE* node)
{
    if (node == NULL || (node->symbol == 0x0 && node->weight == 0x0))
    {
        return;
    }
    if (node->left != NULL && (node->left->symbol != 0x0 || node->left->weight != 0x0))
    {
        convertNodesToSymbol(node->left);
    }
    if (node->right != NULL && (node->right->symbol != 0x0 || node->right->weight != 0x0))
    {
        convertNodesToSymbol(node->right);
    }
    int a;
    if ((a = firstEmptyValueInSymbols()) == -1)
    {
        return;
    }
    if (node->left == NULL && node->right == NULL)
    {
        (*(node_for_symbol + a)) = node;
    }
    return;
}

int setBit(int num, int index)
{
    return num | (1 << index);
}

void makeWeight(NODE* node, int value)
{
    if (node == NULL)
    {
        return;
    }
    node->weight = value;
    if (node->left != NULL)
    {
        makeWeight(node->left, (value << 1) + 0);
    }
    if (node->right != NULL)
    {
        makeWeight(node->left, (value << 1) + 1);
    }
    return;
}

void setWeightsAndTraverse(NODE* node, int parentWeight) 
{
    if (node == NULL) {
        return;
    }
    if (node->parent != NULL) 
    {
        if (node->parent->left == node) 
        {
            node->weight = parentWeight << 1;
        } 
        else if (node->parent->right == node) 
        {
            node->weight = (parentWeight << 1) | 1;
        }
    } 
    else 
    {
        node->weight = 0;
    }
    setWeightsAndTraverse(node->left, node->weight);
    setWeightsAndTraverse(node->right, node->weight);
}

void huffmanTreeToBitOrderInPostOrderForm(NODE* node, unsigned int *bitAccumulator, int *bitCount) 
{
    // fprintf(stderr, "Hello\n");
    // fflush(stderr);
    if (node == NULL) 
    {
        return;
    }
    huffmanTreeToBitOrderInPostOrderForm(node->left, bitAccumulator, bitCount);
    huffmanTreeToBitOrderInPostOrderForm(node->right, bitAccumulator, bitCount);
    if (node->left || node->right) 
    { 
        *bitAccumulator = (*bitAccumulator << 1) | 1;
    } 
    else 
    { 
        *bitAccumulator = (*bitAccumulator << 1);
    }
    (*bitCount)++;

    if (*bitCount == 8) 
    {
        outputByte(*bitAccumulator);
        *bitAccumulator = 0;
        *bitCount = 0;
    }
}

void outputByte(unsigned int byte) 
{
    fputc(byte, stdout);
}

void padAndOutputLastByte(unsigned int bitAccumulator, int bitCount) 
{
    if (bitCount > 0) 
    {
        unsigned int lastByte = bitAccumulator << (8 - bitCount);
        outputByte(lastByte);
    }
}

void outputShortBytes(short num) 
{
    fputc((num >> 8) & 0xFF, stdout);
    fputc(num & 0xFF, stdout);
}

void putCharactersInHuffmanTreeIntoStdoutInPostOrder(NODE* node) 
{
    if (node == NULL) 
    {
        return; 
    }
    putCharactersInHuffmanTreeIntoStdoutInPostOrder(node->left);
    putCharactersInHuffmanTreeIntoStdoutInPostOrder(node->right);
    if (!node->left && !node->right) 
    {
        if (node->symbol == -1) 
        {
            fputc(0xFF, stdout);
            fputc(0x00, stdout);
        } 
        else 
        {
            fputc(node->symbol, stdout);
        }
    }
}

void postOrderSetWeightsIfLeafOrInner(NODE* node)
{
    if (node == NULL)
    {
        return;
    }
    postOrderSetWeightsIfLeafOrInner(node->left);
    postOrderSetWeightsIfLeafOrInner(node->right);
    if (node != nodes)
    {
        if (!node->left && !node->right)
        {
            node->weight = 0;
        }
        else
        {
            node->weight = 1;
        }
    }
}

NODE* findNodeinNodeForSymbolsOnChar(short symbol)
{
    for (int i = 0; i < MAX_SYMBOLS; i++)
    {
        if (*(node_for_symbol + i) == NULL)
        {
            continue;
        }
        if ((*(node_for_symbol + i))->symbol == symbol)
        {
            return *(node_for_symbol + i);
        }
    }
    return NULL;
}

int depthOfNodeInNodes(NODE* node)
{
    int depth = 0;
    while (node->parent != NULL)
    {
        depth = depth + 1;
        node = node->parent;
    }
    return depth;
}

void encodingOfAParticularNode(NODE* node, int* encoding)
{
    *encoding = 0x0;
    int shift = 0;
    while (node->parent != NULL)
    {
        if (node == node->parent->right)
        {
            *encoding |= (1 << shift);
        }
        shift++;
        node = node->parent;
    }
}

void currentBlockToSTDOUT(int check)
{
    // printNodes();
    int encoding = 0x0;
    short symbol = 0x0;

    unsigned char currentByte = 0x0;
    int currentBitsInByte = 0x0;
    int bitLengthOfCurrentEncoding = 0x0;

    unsigned char* charArray = current_block;

    unsigned char tempEncoding = 0x0;
    // int CURRENT_BLOCK_SIZE = (((unsigned int) global_options) >> 16);

    while (charArray <= current_block + check)
    {
        if (charArray == current_block + check)
        {
            NODE* nodeForCurrentSymbol = findNodeinNodeForSymbolsOnChar(-1);
            bitLengthOfCurrentEncoding = depthOfNodeInNodes(nodeForCurrentSymbol);
            encodingOfAParticularNode(nodeForCurrentSymbol, &encoding);
            // bitLengthOfCurrentEncoding = 16;
            while (bitLengthOfCurrentEncoding > 0)
            {
                if (currentBitsInByte == 8)
                {
                    fputc((int) currentByte, stdout);
                    currentByte = 0x0;
                    currentBitsInByte = 0x0;
                }
                else
                {
                    tempEncoding = encoding >> (bitLengthOfCurrentEncoding - 1);
                    tempEncoding = tempEncoding & 0x1;
                    tempEncoding = tempEncoding << (8 - currentBitsInByte - 1);
                    currentByte = currentByte | tempEncoding;
                    bitLengthOfCurrentEncoding--;
                    currentBitsInByte++;    
                }
                
            }
            fputc(currentByte, stdout);
            break;
        }
        // symbol = charArray == current_block + MAX_BLOCK_SIZE ? -1 : *charArray;
        symbol = *charArray;
        NODE* nodeForCurrentSymbol = findNodeinNodeForSymbolsOnChar(symbol);
        if (nodeForCurrentSymbol == NULL)
        {
            charArray++;
            continue;
        }
        
        // if (nodeForCurrentSymbol == NULL)
        bitLengthOfCurrentEncoding = depthOfNodeInNodes(nodeForCurrentSymbol);
        encodingOfAParticularNode(nodeForCurrentSymbol, &encoding);

        while (bitLengthOfCurrentEncoding > 0)
        {
            if (currentBitsInByte == 8)
            {
                fputc((int) currentByte, stdout);
                currentByte = 0x0;
                currentBitsInByte = 0x0;
            }
            else
            {
                tempEncoding = encoding >> (bitLengthOfCurrentEncoding - 1);
                tempEncoding = tempEncoding & 0x1;
                tempEncoding = tempEncoding << (8 - currentBitsInByte - 1);
                currentByte = currentByte | tempEncoding;
                bitLengthOfCurrentEncoding--;
                currentBitsInByte++;    
            }
            
        }
        charArray++;
    }
    
}