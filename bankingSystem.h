#pragma once

#include "transaction.h"
#include "account.h"
#include <iostream>
#include <vector>
#include <ctime>
#include <map>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <mutex>

using namespace std;
using namespace web;
using namespace web::http;  
using namespace web::http::experimental::listener;

#define SHEBA_LENGTH 26

// سیستم بانکی
class BankingSystem {
    private:
        vector<Transaction> transactions;
        map<string, Account> accounts;
        http_listener listener;
        mutable std::mutex dataMutex;
    
    public:
        BankingSystem() : listener("http://localhost:80/api/sheba") {
            // ایجاد چند حساب نمونه برای تست
            accounts["IR123456789012345678901234"] = Account("IR123456789012345678901234", 1000000000);
            accounts["IR987654321098765432109876"] = Account("IR987654321098765432109876", 500000000);
            
            // تنظیم هندلرهای API
            listener.support(methods::POST, std::bind(&BankingSystem::handle_post, this, std::placeholders::_1));
            listener.support(methods::GET, std::bind(&BankingSystem::handle_get, this, std::placeholders::_1));
            listener.support(methods::PUT, std::bind(&BankingSystem::handle_put, this, std::placeholders::_1));
        }
    
        void start() {
            try {
                listener
                    .open()
                    .then([&]() { cout << "Listening for requests at: " << listener.uri().to_string() << endl; })
                    .wait();
            } catch (exception const & e) {
                cerr << "Error: " << e.what() << endl;
            }
        }
    
    private:
        // هندلر POST برای ایجاد درخواست انتقال
        void handle_post(http_request request) {
            request
                .extract_json()
                .then([this, request](json::value body) {
                    {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    try {
                        // استخراج داده‌های درخواست
                        long long amount = body["price"].as_number().to_int64();
                        string fromSheba = body["fromShebaNumber"].as_string();
                        string toSheba = body["ToShebaNumber"].as_string();
                        string note = body["note"].as_string();
    
                        // اعتبارسنجی شماره شبا
                        if (fromSheba.length() != SHEBA_LENGTH || toSheba.length() != SHEBA_LENGTH || 
                            fromSheba.substr(0, 2) != "IR" || toSheba.substr(0, 2) != "IR") {
                            json::value response;
                            response["message"] = json::value("Invalid Sheba number");
                            response["code"] = json::value("INVALID_SHEBA");
                            request.reply(status_codes::BadRequest, response);
                            return;
                        }
                        // بررسی وجود حساب مبدا و مقصد
                        if (accounts.find(fromSheba) == accounts.end()) {
                            json::value response;
                            response["message"] = json::value("Source account not found");
                            response["code"] = json::value("ACCOUNT_NOT_FOUND");
                            request.reply(status_codes::NotFound, response);
                            return;
                        }
    
                        if (accounts.find(toSheba) == accounts.end()) {
                            json::value response;
                            response["message"] = json::value("Destination account not found");
                            response["code"] = json::value("ACCOUNT_NOT_FOUND");
                            request.reply(status_codes::NotFound, response);
                            return;
                        }
                        std::cout << "source account befor transaction : \n" << "sheba number = " << fromSheba << std::endl << 
                        "account balance = " << accounts[fromSheba].balance << endl;
                        // بررسی موجودی کافی
                        if (!accounts[fromSheba].canReserve(amount)) {
                            json::value response;
                            response["message"] = json::value("Insufficient balance");
                            response["code"] = json::value("INSUFFICIENT_BALANCE");
                            request.reply(status_codes::BadRequest, response);
                            return;
                        }
    
                        // رزرو مبلغ
                        if (!accounts[fromSheba].reserve(amount)) {
                            json::value response;
                            response["message"] = json::value("Reservation failed");
                            response["code"] = json::value("RESERVATION_FAILED");
                            request.reply(status_codes::InternalError, response);
                            return;
                        }

                        std::cout << "source account after reserving : \n" << "sheba number = " << fromSheba << endl << 
                        "account balance = " << accounts[fromSheba].balance << endl;

                        std::cout << "destination account : \n" << "sheba number = " << toSheba << endl << 
                        "account balance = " << accounts[toSheba].balance << endl;
                        
                        // ایجاد تراکنش
                        Transaction trans(fromSheba, toSheba, amount, note);
                        {
                            std::lock_guard<std::mutex> lock(dataMutex);
                            transactions.push_back(trans);
                        }
                        
    
                        // پاسخ موفقیت‌آمیز
                        json::value response;
                        response["message"] = json::value("Request is saved successfully and is in pending status");
                        
                        json::value request_obj;
                        request_obj["id"] = json::value(trans.id);
                        request_obj["price"] = json::value(amount);
                        request_obj["status"] = json::value("pending");
                        request_obj["fromShebaNumber"] = json::value(fromSheba);
                        request_obj["ToShebaNumber"] = json::value(toSheba);
                        request_obj["createdAt"] = json::value(trans.createdAt);
                        
                        response["request"] = request_obj;
                        
                        request.reply(status_codes::OK, response);
                    } catch (const exception &e) {
                        json::value response;
                        response["message"] = json::value("Invalid request format");
                        response["code"] = json::value("INVALID_REQUEST");
                        request.reply(status_codes::BadRequest, response);
                    }
                    }
                })
                .wait();
        }
    
        // هندلر GET برای دریافت لیست درخواست‌ها
        void handle_get(http_request request) {
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                            // مرتب‌سازی بر اساس زمان ایجاد (قدیمی‌ترین اول)
                sort(transactions.begin(), transactions.end(), 
                    [](const Transaction& a, const Transaction& b) { 
                        return a.createdAt < b.createdAt; 
                });
            }
    
            json::value response;
            json::value requests_array = json::value::array();
            
            {
            std::lock_guard<std::mutex> lock(dataMutex);
            for (size_t i = 0; i < transactions.size(); ++i) {
                const auto& t = transactions[i];
                
                json::value request_obj;
                request_obj["id"] = json::value(t.id);
                request_obj["price"] = json::value(t.amount);
                request_obj["status"] = json::value(t.status);
                request_obj["fromShebaNumber"] = json::value(t.fromSheba);
                request_obj["ToShebaNumber"] = json::value(t.toSheba);
                request_obj["createdAt"] = json::value(t.createdAt);
                
                requests_array[i] = request_obj;
            }
            }
    
            response["requests"] = requests_array;
            request.reply(status_codes::OK, response);
        }
    
        // هندلر PUT برای تایید یا رد درخواست
        void handle_put(http_request request) {
            // استخراج request-id از URL
            auto path = uri::split_path(request.relative_uri().to_string());
            if (path.empty()) {
                json::value response;
                response["message"] = json::value("Request ID is required");
                response["code"] = json::value("MISSING_REQUEST_ID");
                request.reply(status_codes::BadRequest, response);
                return;
            }
    
            string requestId = path[0];
    
            request
                .extract_json()
                .then([this, request, requestId](json::value body) {
                    {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    try {
                        string status = body["status"].as_string();
                        string note = body["note"].as_string();
    
                        auto it = find_if(transactions.begin(), transactions.end(), 
                                        [requestId](const Transaction& t) { return t.id == requestId; });
    
                        if (it == transactions.end()) {
                            json::value response;
                            response["message"] = json::value("Request not found");
                            response["code"] = json::value("REQUEST_NOT_FOUND");
                            request.reply(status_codes::NotFound, response);
                            return;
                        }
    
                        if (it->status != "pending") {
                            json::value response;
                            response["message"] = json::value("Request already processed");
                            response["code"] = json::value("ALREADY_PROCESSED");
                            request.reply(status_codes::BadRequest, response);
                            return;
                        }
    
                        if (status == "confirmed") {
                            // تایید تراکنش
                            if (accounts[it->fromSheba].confirmReserved(it->amount)) {
                                accounts[it->toSheba].balance += it->amount;
                                it->status = "confirmed";
                                it->note = note;

                                std::cout << "transaction confirmed : \n" << "source sheba = " << it->fromSheba << std::endl <<
                                "account balance = " << accounts[it->fromSheba].balance << endl <<
                                "destination sheba = " << it->toSheba.substr(0, 2) << std::endl <<
                                "account balance = " << accounts[it->toSheba].balance << endl;
    
                                json::value response;
                                response["message"] = json::value("Request is Confirmed!");
                                
                                json::value request_obj;
                                request_obj["id"] = json::value(it->id);
                                request_obj["price"] = json::value(it->amount);
                                request_obj["status"] = json::value("confirmed");
                                request_obj["fromShebaNumber"] = json::value(it->fromSheba);
                                request_obj["ToShebaNumber"] = json::value(it->toSheba);
                                request_obj["createdAt"] = json::value(it->createdAt);
                                
                                response["request"] = request_obj;
                                
                                request.reply(status_codes::OK, response);
                            } else {
                                json::value response;
                                response["message"] = json::value("Confirmation failed");
                                response["code"] = json::value("CONFIRMATION_FAILED");
                                request.reply(status_codes::InternalError, response);
                            }
                        } else if (status == "canceled") {
                            // رد تراکنش
                            if (accounts[it->fromSheba].cancelReserved(it->amount)) {
                                it->status = "canceled";
                                it->note = note;

                                std::cout << "transaction canceled : \n" << "source sheba = " << it->fromSheba.substr(0, 2) << std::endl <<
                                "account balance = " << accounts[it->fromSheba].balance << endl <<
                                "destination account number = " << it->toSheba << std::endl <<
                                "account balance = " << accounts[it->toSheba].balance << endl;
    
                                json::value response;
                                response["message"] = json::value("Request is Canceled!");
                                
                                json::value request_obj;
                                request_obj["id"] = json::value(it->id);
                                request_obj["price"] = json::value(it->amount);
                                request_obj["status"] = json::value("canceled");
                                request_obj["fromShebaNumber"] = json::value(it->fromSheba);
                                request_obj["ToShebaNumber"] = json::value(it->toSheba);
                                request_obj["createdAt"] = json::value(it->createdAt);
                                
                                response["request"] = request_obj;
                                
                                request.reply(status_codes::OK, response);
                            } else {
                                json::value response;
                                response["message"] = json::value("Cancellation failed");
                                response["code"] = json::value("CANCELLATION_FAILED");
                                request.reply(status_codes::InternalError, response);
                            }
                        } else {
                            json::value response;
                            response["message"] = json::value("Invalid status");
                            response["code"] = json::value("INVALID_STATUS");
                            request.reply(status_codes::BadRequest, response);
                        }
                    } catch (const exception &e) {
                        json::value response;
                        response["message"] = json::value("Invalid request format");
                        response["code"] = json::value("INVALID_REQUEST");
                        request.reply(status_codes::BadRequest, response);
                    }
                    }
                })
                .wait();
        }
    };