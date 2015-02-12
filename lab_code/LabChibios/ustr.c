#include "ustr.h"

/**
 * 
 */
int uStrLen(char *str)
{
   int len = 0;
   while(str[len] != '\0')
       len++;
   return len;
}

/**
 * 
 */
void uStrCpy(char *dest, char *src)
{
    int len = uStrLen(src);
    uint16_t i;
    for(i = 0; i <= len; i++)
    {
	dest[i] = src[i];
    } 
}

/**
 * 
 */
void uStrAppendChar(char *dest, char appChar)
{
    int len = uStrLen(dest);
    dest[len] = appChar;
    dest[len+1] = '\0';
}