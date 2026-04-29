#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/shared.h"
#include <windows.h>
#include <cstring>

TEST_CASE("Testing IPC Circular Queue File Logic") {
    const char* testFileName = "test_queue.bin";
    const int recordCount = 2;

    // 1. Инициализация файла (Логика Receiver)
    HANDLE hFile = CreateFileA(testFileName, GENERIC_READ | GENERIC_WRITE,
                               0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(hFile != INVALID_HANDLE_VALUE);

    QueueHeader initHeader = { 0, 0, recordCount };
    DWORD bytesWritten;
    WriteFile(hFile, &initHeader, sizeof(QueueHeader), &bytesWritten, NULL);
    CloseHandle(hFile);

x    // 2. Имитация записи первого сообщения (Логика Sender)
    hFile = CreateFileA(testFileName, GENERIC_READ | GENERIC_WRITE,
                        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    REQUIRE(hFile != INVALID_HANDLE_VALUE);

    QueueHeader header;
    DWORD bytesRead;
    ReadFile(hFile, &header, sizeof(QueueHeader), &bytesRead, NULL);
    
    CHECK(header.writePtr == 0);
    CHECK(header.readPtr == 0);

    Message msg1;
    std::memset(msg1.text, 0, MESSAGE_LEN);
    std::strncpy(msg1.text, "HelloTest", MESSAGE_LEN - 1);

    LONG offset = static_cast<LONG>(sizeof(QueueHeader) + header.writePtr * sizeof(Message));
    SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
    WriteFile(hFile, &msg1, sizeof(Message), &bytesWritten, NULL);

    header.writePtr = (header.writePtr + 1) % header.maxSpaces;
    
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, &header, sizeof(QueueHeader), &bytesWritten, NULL);
    CloseHandle(hFile);

    hFile = CreateFileA(testFileName, GENERIC_READ | GENERIC_WRITE,
                        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    ReadFile(hFile, &header, sizeof(QueueHeader), &bytesRead, NULL);
    
    CHECK(header.writePtr == 1);
    CHECK(header.readPtr == 0);

    offset = static_cast<LONG>(sizeof(QueueHeader) + header.readPtr * sizeof(Message));
    SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
    
    Message readMsg;
    ReadFile(hFile, &readMsg, sizeof(Message), &bytesRead, NULL);
    
    CHECK(std::strcmp(readMsg.text, "HelloTest") == 0);

    header.readPtr = (header.readPtr + 1) % header.maxSpaces;
    
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, &header, sizeof(QueueHeader), &bytesWritten, NULL);
    CloseHandle(hFile);

    DeleteFileA(testFileName);
}
