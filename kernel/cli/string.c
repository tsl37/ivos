#include "string.h"

void *memcpy(void *dest, const void *src, size_t n)
{
    char *d = dest;
    const char *s = src;
    while (n--)
    {
        *d++ = *s++;
    }
    return dest;
}

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    while (n--)
    {
        *p++ = (unsigned char)c;
    }
    return s;
}

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p)
    {
        p++;
    }
    return (size_t)(p - s);
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strcasecmp(const char *s1, const char *s2)
{
    while (*s1 && (tolower((unsigned char)*s1) == tolower((unsigned char)*s2)))
    {
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++))
        ;
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *d = dest;
    while (n > 0 && *src != '\0')
    {
        *d++ = *src++;
        n--;
    }
    while (n > 0)
    {
        *d++ = '\0';
        n--;
    }
    return dest;
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c)
    {
        if (!*s)
        {
            return NULL;
        }
        s++;
    }
    return (char *)s;
}

char *strrchr(const char *s, int c)
{
    const char *last = NULL;
    do
    {
        if (*s == (char)c)
        {
            last = s;
        }
    } while (*s++);

    return (char *)last;
}

char *strtok(char *str, const char *delim)
{
    static char *last_ptr = NULL;

    if (str == NULL)
    {
        str = last_ptr;
    }

    if (str == NULL)
    {
        return NULL;
    }

    // Přeskočení počátečních oddělovačů
    while (*str && strchr(delim, *str))
    {
        str++;
    }

    if (*str == '\0')
    {
        last_ptr = NULL;
        return NULL;
    }

    char *start = str;

    // Nalezení konce tokenu
    while (*str && !strchr(delim, *str))
    {
        str++;
    }

    if (*str)
    {
        *str = '\0';
        last_ptr = str + 1;
    }
    else
    {
        last_ptr = NULL;
    }

    return start;
}