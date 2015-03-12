/**
 * Small footprint string library for embedded platforms
 * No dynamic allocation
 * Author: Cody Hyman
 */

#ifndef _USTR_H_
#define _USTR_H_

#include <stdint.h>

int uStrLen(const char *str);
void uStrCpy(char *dest, const char *src);
void uStrAppendChar(char *dest, char appChar);
void uStrPrependChar(char *dest, char preChar);
void uStrInsertChar(char *dest, char iChar, int insertLocation);
void uStrCat(char *str1, const char *str2);
int uStrFindChar(const char *str, char find);
int uStrFindCharN(const char *str, char find, int instance);

#endif