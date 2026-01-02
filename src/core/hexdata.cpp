#include "hexdata.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <stdlib.h>
    #include <string.h>
#endif


static void* sys_alloc(size_t size) {
#ifdef _WIN32
    return HeapAlloc(GetProcessHeap(), 0, size);
#else
    return malloc(size);
#endif
}

static void* sys_realloc(void* ptr, size_t size) {
#ifdef _WIN32
    if (!ptr) return HeapAlloc(GetProcessHeap(), 0, size);
    if (size == 0) {
        HeapFree(GetProcessHeap(), 0, ptr);
        return NULL;
    }
    return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
#else
    return realloc(ptr, size);
#endif
}

static void sys_free(void* ptr) {
#ifdef _WIN32
    if (ptr) HeapFree(GetProcessHeap(), 0, ptr);
#else
    if (ptr) free(ptr);
#endif
}


static void mem_copy(void* dst, const void* src, size_t n) {
    unsigned char*       d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) {
        *d++ = *s++;
    }
}

static void mem_set(void* dst, int value, size_t n) {
    unsigned char* d = (unsigned char*)dst;
    unsigned char v = (unsigned char)value;
    while (n--) {
        *d++ = v;
    }
}


static void ss_init(SimpleString* s) {
    s->data     = NULL;
    s->length   = 0;
    s->capacity = 0;
}

static void ss_free(SimpleString* s) {
    if (s->data) {
        sys_free(s->data);
        s->data = NULL;
    }
    s->length   = 0;
    s->capacity = 0;
}

static void ss_clear(SimpleString* s) {
    s->length = 0;
    if (s->data) s->data[0] = '\0';
}

static bool ss_reserve(SimpleString* s, size_t newCap) {
    if (newCap <= s->capacity) return true;
    size_t allocSize = newCap + 1;
    char* newData = (char*)sys_realloc(s->data, allocSize);
    if (!newData) return false;
    s->data     = newData;
    s->capacity = newCap;
    if (s->length == 0) s->data[0] = '\0';
    return true;
}

static bool ss_append_char(SimpleString* s, char c) {
    if (s->length + 1 >= s->capacity) {
        size_t newCap = s->capacity ? s->capacity * 2 : 32;
        if (!ss_reserve(s, newCap)) return false;
    }
    s->data[s->length++] = c;
    s->data[s->length]   = '\0';
    return true;
}

static bool ss_append_cstr(SimpleString* s, const char* str) {
    if (!str) return true;
    size_t len = 0;
    while (str[len] != '\0') ++len;

    if (s->length + len >= s->capacity) {
        size_t newCap = s->capacity ? s->capacity : 32;
        while (s->length + len >= newCap) newCap *= 2;
        if (!ss_reserve(s, newCap)) return false;
    }

    mem_copy(s->data + s->length, str, len);
    s->length += len;
    s->data[s->length] = '\0';
    return true;
}

static bool ss_append_dec2(SimpleString* s, unsigned int value) {
    char buf[3];
    buf[0] = (char)('0' + ((value / 10) % 10));
    buf[1] = (char)('0' + (value % 10));
    buf[2] = '\0';
    return ss_append_cstr(s, buf);
}

static bool ss_append_hex8(SimpleString* s, unsigned int value) {
    char buf[9];
    for (int i = 7; i >= 0; --i) {
        unsigned int nibble = value & 0xF;
        buf[i] = (char)(nibble < 10 ? ('0' + nibble) : ('A' + (nibble - 10)));
        value >>= 4;
    }
    buf[8] = '\0';
    return ss_append_cstr(s, buf);
}

static bool ss_append_hex2(SimpleString* s, unsigned int value) {
    char buf[3];
    for (int i = 1; i >= 0; --i) {
        unsigned int nibble = value & 0xF;
        buf[i] = (char)(nibble < 10 ? ('0' + nibble) : ('A' + (nibble - 10)));
        value >>= 4;
    }
    buf[2] = '\0';
    return ss_append_cstr(s, buf);
}


static void bb_init(ByteBuffer* b) {
    b->data     = NULL;
    b->size     = 0;
    b->capacity = 0;
}

static void bb_free(ByteBuffer* b) {
    if (b->data) {
        sys_free(b->data);
        b->data = NULL;
    }
    b->size     = 0;
    b->capacity = 0;
}

static bool bb_resize(ByteBuffer* b, size_t newSize) {
    if (newSize > b->capacity) {
        size_t newCap = b->capacity ? b->capacity : 256;
        while (newCap < newSize) newCap *= 2;
        uint8_t* newData = (uint8_t*)sys_realloc(b->data, newCap);
        if (!newData) return false;
        b->data     = newData;
        b->capacity = newCap;
    }
    b->size = newSize;
    return true;
}

static bool bb_empty(const ByteBuffer* b) {
    return b->size == 0;
}


static void la_init(LineArray* a) {
    a->lines    = NULL;
    a->count    = 0;
    a->capacity = 0;
}

static void la_clear(LineArray* a) {
    if (a->lines) {
        for (size_t i = 0; i < a->count; ++i) {
            ss_free(&a->lines[i]);
        }
    }
    a->count = 0;
}

static void la_free(LineArray* a) {
    la_clear(a);
    if (a->lines) {
        sys_free(a->lines);
        a->lines = NULL;
    }
    a->capacity = 0;
}

static bool la_reserve(LineArray* a, size_t newCap) {
    if (newCap <= a->capacity) return true;
    SimpleString* newLines =
        (SimpleString*)sys_realloc(a->lines, newCap * sizeof(SimpleString));
    if (!newLines) return false;

    for (size_t i = a->capacity; i < newCap; ++i) {
        ss_init(&newLines[i]);
    }

    a->lines    = newLines;
    a->capacity = newCap;
    return true;
}

static bool la_push_back(LineArray* a, const SimpleString* s) {
    if (a->count >= a->capacity) {
        size_t newCap = a->capacity ? a->capacity * 2 : 16;
        if (!la_reserve(a, newCap)) return false;
    }

    SimpleString* dst = &a->lines[a->count];
    ss_clear(dst);
    if (s->length > 0) {
        if (!ss_reserve(dst, s->length)) return false;
        ss_append_cstr(dst, s->data);
    }
    ++a->count;
    return true;
}

static bool la_push_back_cstr(LineArray* a, const char* str) {
    SimpleString tmp;
    ss_init(&tmp);
    ss_append_cstr(&tmp, str);
    bool ok = la_push_back(a, &tmp);
    ss_free(&tmp);
    return ok;
}


static bool read_file_all(const char* path, ByteBuffer* outBuffer) {
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER liSize;
    if (!GetFileSizeEx(hFile, &liSize)) {
        CloseHandle(hFile);
        return false;
    }

    if (liSize.QuadPart < 0 || liSize.QuadPart > 0x7FFFFFFFLL) {
        CloseHandle(hFile);
        return false;
    }

    size_t size = (size_t)liSize.QuadPart;
    if (!bb_resize(outBuffer, size)) {
        CloseHandle(hFile);
        return false;
    }

    DWORD readBytes = 0;
    if (!ReadFile(hFile, outBuffer->data, (DWORD)size, &readBytes, NULL) ||
        readBytes != size) {
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    return true;
#else
    int fd = open(path, O_RDONLY);
    if (fd < 0) return false;

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return false;
    }

    if (st.st_size < 0) {
        close(fd);
        return false;
    }

    size_t size = (size_t)st.st_size;
    if (!bb_resize(outBuffer, size)) {
        close(fd);
        return false;
    }

    size_t totalRead = 0;
    while (totalRead < size) {
        ssize_t r = read(fd, outBuffer->data + totalRead, size - totalRead);
        if (r <= 0) {
            close(fd);
            return false;
        }
        totalRead += (size_t)r;
    }

    close(fd);
    return true;
#endif
}

static bool write_file_all(const char* path, const uint8_t* data, size_t size) {
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    if (!WriteFile(hFile, data, (DWORD)size, &written, NULL) ||
        written != size) {
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    return true;
#else
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return false;

    size_t totalWritten = 0;
    while (totalWritten < size) {
        ssize_t w = write(fd, data + totalWritten, size - totalWritten);
        if (w <= 0) {
            close(fd);
            return false;
        }
        totalWritten += (size_t)w;
    }

    close(fd);
    return true;
#endif
}


static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}


HexData::HexData()
    : currentBytesPerLine(16),
      modified(false),
      capstoneInitialized(false),
      currentArch(0),
      currentMode(0),
      csHandle(0) {
    bb_init(&fileData);
    la_init(&hexLines);
    la_init(&disassemblyLines);
    ss_init(&headerLine);

}

HexData::~HexData() {
    clear();
    bb_free(&fileData);
    la_free(&hexLines);
    la_free(&disassemblyLines);
    ss_free(&headerLine);
}

bool HexData::initializeCapstone() {
    /*
    if (capstoneInitialized) {
        cleanupCapstone();
    }

    if (cs_open(currentArch, currentMode, &csHandle) != CS_ERR_OK) {
        capstoneInitialized = false;
        return false;
    }

    cs_option(csHandle, CS_OPT_DETAIL, CS_OPT_ON);
    cs_option(csHandle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL);

    capstoneInitialized = true;
    return true;
    */
    return false;
}

void HexData::cleanupCapstone() {
    /*
    if (capstoneInitialized) {
        cs_close(&csHandle);
        capstoneInitialized = false;
    }
    */
}

void HexData::setArchitecture(int /*arch*/, int /*mode*/) {
    /*
    currentArch = arch;
    currentMode = mode;
    initializeCapstone();

    if (!bb_empty(&fileData)) {
        regenerateHexLines(currentBytesPerLine);
    }
    */
}

bool HexData::loadFile(const char* filepath) {
    if (!read_file_all(filepath, &fileData)) {
        la_clear(&hexLines);
        la_push_back_cstr(&hexLines, "Error: Failed to open or read file");
        return false;
    }

    convertDataToHex(16);
    modified = false;
    return true;
}

bool HexData::saveFile(const char* filepath) {
    if (!write_file_all(filepath, fileData.data, fileData.size)) {
        return false;
    }
    modified = false;
    return true;
}

bool HexData::editByte(size_t offset, uint8_t newValue) {
    if (offset >= fileData.size) return false;
    fileData.data[offset] = newValue;
    modified = true;
    regenerateHexLines(currentBytesPerLine);
    return true;
}

uint8_t HexData::getByte(size_t offset) const {
    if (offset >= fileData.size) return 0;
    return fileData.data[offset];
}

void HexData::clear() {
    bb_resize(&fileData, 0);
    la_clear(&hexLines);
    la_clear(&disassemblyLines);
    ss_clear(&headerLine);
    modified = false;
}

void HexData::regenerateHexLines(int bytesPerLine) {
    if (!bb_empty(&fileData)) {
        convertDataToHex(bytesPerLine);
    }
}

void HexData::generateHeader(int bytesPerLine) {
    ss_clear(&headerLine);
    ss_append_cstr(&headerLine, "Offset    ");
    for (int i = 0; i < bytesPerLine; ++i) {
        ss_append_dec2(&headerLine, (unsigned int)i);
        ss_append_char(&headerLine, ' ');
    }
    ss_append_char(&headerLine, ' ');
    ss_append_cstr(&headerLine, "Decoded text");
}

void HexData::disassembleInstruction(size_t offset,
                                     int& instructionLength,
                                     SimpleString& outInstr) {
    ss_clear(&outInstr);
    instructionLength = 1;
    
    /*
    ss_clear(&outInstr);
    if (!capstoneInitialized || offset >= fileData.size) {
        instructionLength = 1;
        return;
    }

    size_t remainingBytes = fileData.size - offset;
    size_t bytesToDisasm  = (remainingBytes < 15) ? remainingBytes : 15;

    cs_insn* insn = NULL;
    size_t count  = cs_disasm(csHandle,
                              fileData.data + offset,
                              bytesToDisasm,
                              (uint64_t)offset,
                              1,
                              &insn);

    if (count > 0 && insn) {
        instructionLength = insn[0].size;
        ss_append_cstr(&outInstr, insn[0].mnemonic);
        if (insn[0].op_str[0] != '\0') {
            ss_append_char(&outInstr, ' ');
            ss_append_cstr(&outInstr, insn[0].op_str);
        }
        cs_free(insn, count);
    } else {
        instructionLength = 1;
    }
    */
}

void HexData::generateDisassembly(int bytesPerLine) {
    (void)bytesPerLine;
    la_clear(&disassemblyLines);
    
    /*
    (void)bytesPerLine; 
    la_clear(&disassemblyLines);
    if (bb_empty(&fileData)) return;

    for (size_t i = 0; i < fileData.size; i += (size_t)bytesPerLine) {
        SimpleString line;
        ss_init(&line);

        size_t lineOffset = i;
        while (lineOffset < i + (size_t)bytesPerLine && lineOffset < fileData.size) {
            int instrLen = 0;
            SimpleString instr;
            ss_init(&instr);

            disassembleInstruction(lineOffset, instrLen, instr);

            if (instr.length > 0) {
                if (line.length > 0) {
                    ss_append_cstr(&line, "; ");
                }
                ss_append_cstr(&line, instr.data);
            }

            ss_free(&instr);
            lineOffset += (size_t)instrLen;
            break;
        }

        la_push_back(&disassemblyLines, &line);
        ss_free(&line);
    }
    */
}

void HexData::convertDataToHex(int bytesPerLine) {
    la_clear(&hexLines);

    if (bb_empty(&fileData)) {
        la_push_back_cstr(&hexLines, "No data to display");
        ss_clear(&headerLine);
        return;
    }

    bytesPerLine = clamp_int(bytesPerLine, 8, 48);
    currentBytesPerLine = bytesPerLine;

    generateHeader(bytesPerLine);
    generateDisassembly(bytesPerLine);

    for (size_t i = 0; i < fileData.size; i += (size_t)bytesPerLine) {
        SimpleString line;
        ss_init(&line);

        ss_append_hex8(&line, (unsigned int)i);
        ss_append_cstr(&line, "  ");

        for (int j = 0; j < bytesPerLine; ++j) {
            size_t idx = i + (size_t)j;
            if (idx < fileData.size) {
                unsigned int v = fileData.data[idx];
                ss_append_hex2(&line, v);
                ss_append_char(&line, ' ');
            } else {
                ss_append_cstr(&line, "   ");
            }
        }

        ss_append_char(&line, ' ');

        for (int j = 0; j < bytesPerLine; ++j) {
            size_t idx = i + (size_t)j;
            if (idx >= fileData.size) break;
            uint8_t b = fileData.data[idx];
            if (b >= 32 && b <= 126) {
                ss_append_char(&line, (char)b);
            } else {
                ss_append_char(&line, '.');
            }
        }

        ss_append_cstr(&line, "      ");

        la_push_back(&hexLines, &line);
        ss_free(&line);
    }
}
