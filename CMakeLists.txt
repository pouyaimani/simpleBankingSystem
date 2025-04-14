cmake_minimum_required(VERSION 3.9)
project(BankingSystem)

# Set C++ standard (if applicable)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)  # Find OpenSSL

add_executable(BankingSystem bankingSystem.cpp)
target_link_libraries(BankingSystem PRIVATE cpprestsdk::cpprest Boost::boost OpenSSL::SSL OpenSSL::Crypto)