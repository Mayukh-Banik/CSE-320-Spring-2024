#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "huff.h"
#include "debug.h"


#include "helper.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Emits a description of the Huffman tree used to compress the current block.
 * @details This function emits, to the standard output, a description of the
 * Huffman tree used to compress the current block.  Refer to the assignment handout
 * for a detailed specification of the format of this description.
 */
void emit_huffman_tree() 
{
    unsigned int bitAccumulator = 0;
    int bitCount = 0;
    outputShortBytes((short) num_nodes);
    huffmanTreeToBitOrderInPostOrderForm(nodes, &bitAccumulator, &bitCount);
    padAndOutputLastByte(bitAccumulator, bitCount);
    putCharactersInHuffmanTreeIntoStdoutInPostOrder(nodes);
    fflush(stdout);
    return;
}

/**
 * @brief Reads a description of a Huffman tree and reconstructs the tree from
 * the description.
 * @details  This function reads, from the standard input, the description of a
 * Huffman tree in the format produced by emit_huffman_tree(), and it reconstructs
 * the tree from the description.  Refer to the assignment handout for a specification
 * of the format for this description, and a discussion of how the tree can be
 * reconstructed from it.
 *
 * @return 0 if the tree is read and reconstructed without error, 1 if EOF is
 * encountered at the start of a block, otherwise -1 if an error occurs.
 */
int read_huffman_tree() 
{
    {
        int c = fgetc(stdin);
        if (c == EOF)
        {
            return 1;
        }
        ungetc(c, stdin);
    }
    num_nodes = dReadNumNodesFromStartOfHuffmanEncoding();
    // fprintf(stderr, "%d", num_nodes);
    // fflush(stderr);
    if (num_nodes <= 0)
    {
        return -1;
    }
    if (buildTreeFromDescription(num_nodes) == -1)
    {
        return -1;
    }
    dNodeLeafsToSymbols(nodes);
    if (dReadCharactersToSymbols() == -1)
    {
        return -1;
    }
    // printSymbols();
    return 0;
}

/**
 * @brief Reads one block of data from standard input and emits corresponding
 * compressed data to standard output.
 * @details This function reads raw binary data bytes from the standard input
 * until the specified block size has been read or until EOF is reached.
 * It then applies a data compression algorithm to the block and outputs the
 * compressed block to the standard output.  The block size parameter is
 * obtained from the global_options variable.
 *
 * @return 0 if compression completes without error, -1 if an error occurs.
 */
int compress_block() 
{

    zeroCurrentBlock();
    wipeNodes();
    wipeAllNodeForSymbol();
    zeroSymbols();
    // fprintf(stderr, "hey");
    // fflush(stderr);
    int CURRENT_BLOCK_SIZE = (((unsigned int) global_options) >> 16) + 1;
	int check = -1;
	check = readIntoCurrentBlock(CURRENT_BLOCK_SIZE);
    // printCurrentBlock(check);
	if (check == -1)
	{
		return -1;
	}
    // printCurrentBlock(check);
    // printNodes();
    currBlockToNodes(check);
    // printNodes();
    // printCurrentBlock(check);
    // printNodes();
    num_nodes = 2 * num_nodes - 1;
    num_nodes = returnNumOfNodes();
    initBuildHuffmanTree();
    // pPrintTree();
    emit_huffman_tree();
    postOrderSetWeightsIfLeafOrInner(nodes);
    currentBlockToSTDOUT(check);
    fflush(stdout);

    return 0;
}

/**
 * @brief Reads one block of compressed data from standard input and writes
 * the corresponding uncompressed data to standard output.
 * @details This function reads one block of compressed data from the standard
 * input, it decompresses the block, and it outputs the uncompressed data to
 * the standard output.  The input data blocks are assumed to be in the format
 * produced by compress().  If EOF is encountered before a complete block has
 * been read, it is an error.
 *
 * @return 0 if decompression completes without error, -1 if an error occurs.
 */
int decompress_block() 
{
    zeroCurrentBlock();
    wipeNodes();
    wipeAllNodeForSymbol();
    if (read_huffman_tree() != 0)
    {
        return -1;
    }
    // pPrintTree();
    if (dPrintOutValues() != 0)
    {
        return -1;
    }
    fflush(stdout);
    return 0;
}

/**
 * @brief Reads raw data from standard input, writes compressed data to
 * standard output.
 * @details This function reads raw binary data bytes from the standard input in
 * blocks of up to a specified maximum number of bytes or until EOF is reached,
 * it applies a data compression algorithm to each block, and it outputs the
 * compressed blocks to standard output.  The block size parameter is obtained
 * from the global_options variable.
 *
 * @return 0 if compression completes without error, -1 if an error occurs.
 */
int compress() 
{
    int c = 0x0;
    while ((c = fgetc(stdin)) != EOF)
    {
        ungetc(c, stdin);
        if (compress_block() == -1)
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Reads compressed data from standard input, writes uncompressed
 * data to standard output.
 * @details This function reads blocks of compressed data from the standard
 * input until EOF is reached, it decompresses each block, and it outputs
 * the uncompressed data to the standard output.  The input data blocks
 * are assumed to be in the format produced by compress().
 *
 * @return 0 if decompression completes without error, -1 if an error occurs.
 */
int decompress() 
{
    // abort();
    // decompress_block();
    int c = 0x0;
    while ((c = fgetc(stdin)) != EOF)
    {
        ungetc(c, stdin);
        if (decompress_block() == -1)
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
	global_options = 0x0;
    int MINIMUM = 1;
    int MAXIMUM = 4;
    if (argc <= MINIMUM || argc > MAXIMUM)
    {
        return -1;
    }
    int i = 1;    
    while (i < argc)
    {  
        switch (**(argv+i))
        {
            case '-':
                char a = *(*(argv+i) + 1);
                switch (a)
                {
                    case 'h':
						if (isValidOption(a) == -1 || (*(*(argv+i) + 2) != '\0'))
						{
							return -1;
						}
                        global_options |= 1;
                        return 0;
                    break;
                    case 'c':
						if (isValidOption(a) == -1 || (*(*(argv+i) + 2) != '\0'))
						{
							return -1;
						}
						global_options |= 0x2;
                    break;
                    case 'd':
						if (isValidOption(a) == -1 || (*(*(argv+i) + 2) != '\0'))
						{
							return -1;
						}
                        global_options |= 0x4;
						global_options |= (0xFFFF << 16);
                    break;
                    case 'b':
                        if (isValidOption(a) == -1 || (*(*(argv+i) + 2) != '\0'))
						{
							return -1;
						}
						if (userPassedValidBlockSize(*(argv + i + 1)) == -1)
						{
							return -1;
						}
						global_options |= (userBlockSize(*(argv +i + 1)) << 16);
						return 0;
                    break;
                    default:
                        return -1;
                    break;
                }
            break;
            default:
                return -1;
            break;
        }
        i++;
    }
	if (global_options == 0x2)
	{
        // global_options |= 0x00010000;
		global_options |= 0xFFFF0000;
	}
    return 0;
}











