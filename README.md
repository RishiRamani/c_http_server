# 🚀 C HTTP Server (POSIX, Multi-threaded, JSON API)

A lightweight HTTP/1.1 web server built from scratch in **C using POSIX sockets**, supporting persistent connections, multithreading, and a full CRUD API with JSON responses.

---

## 🧠 Overview

This project implements a low-level HTTP server without using any external frameworks.
It handles raw TCP connections, parses HTTP requests manually, and serves both API responses and static files.

---

## ✨ Features

* 🔌 **TCP Server (POSIX sockets)**
* 🌐 **HTTP/1.1 Support**

  * Request parsing (method, path, headers)
  * Persistent connections (`keep-alive`)
* ⚡ **Multithreading (pthreads)**

  * Concurrent client handling
* 🔒 **Thread Safety**

  * Mutex-protected shared storage
* 📦 **CRUD API**

  * `POST` → Create
  * `GET` → Read (all / by id)
  * `PUT` → Update
  * `DELETE` → Delete
* 🧾 **JSON Responses**
* 📁 **Static File Serving**

  * HTML, CSS, JS, favicon
* 🧠 **Manual TCP Buffer Handling**

  * Handles partial reads using `Content-Length`

---

## 📂 Project Structure

```
.
├── src/
│   ├── main.c              # Entry point
│   ├── server.c            # Socket setup, accept loop, threading
│   ├── http.c              # HTTP request parsing + handler dispatch
│   ├── handlers.c          # GET / POST / PUT / DELETE API logic
│   ├── storage.c           # In-memory CRUD storage (thread-safe, mutex-protected)
│   ├── utils.c             # JSON response builders + logging + MIME types
│   │
│   └── include/
│       ├── config.h        # Configuration: buffers, storage size, port
│       ├── server.h        # Server setup interface
│       ├── http.h          # Request handler interface
│       ├── handlers.h      # HTTP method handler interfaces
│       ├── storage.h       # Storage CRUD interface
│       └── utils.h         # Utility functions interface
│
├── public/
│   ├── index.html          # Frontend tester UI
│   ├── style.css           # Styling
│   ├── script.js           # API client code
│   └── favicon.ico         # Icon
│
├── Makefile                # Build configuration
└── README.md               # This file
```

---

## ⚙️ Build & Run

### 🔨 Compile

```bash
make
```

### ▶️ Run server

```bash
./server
```

Server runs on:

```
http://localhost:8080
```

---

## ⚙️ Configuration

All server settings can be customized in `src/include/config.h`:

```c
/* Server Port */
#define SERVER_PORT 8080

/* Storage Configuration */
#define STORAGE_SIZE 100              // Max items in storage
#define USER_DATA_SIZE 256            // Max data per item

/* Buffer Sizes */
#define HTTP_BUFFER_SIZE 2048         // Request header buffer
#define HTTP_RESPONSE_SIZE 16384      // Response buffer
#define POST_DATA_BUFFER_SIZE 2048    // POST body buffer
#define RESPONSE_BODY_SIZE 8192       // Response body buffer
#define BODY_LINE_SIZE 512            // Single line in response
#define FILE_PATH_SIZE 256            // File path buffer
#define TIME_STR_SIZE 100             // Timestamp buffer
```

After modifying config, recompile:

```bash
make clean && make
```

---

## 🧪 API Endpoints

### ➕ Create

```bash
curl -X POST -d "hello" http://localhost:8080/submit
```

Response (201 Created):

```json
{
  "status": 201,
  "data": {
    "id": 1,
    "message": "Data stored successfully"
  }
}
```

---

### 📥 Read All

```bash
curl http://localhost:8080/data
```

Response (200 OK):

```json
{
  "status": 200,
  "data": [
    {"id": 1, "data": "hello"},
    {"id": 2, "data": "world"}
  ]
}
```

---

### 🔍 Read by ID

```bash
curl http://localhost:8080/data?id=1
```

Response (200 OK):

```json
{
  "status": 200,
  "data": {
    "id": 1,
    "data": "hello"
  }
}
```

---

### ✏️ Update

```bash
curl -X PUT -d "updated" http://localhost:8080/data?id=1
```

Response (200 OK):

```json
{
  "status": 200,
  "data": {
    "id": 1,
    "message": "Item updated successfully"
  }
}
```

---

### ❌ Delete

```bash
curl -X DELETE http://localhost:8080/data?id=1
```

Response (200 OK):

```json
{
  "status": 200,
  "message": "Item deleted successfully"
}
```

---

## 🌐 Frontend Tester

A simple frontend is included in `public/` to test all API endpoints directly in the browser.

Open:

```
http://localhost:8080/
```

---
<img width="1843" height="909" alt="image" src="https://github.com/user-attachments/assets/364dc61e-b6e6-45db-92d4-d5a377d99ed3" />

---

## ⚙️ Technical Highlights

### 🔹 Modular Architecture

* `server.c` - Socket and threading layer
* `http.c` - HTTP parsing and request dispatch
* `handlers.c` - Business logic (GET/POST/PUT/DELETE)
* `storage.c` - Data persistence layer with CRUD operations
* `utils.c` - JSON response builders and utilities

### 🔹 HTTP Parsing

* Manual parsing of request line and headers
* Handles `Content-Length` for POST/PUT bodies
* Supports persistent connections (`keep-alive`)

### 🔹 TCP Handling

* Reads data until full request is received (`\r\n\r\n`)
* Handles partial TCP packets correctly
* Buffers configurable in `config.h`

### 🔹 Concurrency Model

* Thread-per-request using `pthread`
* Shared data protected via mutex
* Lock held only during critical sections

### 🔹 JSON API

* All API endpoints return valid JSON
* Consistent response format with `status` field
* Static file serving (HTML, CSS, JS) remains raw

### 🔹 Static File Serving

* Serves files from `/public` directory
* Supports MIME types (HTML, CSS, JS, ICO)
* Binary file support for all file types

---

## 🧠 Design Decisions

* **Modular separation** - Each responsibility (socket, HTTP, handlers, storage, utils) in its own file
* **Centralized configuration** - All constants in one `config.h` file
* **JSON responses** - All API responses use JSON for consistency
* **Storage layer abstraction** - CRUD operations via dedicated functions
* **Thread-per-request model** - Simple concurrency model suitable for this use case
* **No external dependencies** - Only POSIX and standard C library

---

## ⚠️ Limitations

* Data is not persistent (resets on restart)
* No advanced routing system
* No request validation / sanitization
* No thread pool (can be added as an extension)

---

## 🛠️ Tech Stack

* **Language:** C
* **Networking:** POSIX sockets
* **Concurrency:** pthreads
* **Protocols:** HTTP/1.1

---

## 📌 Key Learnings

* Low-level networking using sockets
* HTTP protocol internals
* Handling partial TCP reads
* Multithreading and synchronization
* Building a backend system without frameworks

---

## ⭐ If you like this project

Give it a star ⭐ or fork it!

---
