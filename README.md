# Simple Database Application

A simple command-line database application written in C++ that allows users to store key-value pairs persistently and print all stored data.

## Features

- Store key-value pairs in memory and persist them to a file.
- Load data from a file on startup.
- Simple command-line interface for interacting with the database.

## Building

Run the compilation script:
```bash
install.bat
```

Or manually compile using MSVC:
```bash
cl /EHsc /std:c++17 main.cpp core/Database.cpp /Fo"bin/" /Fe"bin/database.exe"
```

## Running

```bash
.\dbs.exe
```

## Commands

- `<key> <value>` - Insert a new key-value pair.
- `PRINT` - Print all stored key-value pairs.
- `EXIT` - Exit the application.

## Example Usage

```bash
> name Alice
Saved: name = Alice
> age 25
Saved: age = 25
> PRINT
name : Alice
age : 25
> EXIT
```

## Data Storage

- Data is saved to `database.txt` in the working directory.
- The database is loaded from the file on startup and saved after each modification.
