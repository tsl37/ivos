#ifndef STRING_H
#define STRING_H

#define NULL ((void *)0)

// Standardní typ pro velikosti (v 32-bitovém OS obvykle unsigned int)
typedef unsigned int size_t;

// Makra z <ctype.h>, která používá fat.c
#define toupper(c)  (((c) >= 'a' && (c) <= 'z') ? ((c) - 32) : (c))
#define tolower(c)  (((c) >= 'A' && (c) <= 'Z') ? ((c) + 32) : (c))

// Funkce pro práci s pamětí
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

// Funkce pro práci s řetězci
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strcasecmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strtok(char *str, const char *delim);

#endif