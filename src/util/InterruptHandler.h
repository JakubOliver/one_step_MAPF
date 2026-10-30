#ifndef MAPF_INTERRUPT_HANDLER_H
#define MAPF_INTERRUPT_HANDLER_H

#include <csignal>

namespace util {

class InterruptHandler {
private:
    static inline volatile std::sig_atomic_t interrupted = 0;

    static void handle(int) {
        interrupted = 1;
    }

public:
    InterruptHandler() {
        interrupted = 0;
        std::signal(SIGINT, handle);
    }

    ~InterruptHandler() {
        std::signal(SIGINT, SIG_DFL);
    }

    bool wasInterrupted() const {
        return interrupted != 0;
    }
};

}

#endif //MAPF_INTERRUPT_HANDLER_H
