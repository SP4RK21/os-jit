#include <iostream>
#include <sys/mman.h>
#include <cstring>
#include <sstream>
#include <algorithm>

const std::string MANUAL = "This program summarizes digits of given number (by default it's 121) \n"
        "Available commands:\n"
        "   help - if you need help \n"
        "   exit - to stop program and delete function from memory \n"
        "   modify [value] - to change number to execute program on (value should be positive) \n"
        "   execute - to execute program with currently set value\n"
        "Enjoy your time here :) \n";

const size_t codeSize = 73;
const size_t numberPosition = 14;
unsigned char functionCode[] = {
        0x55,
        0x48, 0x89, 0xe5,
        0xc7, 0x45, 0xfc, 0x00, 0x00, 0x00, 0x00,
        0xc7, 0x45, 0xf8, 0x79, 0x00, 0x00, 0x00,
        0xc7, 0x45, 0xf4, 0x00, 0x00, 0x00, 0x00,
        0x83, 0x7d, 0xf8, 0x00,
        0x0f, 0x8e, 0x21, 0x00, 0x00, 0x00,
        0x8b, 0x45, 0xf8,
        0x99,
        0xb9, 0x0a, 0x00, 0x00, 0x00,
        0xf7, 0xf9,
        0x03, 0x55, 0xf4,
        0x89, 0x55, 0xf4,
        0x8b, 0x55, 0xf8,
        0x89, 0xd0,
        0x99,
        0xf7, 0xf9,
        0x89, 0x45, 0xf8,
        0xe9, 0xd5, 0xff, 0xff, 0xff,
        0x8b, 0x45, 0xf4,
        0x5d,
        0xc3
};

typedef unsigned char (* function)();
void* memoryPtr;

void writeFunc() {
    memoryPtr = mmap(nullptr, codeSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memoryPtr == MAP_FAILED) {
        std::cerr << "Error while allocating memory" << std::endl;
        exit(EXIT_FAILURE);
    }
    memcpy(memoryPtr, functionCode, codeSize);
}

void free() {
    int status = munmap(memoryPtr, codeSize);
    if (status == -1) {
        std::cerr << "Error while memory release" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void changeMemProtection(int prot) {
    int status = mprotect(memoryPtr, codeSize, PROT_READ | prot);
    if (status == -1) {
        std::cerr << "Error while changing memory protection type" << std::endl;
        free();
        exit(EXIT_FAILURE);
    }
}

void modifyCode(int number) {
    changeMemProtection(PROT_WRITE);
    for (int i = numberPosition; i < numberPosition + 4; ++i) {
        ((char*) memoryPtr)[i] = static_cast<char>(number % 256);
        number /= 256;
    }
}

int execute() {
    changeMemProtection(PROT_EXEC);
    return function(memoryPtr)();
}

int main() {
    std::string args;
    std::string command;
    std::cout << MANUAL << std::endl;
    std::cout << "Writing function to memory..." << std::endl;
    writeFunc();
    std::cout << "Writing to memory successfully finished!" << std::endl;
    std::cout << "jitCompiler$ ";
    long val = 121;
    while (getline(std::cin, args)) {
        std::istringstream stream(args);
        stream >> command;
        if (command == "help") {
            std::cout << MANUAL << std::endl;
        } else if (command == "exit") {
            free();
            std::cout << "Memory cleared, have a god day and goodbye :)" << std::endl;
            exit(EXIT_SUCCESS);
        } else if (command == "modify") {
            long prev = val;
            stream >> val;
            if (val > 0 && val <= INT32_MAX) {
                modifyCode((int) val);
                std::cout << "Value modified. Type 'execute' to see the result" << std::endl;
            } else {
                val = prev;
                std::cout << "Not a positive integer. Try again" << std::endl;
            }
        } else if (command == "execute") {
            std::cout << "Sum of digits of number " << val << " is " << execute() << std::endl;
        } else {
            std::cout << "Unknown command. Type 'help' to see what can be done here" << std::endl;
        }
        std::cout << "jitCompiler$ ";
    }
}