# Simple C++ Banking System (REST API)

A basic banking system implemented in C++ using the [`cpprestsdk`](https://github.com/microsoft/cpprestsdk) (aka Casablanca) to provide a RESTful API for creating, confirming, canceling, and listing money transfer requests.

This project is intended for learning and demonstration purposes, focusing on **thread-safe** handling of concurrent HTTP requests and transaction reservation.

---

## Features

- Account and transaction management
- Create a money transfer request (`POST`)
- Confirm or cancel a transaction (`PUT`)
- List all transactions (`GET`)
- Thread-safe with mutex-protected shared data
- Basic request validation and status handling

---

## API Endpoints

### `POST /api/sheba`

Creates a pending transfer between accounts.

**Request Body:**

```json
{
  "fromShebaNumber": "IR123456789012345678901234",
  "ToShebaNumber": "IR987654321098765432109876",
  "price": 100000,
  "note": "Rent payment"
}
```

**Response (200 OK):**

```json
{
  "message": "Request is saved successfully and is in pending status",
  "request": {
    "id": "req-1-1713100000",
    "price": 100000,
    "status": "pending",
    "fromShebaNumber": "IR123456789012345678901234",
    "ToShebaNumber": "IR987654321098765432109876",
    "createdAt": "2025-04-14T18:00:00-02:00"
  }
}
```

---

### `GET /api/sheba`

Returns all transactions sorted by creation time (oldest first).

---

### `PUT /api/sheba/{id}`

Updates status of a transfer (confirm or cancel).

**Request Body:**

```json
{
  "status": "confirmed",
  "note": "Approved by admin"
}
```

---

## ⚙️ Build Instructions 

### Prerequisites

- C++17-compatible compiler (e.g., `clang++`)
- [CMake](https://cmake.org/) >= 3.10
- [`cpprestsdk`](https://github.com/microsoft/cpprestsdk)

### Install dependencies

#### macOS

```bash
brew install vcpkg
vcpkg install cpprestsdk
```

Or with **Homebrew** (recommended on macOS):

```bash
brew install cpprestsdk
```

#### Linux

```bash
sudo apt update
sudo apt install -y cmake g++ libboost-all-dev libssl-dev libcpprest-dev
```

#### Windows

Windows (Visual Studio)
Visual Studio 2019 or later with:
C++ development tools
CMake
Install vcpkg:

```bash
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install cpprestsdk
```

Use CMake with the vcpkg toolchain file:

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake -B build -S .
cmake --build build
```

### Build with CMake

1. Build:

```bash
mkdir build
cd build
cmake ..
make
```

2. Run:

```bash
sudo ./BankingSystem
```

> Server listens on `http://localhost:80/api/sheba`

---

## Sample cURL Commands

**Create a transaction:**

```bash
curl -X POST http://localhost:80/api/sheba \
-H "Accept: application/json" \
-H "Content-Type: application/json" \
-d '{
    "price": 200000000,
    "fromShebaNumber": "IR123456789012345678901234",
    "ToShebaNumber": "IR987654321098765432109876",
    "note": "transaction explanation"
}'
```

**List transactions:**

```bash
curl -X GET http://localhost:80/api/sheba \
-H "Accept: application/json"
```

**Confirm a transaction:**

```bash
curl -X PUT http://localhost:80/api/sheba/[request id] \
-H "Accept: application/json" \
-H "Content-Type: application/json" \
-d '{
    "status": "confirmed",
    "note": "confirmed by operator"
}'
```

**Cancel a transaction:**

```bash
curl -X PUT http://localhost:80/api/sheba/[request id] \
-H "Accept: application/json" \
-H "Content-Type: application/json" \
-d '{
    "status": "canceled",
    "note": "canceled by operator"
}'
```

---

## Thread Safety

- All accesses to shared `accounts` and `transactions` data structures are guarded by a `std::mutex`.
- Transaction ID generation uses an `std::atomic<int>` counter.

---

## Limitations

- All data is in-memory (no persistence)
- No authentication or authorization
- Basic request validation only
- Hardcoded timezone in timestamp (`-02:00`)

---

## License

MIT (or specify your own license)

---

## Author

Pouya Imani - (https://github.com/pouyaimani)
