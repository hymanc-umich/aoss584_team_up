#include "ustr.h"

/**
 * 
 */
int uStrLen(const char *str)
{
   int len = 0;
   while(str[len] != '\0')
       len++;
   return len;
}

/**
 * 
 */
void uStrCpy(char *dest, const char *src)
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

/**
 * 
 */
void uStrPrependChar(char *dest, char preChar)
{
    int i;
    int len = uStrLen(dest);
    for(i = len+1; i > 0; i--)
    {
	dest[i] = dest[i-1]; 
    }
    dest[0] = preChar;
}

/**
 * 
 */
void uStrCat(char *str1, const char *str2)
{
    int len1 = uStrLen(str1);
    int len2 = uStrLen(str2);
    int i;
    for(i = 0; i <= len2; i++)
    {
	str1[i+len1] = str2[i];
    }
}