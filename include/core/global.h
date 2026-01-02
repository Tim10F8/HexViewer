#ifndef GLOBAL_H
#define GLOBAL_H

#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>

inline void* PlatformAlloc(size_t size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

inline void PlatformFree(void* ptr, size_t = 0)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}


#elif defined(__APPLE__)
#include <sys/mman.h>

inline void* PlatformAlloc(size_t size)
{
    return mmap(NULL, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

inline void PlatformFree(void* ptr, size_t size)
{
    munmap(ptr, size);
}

#else
#include <sys/mman.h>

inline void* PlatformAlloc(size_t size)
{
    return mmap(NULL, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

inline void PlatformFree(void* ptr, size_t size)
{
    munmap(ptr, size);
}

#endif

extern int g_SearchCaretX;
extern int g_SearchCaretY;
extern bool caretVisible;

inline void* MemSet(void* dest, int val, size_t count)
{
    unsigned char* p = (unsigned char*)dest;
    while (count--) {
        *p++ = (unsigned char)val;
    }
    return dest;
}

inline char* StrCopy(char* dest, const char* src)
{
    char* original = dest;
    while ((*dest++ = *src++)) {
    }
    return original;
}

inline int StringLength(const char *str)
{
    int len = 0;
    while (str[len])
        len++;
    return len;
}

inline void StringCopy(char *dest, const char *src, int maxLen)
{
    int i = 0;
    while (src[i] && i < maxLen - 1)
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

inline void StringAppend(char *dest, char ch, int maxLen)
{
    int len = StringLength(dest);
    if (len < maxLen - 1)
    {
        dest[len] = ch;
        dest[len + 1] = 0;
    }
}

inline void StringRemoveLast(char *str)
{
    int len = StringLength(str);
    if (len > 0)
    {
        str[len - 1] = 0;
    }
}

inline bool StringIsEmpty(const char *str)
{
    return str[0] == 0;
}

inline int StringToInt(const char *str)
{
    int result = 0;
    int i = 0;
    bool isHex = false;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        isHex = true;
        i = 2;
    }

    while (str[i])
    {
        if (isHex)
        {
            result *= 16;
            if (str[i] >= '0' && str[i] <= '9')
                result += str[i] - '0';
            else if (str[i] >= 'a' && str[i] <= 'f')
                result += str[i] - 'a' + 10;
            else if (str[i] >= 'A' && str[i] <= 'F')
                result += str[i] - 'A' + 10;
        }
        else
        {
            if (str[i] >= '0' && str[i] <= '9')
            {
                result = result * 10 + (str[i] - '0');
            }
        }
        i++;
    }
    return result;
}

inline void IntToString(int value, char* out, int maxLen)
{
    if (!out || maxLen <= 1)
        return;

    unsigned int v;
    int pos = 0;

    if (value < 0)
    {
        if (maxLen < 2) { out[0] = '\0'; return; }
        out[pos++] = '-';
        v = (unsigned int)(-value);
    }
    else
    {
        v = (unsigned int)value;
    }

    char temp[32];
    int tpos = 0;

    do {
        temp[tpos++] = '0' + (v % 10);
        v /= 10;
    } while (v && tpos < 31);

    for (int i = tpos - 1; i >= 0 && pos < maxLen - 1; i--)
        out[pos++] = temp[i];

    out[pos] = '\0';
}

inline size_t StrLen(const char* str)
{
    const char* s = str;
    while (*s) s++;
    return (size_t)(s - str);
}

inline char* AllocString(const char* str)
{
    if (!str)
        return nullptr;

    size_t len = StrLen(str);
    char* result = (char*)PlatformAlloc(len + 1);
    if (!result)
        return nullptr;

    StrCopy(result, str);
    return result;
}

template<typename T>
inline T Clamp(T value, T min, T max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

template<typename T>
inline T Min(T a, T b)
{
    return (a < b) ? a : b;
}

template<typename T>
inline T Max(T a, T b)
{
    return (a > b) ? a : b;
}

inline double Floor(double x)
{
    long long i = (long long)x;
    return (double)(i - (x < i));
}

inline bool StrEquals(const char* a, const char* b)
{
    if (a == b) return true;
    if (!a || !b) return false;

    while (*a && *b)
    {
        if (*a != *b)
            return false;
        a++;
        b++;
    }
    return *a == *b;
}

inline void* MemAlloc(size_t size) {
    return PlatformAlloc(size);
}

inline void MemFree(void* ptr, size_t size = 0) {
    PlatformFree(ptr, size);
}

inline int StrToInt(const char* s)
{
    if (!s) return 0;

    while (*s == ' ' || *s == '\t')
        s++;

    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }

    int value = 0;
    while (*s >= '0' && *s <= '9')
    {
        value = value * 10 + (*s - '0');
        s++;
    }

    return value * sign;
}

inline void IntToStr(int value, char* buffer, size_t bufferSize)
{
    if (!buffer || bufferSize == 0)
        return;

    if (value == 0)
    {
        if (bufferSize >= 2)
        {
            buffer[0] = '0';
            buffer[1] = '\0';
        }
        return;
    }

    bool negative = false;
    unsigned int v;

    if (value < 0)
    {
        negative = true;
        v = (unsigned int)(-value);
    }
    else
    {
        v = (unsigned int)value;
    }

    char temp[32];
    size_t idx = 0;

    while (v > 0 && idx < sizeof(temp))
    {
        temp[idx++] = (char)('0' + (v % 10));
        v /= 10;
    }

    if (negative && idx < sizeof(temp))
        temp[idx++] = '-';

    size_t out = 0;
    if (idx >= bufferSize)
        idx = bufferSize - 1;

    while (idx > 0)
        buffer[out++] = temp[--idx];

    buffer[out] = '\0';
}

inline size_t WcsLen(const wchar_t* str)
{
    const wchar_t* s = str;
    while (*s) s++;
    return (size_t)(s - str);
}

inline wchar_t* WcsCopy(wchar_t* dest, const wchar_t* src)
{
    wchar_t* original = dest;
    while ((*dest++ = *src++)) {
    }
    return original;
}

inline void MemCopy(void* dest, const void* src, size_t n)
{
    if (!dest || !src || n == 0)
        return;

    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    if (d > s && d < s + n) {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    } else {
        while (n--) *d++ = *s++;
    }
}

inline void ByteToHex(uint8_t b, char out[2])
{
    const char* hex = "0123456789ABCDEF";
    out[0] = hex[(b >> 4) & 0xF];
    out[1] = hex[b & 0xF];
}

inline bool IsXDigit(char c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}

inline int HexDigitToInt(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return 0;
}

inline long long ClampLL(long long v, long long lo, long long hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

inline int ClampInt(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

inline char IntToHexChar(int digit)
{
    if (digit < 10) return (char)('0' + digit);
    return (char)('A' + (digit - 10));
}

inline wchar_t* AllocWideString(const wchar_t* str)
{
    if (!str)
        return nullptr;

    size_t len = WcsLen(str);
    wchar_t* result = (wchar_t*)PlatformAlloc((len + 1) * sizeof(wchar_t));
    if (!result)
        return nullptr;

    WcsCopy(result, str);
    return result;
}

inline void StrCat(char* dest, const char* src)
{
    if (!dest || !src)
        return;

    while (*dest)
        dest++;

    while (*src)
        *dest++ = *src++;

    *dest = '\0';
}

template<typename T>
class Vector {
private:
    T* data;
    size_t count;
    size_t capacity;

public:
    Vector() : data(nullptr), count(0), capacity(0) {}

    ~Vector()
    {
        if (data)
            PlatformFree(data, capacity * sizeof(T));
    }

    void push_back(const T& item)
    {
        if (count >= capacity)
        {
            size_t newCapacity = (capacity == 0) ? 4 : capacity * 2;
            T* newData = (T*)PlatformAlloc(newCapacity * sizeof(T));

            for (size_t i = 0; i < count; i++)
                newData[i] = data[i];

            if (data)
                PlatformFree(data, capacity * sizeof(T));

            data = newData;
            capacity = newCapacity;
        }

        data[count++] = item;
    }

    void Add(const T& item) { push_back(item); }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    size_t size() const { return count; }
    size_t Count() const { return count; }

    bool empty() const { return count == 0; }

    void clear() { count = 0; }
};

inline char* StrDup(const char* s) {
    if (!s) return nullptr;
    size_t len = StrLen(s);
    char* result = (char*)PlatformAlloc(len + 1);
    StrCopy(result, s);
    return result;
}

#endif
