#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

#include <sys/mman.h>
#include <unistd.h>


unsigned char funCode[] = {
        0x55,                        //      	push   %rbp             //
        0x48, 0x89, 0xe5,            //         mov    %rsp,%rbp        //
        0x89, 0x7d, 0xfc,            //       	mov    %edi,-0x4(%rbp)  //  int inc(int number){
        0x8b, 0x45, 0xfc,            //       	mov    -0x4(%rbp),%eax  //     return number + 42;
        0x83, 0xc0, 0x2a,            //       	add    $0x2a,%eax       //  }
        0x5d,                        //     	pop    %rbp             //
        0xc3,                        //         retq                    //
};

const int funSize = 15;
const int pos = 12;

void printErr(const std::string& message) {
    fprintf(stderr, "ERROR %s: %s\n", message.c_str(), strerror(errno));
}

struct Function {
    Function(unsigned char* code, size_t size) : size(size), prot(PROT_READ | PROT_WRITE) {
        memory = mmap(nullptr, size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (memory == MAP_FAILED) {
            printErr("Unable to allocate writable memory");

            exit(EXIT_FAILURE);
        }

        memcpy(memory, code, sizeof(unsigned char) * size);
    }

    ~Function() {
        if (munmap(memory, size) == -1) {
            printErr("Unable to deallocate memory");

            exit(EXIT_FAILURE);
        }
    }

    void changeProt(int newProt) {
        if (prot != newProt) {

            if (mprotect(memory, size, newProt) == -1) {
                printErr("Unable to change prot");

                exit(EXIT_FAILURE);
            }
        }

        prot = newProt;
    }

    void patch(unsigned char value, size_t pos) {
        changeProt(PROT_READ | PROT_WRITE);

        ((unsigned char*) memory)[pos] = value;
    }

    int execute(int number) {
        changeProt(PROT_READ | PROT_EXEC);
        auto f = reinterpret_cast<int (*)(int)>(memory);
        return f(number);
    };

    void* memory;
    size_t size;
    int prot;
};


int main() {
    Function function = Function(funCode, funSize);

    std::string line;
    printf("Tell me your number and i'll give you answer...\n");

    while (true) {
        std::string answer;
        std::cin >> answer;

        if (answer == "ok" || answer == "execute") {
            int number;
            std::cin >> number;

            printf("My answer is: %d!\n", function.execute(number));
        } else if (answer == "now" || answer == "change") {
            int number;
            std::cin >> number;
            int casted = (unsigned char) number;

            function.patch(casted, pos);
            printf("My mind changed.\n");
        } else if (answer == "no" || answer == "exit") {
            break;
        } else {
            printf("Unknown number\n");
        }
    }

    exit(EXIT_SUCCESS);
}