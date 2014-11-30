#include "reactor.h"
#include <iostream>

int main() {
    Reactor reactor;
    FD& stdin = reactor.take_fd(0);
    //FD& stdout = reactor.take_fd(1);

    stdin.on_read_ready = [&]() {
        std::cout << "on_read_ready" << std::endl;
        while(true) {
            char buff[10];
            auto size = stdin.read(buff, sizeof(buff));
            std::cout << "read bytes=" << size << std::endl;
            if(size == 0) break;
        }
    };

    reactor.run();
}