[![Build status](https://ci.appveyor.com/api/projects/status/y8kbm1o1i8xs138c?svg=true)](https://ci.appveyor.com/project/ali-reza-zareian/filemonitor)

# FileMonitor

FileMonitor is a Windows-based application that synchronizes folders by monitoring source directories and triggering actions on a central server.  

### How It Works

1. Each **Client** (`file_monitor.exe`) monitors a specific source folder for changes (create, modify, delete, rename).  
2. When a change is detected, the client sends a message with its **Client ID** to the **Server** (`socket_server.exe`).  
3. The server identifies the client by ID, looks up the corresponding source and destination folders, and triggers **Robocopy** to synchronize the source folder to the destination folder.  
4. This ensures that every change in the monitored folder is reflected in the destination folder automatically. 

### Next modification (Cold delay)
Client detects changes → Sends message to Server → Server waits X seconds for Cold delay → Runs Robocopy from SRC to DST

---

## Setup

### Prerequisites
- Windows 10/11
- Visual Studio 2022 or newer (with C++ Desktop development workload)
- CMake 3.24+ (for building the project)
- Robocopy (comes with Windows)

### Build Steps
1. Clone the repository:
   
   git clone https://github.com/your-username/FileMonitor.git

   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat

   cd FileMonitor
   .\compile_debug.bat 
