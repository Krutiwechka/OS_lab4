#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include "shared.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    string fileName = argv[1];

    HANDLE mtxFile = OpenMutexA(SYNCHRONIZE, FALSE, "Global\\Lab4_FileMutex");
    HANDLE semEmpty = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, "Global\\Lab4_SemEmpty");
    HANDLE semFull = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, "Global\\Lab4_SemFull");
    HANDLE semReady = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, "Global\\Lab4_SemReady");

    if (!mtxFile || !semEmpty || !semFull || !semReady) {
        return static_cast<int>(GetLastError());
    }

    ReleaseSemaphore(semReady, 1, NULL);

    while (true) {
        cout << "\nChoose action:\n1. Send message\n2. Exit" << endl;
        int choice;
        cin >> choice;

        if (choice == 2) {
            break;
        }

        if (choice == 1) {
            cout << "Enter message text (less than 20 chars): ";
            string textInput;
            cin >> textInput;

            Message msg;
            std::memset(msg.text, 0, MESSAGE_LEN);
            std::strncpy(msg.text, textInput.c_str(), MESSAGE_LEN - 1);

            cout << "Waiting for empty space in file (if full, thread will block)..." << endl;
            
            WaitForSingleObject(semEmpty, INFINITE);
            
            WaitForSingleObject(mtxFile, INFINITE);

            HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            QueueHeader currentHeader;
            DWORD bytesRead;
            ReadFile(hFile, &currentHeader, sizeof(QueueHeader), &bytesRead, NULL);

            LONG offset = static_cast<LONG>(sizeof(QueueHeader) + currentHeader.writePtr * sizeof(Message));
            SetFilePointer(hFile, offset, NULL, FILE_BEGIN);

            DWORD bytesWritten;
            WriteFile(hFile, &msg, sizeof(Message), &bytesWritten, NULL);

            currentHeader.writePtr = (currentHeader.writePtr + 1) % currentHeader.maxSpaces;

            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
            WriteFile(hFile, &currentHeader, sizeof(QueueHeader), &bytesWritten, NULL);

            CloseHandle(hFile);
            ReleaseMutex(mtxFile);

            ReleaseSemaphore(semFull, 1, NULL);
            cout << "Message successfully sent!" << endl;
        }
    }

    CloseHandle(semReady);
    CloseHandle(semFull);
    CloseHandle(semEmpty);
    CloseHandle(mtxFile);

    return 0;
}
