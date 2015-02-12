#ifndef _USTR_H_
#define _USTR_H_

#include <stdint.h>

int uStrLen(char *str);
void uStrCpy(char *dest, char *src);
void uStrAppendChar(char *dest, char appChar);


#endif