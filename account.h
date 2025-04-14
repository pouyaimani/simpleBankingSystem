#pragma once


#include <string>
#include <algorithm>

using namespace std;

class Account {
    public:
        string sheba;
        long long balance;
        long long reserved;
    
        Account(string sh, long long bal) : sheba(sh), balance(bal), reserved(0) {}
        Account() = default;
        Account(const Account& other) 
        : sheba(other.sheba), 
          balance(other.balance), 
          reserved(other.reserved) {}
    
        // Move constructor (efficient transfer)
        Account(Account&& other) noexcept 
        : sheba(std::move(other.sheba)),
          balance(other.balance),
          reserved(other.reserved) {
            other.balance = 0;
            other.reserved = 0;
        }
    
        // Copy assignment operator
        Account& operator=(const Account& other) {
                if (this != &other) { // Self-assignment check
                    sheba = other.sheba;
                    balance = other.balance;
                    reserved = other.reserved;
                }
                return *this;
        }
        
        // Move assignment operator
        Account& operator=(Account&& other) noexcept {
            if (this != &other) { // Self-assignment check
                sheba = std::move(other.sheba);
                balance = other.balance;
                reserved = other.reserved;
                other.balance = 0;
                other.reserved = 0;
            }
            return *this;
        }
    
    
        bool canReserve(long long amount) {
            return (balance - reserved) >= amount;
        }
    
        bool reserve(long long amount) {
            if (canReserve(amount)) {
                reserved += amount;
                return true;
            }
            return false;
        }
    
        bool confirmReserved(long long amount) {
            if (reserved >= amount) {
                reserved -= amount;
                balance -= amount;
                return true;
            }
            return false;
        }
    
        bool cancelReserved(long long amount) {
            if (reserved >= amount) {
                reserved -= amount;
                return true;
            }
            return false;
        }
    };