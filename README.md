# Marketplace MVP Server (C++)

A minimal viable product (MVP) server for a marketplace, implemented in C++ with PostgreSQL and Docker.

## Project structure
```
project/
├── CMakeLists.txt           # Top-level build configuration (application + tests)
├── Dockerfile               # Dockerfile for the application service
├── src/                     # Application source code
│   ├── app/                 # Application source(there're main.cpp - entry point)
│   ├── include/             # Headers for app
│   ├── modules/             # Source for headers
│   ├── CMakeLists.txt       # Application build config
│   └── ...
├── tests/                   # Test suite
│   ├── CMakeLists.txt       # Test build config
│   └── ...
└── postgres/                # PostgreSQL service files
    └── ...
```

## Requirements
- Docker

#### Application service(these components are installed in Dockerfile)
- C++23 or newer
- CMake 3.10+
- Boost(Asio, Beast, uuid)
- OpenSSL(need for Boost.Asio)
- libpqxx ≥ 7.0
- libbcrypt
- nlohmann/json
- spdlog
- fmt
- jwt-cpp

#### Postgres service
- PostgreSQL 16.x  
- Database `sBeHappy` with table `users`:
```SQL
CREATE TABLE users (
    id SERIAL PRIMARY KEY, 
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    username VARCHAR(100) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(100) NOT NULL
);
```
- and table `user_logs`:
``` SQL
CREATE TABLE user_logs (
     id SERIAL PRIMARY KEY,
     user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
     created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
     log_event TEXT NOT NULL
 );
```
## Build and Run
Clone the repository, build the Docker containers, and start the services with:

```bash
git clone https://github.com/grechaffff/sBeHappy.git
cd sBeHappy
docker-compose build
docker-compose up
```
This will start both the PostgreSQL database and the C++ application server. The application will automatically connect to the database once it is ready.

## Additional Information
- The application uses `Boost.Asio` for asynchronous networking.
- Database connections are handled using `libpqxx`.
- Passwords are hashed with `libbcrypt` for security.
- `nlohmann/json` is used to exchange JSON data between the client and the server.
