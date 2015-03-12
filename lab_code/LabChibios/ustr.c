#include "ustr.h"

/**
 * @brief String length
 */
int uStrLen(const char *str)
{
   int len = 0;
   while(str[len] != '\0')
       len++;
   return len;
}

/**
 * @brief String copy
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
 * @brief Append character to string
 */
void uStrAppendChar(char *dest, char appChar)
{
    int len = uStrLen(dest);
    dest[len] = appChar;
    dest[len+1] = '\0';
}

/**
 * @brief Prepend character to string
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
 * @brief Insert character to string
 */
void uStrInsertChar(char *dest, char iChar, int insertLocation)
{
    int i = 0;
    int len = uStrLen(dest);
    for(i = len+1; i > insertLocation; i--)
    {
	dest[i] = dest[i-1];
    }
    dest[insertLocation] = iChar;
}

/**
 * @brief Concatenate strings
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

/**
 * @brief Find first instance of token character
 */
int uStrFindChar(const char *str, char find)
{
    int i = 0;
    while(str[i] != '\0')
    {
	if(str[i] == find)
	    return i;
	i++;
    }
    return -1;
}

/**
 * @brief Finds specified instance of token character
 */
int uStrFindCharN(const char *str, char find, int instance)
{
    int i = 0;
    int icnt = 0;
    while(str[i] != '\0')
    {
	if(str[i] == find)
	    icnt++;
	if(icnt == instance)
	    return i;
	i++;
    }
    return -1;
}
