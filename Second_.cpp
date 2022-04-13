#include <iostream>
#include <windows.h>
#include <time.h>
#include <tchar.h>
#include <string>

using namespace std;

void stringToLPSTR(string str, char* data);

// student ID: 930524
DWORD const pageCnt = 3 + 0 + 5 + 2 + 4; // 14
DWORD const pageSize = 4096;

int main() {
    string filename = "D:\\data.txt";
    string mapfilename = "mapdata.txt";
    // создание проецируемого файла
    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    HANDLE hFileMapping = CreateFileMappingA(
        hFile,
        NULL,
        PAGE_READWRITE,
        0,
        pageCnt * pageSize,
        mapfilename.c_str()
    );
    
    // создание семафоров
    HANDLE writeSem[pageCnt];
    HANDLE readSem[pageCnt];
    string writeSemName = "writeSem";
    string readSemName = "readSem";
    string semName;

    for (int i = 0; i < pageCnt; ++i) {
        semName = writeSemName;
        semName += to_string(i);
        writeSem[i] = CreateSemaphoreA(NULL, 1, 1, semName.c_str());

        semName = readSemName;
        semName += to_string(i);
        readSem[i] = CreateSemaphoreA(NULL, 0, 1, semName.c_str());
    }

    // создание процессов
    HANDLE processes[14];
    STARTUPINFOA startup[14];

    memset(startup, 0, sizeof(STARTUPINFO[14]));
    PROCESS_INFORMATION processInfo[14];
    memset(processInfo, 0, sizeof(PROCESS_INFORMATION[14]));
    string command = "D:\\VS\\repos\\LABA4_1_1\\x64\\Debug\\LABA4_1_1.exe";
    char commandEx[MAX_PATH];

    // создание писателей
    cout << "START:" << endl;
    for (int i = 0; i < 7; ++i)
    {
        stringToLPSTR(
            command + ' ' +
            to_string(0) + ' ' +
            to_string(i) + ' ' +
            to_string(clock()), commandEx);
        cout << commandEx << endl;
        CreateProcessA(
            NULL,
            commandEx,
            NULL,
            NULL,
            0,
            0,
            NULL,
            NULL,
            &startup[i],
            &processInfo[i]
        );
        processes[i] = processInfo[i].hProcess;
    }

    // создание читателей
    for (int i = 0; i < 7; ++i)
    {
        stringToLPSTR(
            command + ' ' +
            to_string(1) + ' ' +
            to_string(i) + ' ' +
            to_string(clock()), commandEx);
        cout << commandEx << endl;
        CreateProcessA(
            NULL,
            commandEx,
            NULL,
            NULL,
            0,
            0,
            NULL,
            NULL,
            &startup[7 + i],
            &processInfo[7 + i]
        );
        processes[7 + i] = processInfo[7 + i].hProcess;
    }

    cout << "WAITING ALL PROCESSES...";
    WaitForMultipleObjects(14, processes, TRUE, INFINITE);
    CloseHandle(hFile);
    CloseHandle(hFileMapping);
    return 0;
}

void stringToLPSTR(string str, char* data) {
    int i;
    for (i = 0; i < str.length(); ++i) {
        data[i] = str[i];
    }
    data[i] = 0;
}