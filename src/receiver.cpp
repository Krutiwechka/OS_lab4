#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "shared.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;

int main() {
    string fileName;
    int recordCount;
    int senderCount;

    cout << "Enter binary file name: ";
    cin >> fileName;
    cout << "Enter maximum records amount in file: ";
    cin >> recordCount;

    HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return static_cast<int>(GetLastError());

    QueueHeader header = { 0, 0, recordCount };
    DWORD bytesWritten;
    WriteFile(hFile, &header, sizeof(QueueHeader), &bytesWritten, NULL);
    CloseHandle(hFile);

    cout << "Enter Senders amount: ";
    cin >> senderCount;

    HANDLE mtxFile = CreateMutexA(NULL, FALSE, "Global\\Lab4_FileMutex"); //мьютекс файла
    HANDLE semEmpty = CreateSemaphoreA(NULL, recordCount, recordCount, "Global\\Lab4_SemEmpty");  //пустые строки
    HANDLE semFull = CreateSemaphoreA(NULL, 0, recordCount, "Global\\Lab4_SemFull"); //занятые строки
    
    HANDLE semReady = CreateSemaphoreA(NULL, 0, senderCount, "Global\\Lab4_SemReady"); //готовые сендеры

    vector<PROCESS_INFORMATION> pi(senderCount);
    for (int i = 0; i < senderCount; ++i) {
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi[i], sizeof(pi[i]));

        string cmdArgs = "sender.exe " + fileName;
        vector<char> cmdArgsChar(cmdArgs.begin(), cmdArgs.end());
        cmdArgsChar.push_back('\0');

        BOOL success = CreateProcessA(NULL, cmdArgsChar.data(), NULL, NULL, FALSE,
                                      CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi[i]);
        if (!success) return static_cast<int>(GetLastError());
    }

    cout << "Waiting for all Senders to be ready..." << endl;
    for (int i = 0; i < senderCount; ++i) {
        WaitForSingleObject(semReady, INFINITE);
    }
    cout << "All Senders are ready. Working phase started." << endl;

    while (true) {
        cout << "\nChoose action:\n1. Read message\n2. Exit" << endl;
        int choice;
        cin >> choice;

        if (choice == 2) {
            break;
        }

        if (choice == 1) {
            cout << "Waiting for message (if queue is empty, thread will block)..." << endl;
            
            WaitForSingleObject(semFull, INFINITE);            
            WaitForSingleObject(mtxFile, INFINITE);

            hFile = CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            QueueHeader currentHeader;
            DWORD bytesRead;
            ReadFile(hFile, &currentHeader, sizeof(QueueHeader), &bytesRead, NULL);
            LONG offset = static_cast<LONG>(sizeof(QueueHeader) + currentHeader.readPtr * sizeof(Message));
            SetFilePointer(hFile, offset, NULL, FILE_BEGIN);

            Message msg;
            ReadFile(hFile, &msg, sizeof(Message), &bytesRead, NULL);
            cout << ">>> Received message: " << msg.text << endl;
            currentHeader.readPtr = (currentHeader.readPtr + 1) % currentHeader.maxSpaces;
            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
            WriteFile(hFile, &currentHeader, sizeof(QueueHeader), &bytesWritten, NULL);

            CloseHandle(hFile);
            ReleaseMutex(mtxFile);
            ReleaseSemaphore(semEmpty, 1, NULL);
        }
    }
    for (int i = 0; i < senderCount; ++i) {
        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);
    }
    CloseHandle(semReady);
    CloseHandle(semFull);
    CloseHandle(semEmpty);
    CloseHandle(mtxFile);

    return 0;
}
