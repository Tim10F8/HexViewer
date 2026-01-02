#pragma once
#include "render.h"
#include "menu.h"
#include "options.h"

struct PatternSearchState
{
    char searchPattern[256];
    int lastMatch;
    bool hasFocus;
};

struct ChecksumState
{
    bool md5;
    bool sha1;
    bool sha256;
    bool crc32;

    bool entireFile;
};

struct CompareState
{
    char filePath[512];
    bool fileLoaded;
};




extern PatternSearchState g_PatternSearch;
extern ChecksumState      g_Checksum;
extern CompareState       g_Compare;



void PatternSearch_SetFocus();
void PatternSearch_Run();
void PatternSearch_FindNext();
void PatternSearch_FindPrev();
static int ParseHexPattern(const char* text, uint8_t* out, int maxOut);


void Checksum_ToggleMD5();
void Checksum_ToggleSHA1();
void Checksum_ToggleSHA256();
void Checksum_ToggleCRC32();

void Checksum_SetModeEntireFile();
void Checksum_SetModeSelection();

void Checksum_Compare();
void Checksum_Compute();



void Compare_OpenFileDialog();
void Compare_Run();



bool HandleBottomPanelContentClick(int x, int y, int windowWidth, int windowHeight);
