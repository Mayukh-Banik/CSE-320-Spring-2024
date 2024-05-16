#include "helper.h"

#include <stdio.h>

int userPassedValidBlockSize(char* str)
{
    if (*str == '\0') 
    {
        return 0;
    }
    int num = 0;
    while (*str) 
    {
        if (*str < '0' || *str > '9') 
		{
			return -1;
		}
        int digit = *str - '0';
        num = num * 10 + digit;
        str++;
    }
    return num >= 1024 && num <= 65536;
}

int userBlockSize(char* str)
{

    if (*str == '\0') 
    {
        return 0;
    }
    int num = 0;
    while (*str) 
    {
        if (*str < '0' || *str > '9') 
		{
			return 0;
		}
        int digit = *str - '0';
        num = num * 10 + digit;
        str++;
    }
    return num - 1;
}

int isValidOption(char Option)
{
	if (Option != 'h' && Option != 'c' && Option != 'd' && Option != 'b')
	{
		return -1;
	}
	if (Option != 'b')
	{
		if (global_options != 0x0)
		{
			return -1;
		}
	}
	else
	{
		if (global_options != 0x2)
		{
			return -1;
		}
	}
	return 0;
}