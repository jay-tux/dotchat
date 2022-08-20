# Dotchat
*A simple, proof-of-concept, custom-protocol chatting system*

## Features
 - [x] Secure traffic using TLS
 - [x] Custom [protocol](protocol.md) allowing arbitrary messages
 - [x] Users, channels and messages
 - [x] Channel ownership
 - [x] Database using SQLite
 - [x] Database managed using ORM
 - [x] Separate thread per connection
 - [x] Reusable connections
 - [ ] Thread manager
 - [ ] TUI for client
 - [ ] TUI for server
 - [ ] Server background workers
 - [ ] Message paging

## Dependencies
All dependencies are managed using Conan. The build system used is CMake.

### Client dependencies
 - OpenSSL 3.0.3 ([GitHub](https://github.com/openssl/openssl), [ConanCenter](https://conan.io/center/openssl))

### Server dependencies
 - OpenSSL 3.0.3 ([GitHub](https://github.com/openssl/openssl), [ConanCenter](https://conan.io/center/openssl))
 - SQLite3 3.37.2 ([GitHub](https://github.com/sqlite/sqlite), [ConanCenter](https://conan.io/center/sqlite3))
 - SQLite ORM 1.7.1 ([GitHub](https://github.com/fnc12/sqlite_orm), [ConanCenter](https://conan.io/center/sqlite_orm))

### Future dependencies
 - ncurses (version TBD)  ([Own Site](https://invisible-island.net/ncurses/), [ConanCenter](https://conan.io/center/ncurses))