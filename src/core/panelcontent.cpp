#include "panelcontent.h"

extern MenuBar g_MenuBar;

#ifdef _WIN32
extern HWND g_Hwnd;
#endif

extern LeftPanelState g_LeftPanel;
extern BottomPanelState g_BottomPanel;
extern ChecksumResults g_Checksums;
const int PANEL_TITLE_HEIGHT = 28;
extern HexData g_HexData;

extern long long cursorBytePos;
extern int cursorNibblePos;
extern int g_ScrollY;
extern int g_LinesPerPage;
void InvalidateWindow();

PatternSearchState g_PatternSearch = {"", -1, false};
ChecksumState g_Checksum = {false, false, false, false, true};
CompareState g_Compare = {"", false};


void PatternSearch_SetFocus()
{
    g_PatternSearch.hasFocus = true;
}

void PatternSearch_Run()
{
    g_PatternSearch.lastMatch = -1;
}

void PatternSearch_FindNext()
{
    uint8_t pattern[128];
    int patLen = ParseHexPattern(g_PatternSearch.searchPattern, pattern, 128);
    if (patLen <= 0)
        return;

    long long fileSize = (long long)g_HexData.getFileSize();
    if (fileSize <= 0)
        return;

    long long start = g_PatternSearch.lastMatch >= 0
                        ? g_PatternSearch.lastMatch + 1
                        : 0;

    for (long long i = start; i <= fileSize - patLen; i++)
    {
        bool match = true;
        for (int j = 0; j < patLen; j++)
        {
            if (g_HexData.getByte((size_t)(i + j)) != pattern[j])
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            g_PatternSearch.lastMatch = i;

            cursorBytePos = i;
            cursorNibblePos = 0;

            long long line = i / 16;
            if (line < g_ScrollY || line >= g_ScrollY + g_LinesPerPage)
            {
                g_ScrollY = (int)line;

#ifdef _WIN32
                SetScrollPos(g_Hwnd, SB_VERT, g_ScrollY, TRUE);
#endif
            }

            InvalidateWindow();
            return;
        }
    }

    g_PatternSearch.lastMatch = -1;
}


void PatternSearch_FindPrev()
{
    uint8_t pattern[128];
    int patLen = ParseHexPattern(g_PatternSearch.searchPattern, pattern, 128);
    if (patLen <= 0)
        return;

    long long fileSize = (long long)g_HexData.getFileSize();
    if (fileSize <= 0)
        return;

    long long start = g_PatternSearch.lastMatch >= 0
                        ? g_PatternSearch.lastMatch - 1
                        : fileSize - patLen;

    for (long long i = start; i >= 0; i--)
    {
        bool match = true;
        for (int j = 0; j < patLen; j++)
        {
            if (g_HexData.getByte((size_t)(i + j)) != pattern[j])
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            g_PatternSearch.lastMatch = i;

            cursorBytePos = i;
            cursorNibblePos = 0;

            long long line = i / 16;
            if (line < g_ScrollY || line >= g_ScrollY + g_LinesPerPage)
            {
                g_ScrollY = (int)line;

#ifdef _WIN32
                SetScrollPos(g_Hwnd, SB_VERT, g_ScrollY, TRUE);
#endif
            }

            InvalidateWindow();
            return;
        }
    }

    g_PatternSearch.lastMatch = -1;
}



void Checksum_ToggleMD5()
{
    g_Checksum.md5 = !g_Checksum.md5;
}

void Checksum_ToggleSHA1()
{
    g_Checksum.sha1 = !g_Checksum.sha1;
}

void Checksum_ToggleSHA256()
{
    g_Checksum.sha256 = !g_Checksum.sha256;
}

void Checksum_ToggleCRC32()
{
    g_Checksum.crc32 = !g_Checksum.crc32;
}

void Checksum_SetModeEntireFile()
{
    g_Checksum.entireFile = true;
}

void Checksum_SetModeSelection()
{
    g_Checksum.entireFile = false;
}

void Checksum_Compare()
{
}

void Checksum_Compute()
{
}


void Compare_OpenFileDialog()
{
    g_Compare.fileLoaded = true;
}

void Compare_Run()
{
    if (!g_Compare.fileLoaded)
        return;
}

static int ParseHexPattern(const char* text, uint8_t* out, int maxOut)
{
    int count = 0;
    int i = 0;

    while (text[i] && count < maxOut)
    {
        while (text[i] == ' ') i++;

        if (!text[i] || !text[i+1])
            break;

        char a = text[i];
        char b = text[i+1];

        auto hex = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };

        int hi = hex(a);
        int lo = hex(b);
        if (hi < 0 || lo < 0)
            break;

        out[count++] = (uint8_t)((hi << 4) | lo);

        i += 2;
    }

    return count;
}


bool HandleBottomPanelContentClick(int x, int y, int windowWidth, int windowHeight)
{
    if (!g_BottomPanel.visible)
        return false;

    Rect bottomBounds = GetBottomPanelBounds(
        g_BottomPanel, windowWidth, windowHeight,
        g_MenuBar.getHeight(), g_LeftPanel);

    int tabHeight = 32;
    bool isVertical = (g_BottomPanel.dockPosition == PanelDockPosition::Left ||
                       g_BottomPanel.dockPosition == PanelDockPosition::Right);

    int contentX = bottomBounds.x + 15;
    int contentY = bottomBounds.y + PANEL_TITLE_HEIGHT +
                   (isVertical ? (tabHeight * 4) : tabHeight) + 10;

    int contentWidth = bottomBounds.width - 30;
    int contentHeight = bottomBounds.height - (contentY - bottomBounds.y) - 10;

    switch (g_BottomPanel.activeTab)
    {
    case BottomPanelState::Tab::EntropyAnalysis:
        return false;

    case BottomPanelState::Tab::PatternSearch:
    {
        int cy = contentY;

        cy += 25;
        cy += 20;

        Rect searchBox(contentX, cy, 200, 28);
        if (IsPointInRect(x, y, searchBox))
        {
            g_PatternSearch.hasFocus = true;
            cursorBytePos = -1;
            InvalidateWindow();
            return true;
        }

        Rect findBtn(contentX + 210, cy, 80, 28);
        if (IsPointInRect(x, y, findBtn))
        {
            PatternSearch_Run();
            g_PatternSearch.hasFocus = false;
            InvalidateWindow();
            return true;
        }

        cy += 40;

        Rect prevBtn(contentX, cy, 120, 28);
        if (IsPointInRect(x, y, prevBtn))
        {
            PatternSearch_FindPrev();
            g_PatternSearch.hasFocus = false;
            InvalidateWindow();
            return true;
        }

        Rect nextBtn(contentX + 130, cy, 100, 28);
        if (IsPointInRect(x, y, nextBtn))
        {
            PatternSearch_FindNext();
            g_PatternSearch.hasFocus = false;
            InvalidateWindow();
            return true;
        }

        return false;
    }

    case BottomPanelState::Tab::Checksum:
    {
        int cy = contentY;
        cy += 25;

        Rect md5Check(contentX, cy, 16, 16);
        if (IsPointInRect(x, y, md5Check))
        {
            Checksum_ToggleMD5();
            InvalidateWindow();
            return true;
        }

        Rect sha1Check(contentX + 100, cy, 16, 16);
        if (IsPointInRect(x, y, sha1Check))
        {
            Checksum_ToggleSHA1();
            InvalidateWindow();
            return true;
        }

        Rect sha256Check(contentX + 200, cy, 16, 16);
        if (IsPointInRect(x, y, sha256Check))
        {
            Checksum_ToggleSHA256();
            InvalidateWindow();
            return true;
        }

        Rect crc32Check(contentX + 330, cy, 16, 16);
        if (IsPointInRect(x, y, crc32Check))
        {
            Checksum_ToggleCRC32();
            InvalidateWindow();
            return true;
        }

        cy += 35;

        Rect entireFileRadio(contentX, cy, 16, 16);
        if (IsPointInRect(x, y, entireFileRadio))
        {
            Checksum_SetModeEntireFile();
            InvalidateWindow();
            return true;
        }

        Rect selectionRadio(contentX + 150, cy, 16, 16);
        if (IsPointInRect(x, y, selectionRadio))
        {
            Checksum_SetModeSelection();
            InvalidateWindow();
            return true;
        }

        cy += 35;

        Rect compareBtn(contentX, cy, 100, 28);
        if (IsPointInRect(x, y, compareBtn))
        {
            Checksum_Compare();
            InvalidateWindow();
            return true;
        }

        Rect hashCalcBtn(contentX + 110, cy, 150, 28);
        if (IsPointInRect(x, y, hashCalcBtn))
        {
            Checksum_Compute();
            InvalidateWindow();
            return true;
        }

        if (IsPointInRect(x, y, bottomBounds))
        {
            return true;
        }

        return false;
    }

    case BottomPanelState::Tab::Compare:
    {
        int cy = contentY;
        cy += 25;
        cy += 30;

        Rect selectBtn(contentX, cy, 150, 32);
        if (IsPointInRect(x, y, selectBtn))
        {
            Compare_OpenFileDialog();
            Compare_Run();
            InvalidateWindow();
            return true;
        }

        if (IsPointInRect(x, y, bottomBounds))
        {
            return true;
        }

        return false;
    }
    }

    return false;
}
