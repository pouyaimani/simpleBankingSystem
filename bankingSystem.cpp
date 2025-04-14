#include <iomanip>
#include <thread>
#include "bankingSystem.h"


int main() {
    BankingSystem bank;
    bank.start();

    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}