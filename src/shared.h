#pragma once

const int MESSAGE_LEN = 20;

struct Message {
    char text[MESSAGE_LEN];
};

struct QueueHeader {
    int readPtr;   
    int writePtr;  
    int maxSpaces;
};
