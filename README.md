[![Build status](https://ci.appveyor.com/api/projects/status/y8kbm1o1i8xs138c?svg=true)](https://ci.appveyor.com/project/ali-reza-zareian/filemonitor)

# FileMonitor

FileMonitor is a Windows-based application written in C++ that provides:

- **File monitoring**  
- **Socket server communication**  
- **Support for multiple clients connecting to the server**  
- **Robocopy integration** for efficient file copying  
- **Configuration via `config.ini`**

---

## Features

### 1. File Monitor
- Monitors changes in specified folders.
- Automatically logs or synchronizes changes according to the configuration.

### 2. Socket Server
- Handles multiple client connections simultaneously.
- Clients can send/receive commands and updates to/from the server.

### 3. Robocopy Support
- Uses **Robocopy** for robust file transfer and synchronization.
- Can copy files from monitored directories to specified destinations reliably.

### 4. Configuration (`config.ini`)
- All settings are stored in a `config.ini` file located alongside the executable.
- Example options:
  ```ini
  [Server]
  MaxClients=10

  [Sync1]
  SourceFolder=C:\Source
  DestinationFolder=D:\Backup
  Monitoring=1
  UseRobocopy=True

