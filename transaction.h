#pragma once

#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

class Transaction {
    public:
        string id;
        string fromSheba;
        string toSheba;
        long long amount;
        string status; // "pending", "confirmed", "canceled"
        string note;
        string createdAt;
    
        Transaction(string from, string to, long long amt, string nt) {
            fromSheba = from;
            toSheba = to;
            amount = amt;
            note = nt;
            status = "pending";
            createdAt = currentDateTime();
            id = generateId();
        }
    
        string currentDateTime() {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            stringstream ss;
            ss << 1900 + ltm->tm_year << "-" 
               << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
               << setw(2) << setfill('0') << ltm->tm_mday << "T"
               << setw(2) << setfill('0') << ltm->tm_hour << ":"
               << setw(2) << setfill('0') << ltm->tm_min << ":"
               << setw(2) << setfill('0') << ltm->tm_sec << "-02:00";
            return ss.str();
        }
    
        string generateId() {
            static std::atomic<int> counter{0};
            return "req-" + to_string(++counter) + "-" + to_string(time(0));
        }
    };