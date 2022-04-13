#include <iostream>
#include <windows.h>
#include <time.h>
#include <tchar.h>
#include <string>
#include <fstream>

using namespace std;

// student ID: 930524
DWORD const pageCnt = 3 + 0 + 5 + 2 + 4; // 14
DWORD const pageSize = 4096;

int main(int argc, char* argv[])
{
    string filename = "mapdata.txt";
    srand(time(NULL));

    // инициализация входных параметров
    BOOL WoR = atoi(argv[1]); // 0 - Писатель, 1 - Читатель
    DWORD processNum = atoi(argv[2]);
    DWORD startTime = atoi(argv[3]);

    // инициализация лог-файла
    string logName = "D:\\";
    logName += WoR ? "Reader" : "Writer";
    logName += argv[2];
    logName += "_log.txt";
    ofstream fout;
    fout.open(logName);

    // инициализация проецируемого файла
    HANDLE hFile;
    LPVOID lpFileMapping;
    hFile = OpenFileMappingA(
        WoR ? GENERIC_READ : GENERIC_WRITE,
        FALSE,
        filename.c_str()
    );
    lpFileMapping = MapViewOfFile(
        hFile,
        WoR ? FILE_MAP_READ : FILE_MAP_WRITE,
        0,
        0,
        0
    );
    VirtualLock(lpFileMapping, pageCnt * pageSize);

    // инициализация семафоров
    HANDLE writeSem[pageCnt];
    HANDLE readSem[pageCnt];
    string writeSemName = "writeSem";
    string readSemName = "readSem";
    string semName;
    for (int i = 0; i < pageCnt; ++i)
    {
        semName = writeSemName;
        semName += to_string(i);
        writeSem[i] = OpenSemaphoreA(
            SEMAPHORE_ALL_ACCESS,
            FALSE,
            semName.c_str()
        );

        semName = readSemName;
        semName += to_string(i);
        readSem[i] = OpenSemaphoreA(
            SEMAPHORE_ALL_ACCESS,
            FALSE, 
            semName.c_str()
        );
    }

    // инициализация буфера данных
    char buff[pageSize];
    if (WoR) memset(buff, 0, pageSize); // Read
    else memset(buff, 'A' + processNum, pageSize); // Write

    // начало работы с файлом
    DWORD currentPage;
    LPSTR dataStart;
    for (int i = 1; i <= 3; i++)
    {
        // ожидание доступной страницы
        fout << "WAITING 0 " << clock() + startTime << endl;
        if (WoR) // Читатель
            currentPage = WaitForMultipleObjects(
                pageCnt,
                readSem,
                FALSE,
                INFINITE
            );
        else // Писатель
            currentPage = WaitForMultipleObjects(
                pageCnt,
                writeSem,
                FALSE,
                INFINITE
            );
       Sleep(50 + rand() % 251);

        // обработка страницы
        fout << "PROCESSING " << currentPage + 1 << " " << clock() + startTime << endl;
        dataStart = (LPSTR)lpFileMapping;
        dataStart += currentPage * pageSize;
        if (WoR) memcpy(buff, dataStart, pageSize); // Read
        else memcpy(dataStart, buff, pageSize); // Write
        Sleep(500 + rand() % 1001);

        // Заверешние
        fout << "FINISHING -1 " << clock() + startTime << endl;
        if (WoR) // Читатель
            ReleaseSemaphore(
                writeSem[currentPage],
                1,
                NULL
            );
        else // Писатель
            ReleaseSemaphore(
                readSem[currentPage],
                1,
                NULL
            );
       Sleep(50 + rand() % 251);
    }
    return 0;
}