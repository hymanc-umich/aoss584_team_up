#ifndef _USTR_H_
#define _USTR_H_

#include <stdint.h>

int uStrLen(const char *str);
void uStrCpy(char *dest, const char *src);
void uStrAppendChar(char *dest, char appChar);
void uStrPrependChar(char *dest, char preChar);
void uStrCat(char *str1, const char *str2);

#endif