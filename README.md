# C HTTP Server

A lightweight HTTP web server built from scratch in C using POSIX sockets.

This project implements core web server functionality including request parsing, static file serving, routing, logging, and concurrency using `fork()`.

---

## Features

* Built using POSIX sockets (`socket`, `bind`, `listen`, `accept`)
* Handles HTTP GET requests
* Serves static files (HTML, CSS, JS)
* Returns proper HTTP responses (200, 404)
* Content-Type (MIME) handling
* Concurrent client handling using `fork()`
* Request logging with timestamps
* Modular code structure (multiple source files)
* Simple Makefile for building

---

## Project Structure

```
c_http_server/
│
├── src/
│   ├── main.c
│   ├── server.c
│   ├── server.h
│   ├── http.c
│   ├── http.h
│   ├── utils.c
│   ├── utils.h
│
├── public/
│   ├── index.html
│   ├── style.css
│   ├── script.js
│
├── makefile
└── README.md
```

---

## How to Run

### 1. Build the server

```bash
make
```

### 2. Run the server

```bash
./server
```

### 3. Open in browser

```
http://localhost:8080
```

---

## Example Logs

```
[2026-04-19 18:16:11] GET / 200
[2026-04-19 18:16:12] GET /style.css 200
[2026-04-19 18:16:13] GET /script.js 200
[2026-04-19 18:16:15] GET /abc 404
```

---

## How It Works

1. The server creates a TCP socket using POSIX APIs.
2. It binds to a port and listens for incoming connections.
3. For each client connection:

   * `accept()` establishes the connection
   * `fork()` creates a new process
   * The child process handles the request
4. The server:

   * Parses the HTTP request
   * Extracts method and path
   * Maps path to a file in the `public/` directory
   * Sends the correct HTTP response
5. The parent process continues accepting new clients.

---

## What I Learned

* How HTTP works at a low level
* Socket programming using POSIX APIs
* Process-based concurrency using `fork()`
* File I/O and serving static content
* Importance of Content-Type (MIME types)
* Handling multiple clients simultaneously
* Structuring C projects into modular components
* Writing build systems using Makefiles

---

## Future Improvements

* Support for POST requests
* Better HTTP parsing (headers, query params)
* Thread-based concurrency (pthreads)
* Logging to file instead of stdout
* Security improvements (path sanitization)
* Support for large file streaming
* HTTP/1.1 persistent connections (keep-alive)

---

## Notes

This project was built as a learning exercise to understand how web servers work under the hood, beyond high-level frameworks like Express or Django.

---

## License

This project is open-source and free to use.
