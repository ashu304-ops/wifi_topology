# WiFiScope — Wi-Fi Network Discovery & Topology Mapper

WiFiScope is a **C++17 Linux-based network discovery and topology mapping tool** that discovers devices on a local Wi-Fi/LAN network, identifies their network information, scans common TCP ports, builds a logical network topology, and generates a visual network map using Graphviz.

The project is designed to demonstrate practical **C++17, Linux networking, raw sockets, ARP, TCP sockets, concurrency, CMake, and graph-based topology construction**.

---

## Features

* Detect active network interface
* Detect local IP address
* Detect subnet mask
* Calculate network address
* Detect default gateway
* Detect local MAC address
* Discover local network devices using **ARP**
* Resolve hostnames using reverse DNS
* Identify gateway and local machine
* Scan common TCP ports
* Perform bounded concurrent port scanning
* Build a logical network topology
* Export topology to Graphviz `.dot`
* Generate a visual network topology as PNG/SVG

---

## Architecture

```text
                    WiFiScope
                        |
                        v
              Network Interface
                        |
        +---------------+---------------+
        |                               |
        v                               v
   ARP Discovery                  Network Information
        |                               |
        +---------------+---------------+
                        |
                        v
                Host Discovery
                        |
                        v
                 Device Objects
                        |
                        v
                  Port Scanner
                        |
                        v
                Topology Builder
                        |
                        v
              Topology Exporter
                        |
                        v
                  Graphviz DOT
                        |
                        v
                Network Diagram
```

---

## Project Structure

```text
wifi-topology-mapper/
│
├── CMakeLists.txt
├── README.md
│
├── docs/
│
├── src/
│   ├── main.cpp
│   │
│   ├── models/
│   │   └── Device.h
│   │
│   ├── network/
│   │   ├── NetworkInterface.cpp
│   │   ├── NetworkInterface.h
│   │   ├── ARPScanner.cpp
│   │   └── ARPScanner.h
│   │
│   ├── discovery/
│   │   ├── HostDiscovery.cpp
│   │   └── HostDiscovery.h
│   │
│   ├── scanner/
│   │   ├── PortScanner.cpp
│   │   └── PortScanner.h
│   │
│   └── topology/
│       ├── TopologyBuilder.cpp
│       ├── TopologyBuilder.h
│       ├── TopologyExporter.cpp
│       └── TopologyExporter.h
│
└── tests/
```

---

# Technologies Used

| Technology              | Purpose                  |
| ----------------------- | ------------------------ |
| C++17                   | Core implementation      |
| Linux                   | Target operating system  |
| POSIX Sockets           | Network communication    |
| Raw `AF_PACKET` sockets | ARP packet handling      |
| ARP                     | Local device discovery   |
| TCP sockets             | Port scanning            |
| `std::async`            | Concurrent port scanning |
| Mutex                   | Synchronization          |
| CMake                   | Build system             |
| Graphviz                | Network visualization    |

---

# How It Works

## 1. Network Interface Detection

WiFiScope first identifies the active network configuration.

Example:

```text
Interface       : wlp3s0
IP Address      : 192.168.0.108
Subnet Mask     : 255.255.255.0
Network Address : 192.168.0.0
Gateway         : 192.168.0.1
MAC Address     : 4c:bb:58:de:19:1a
```

This information is used to determine the local network range.

---

## 2. ARP Device Discovery

WiFiScope uses raw Ethernet sockets and constructs ARP request packets.

Conceptually:

```text
WiFiScope
   |
   | ARP Request
   | "Who has 192.168.0.x?"
   |
   v
Local Network
   |
   +---- Device
   |
   +---- Device
   |
   +---- Router
```

Devices that respond provide their MAC address and IP address.

The implementation uses Linux:

```cpp
socket(
    AF_PACKET,
    SOCK_RAW,
    htons(ETH_P_ARP)
);
```

Because raw packet sockets require elevated privileges, WiFiScope should normally be executed with `sudo`.

---

## 3. Hostname Resolution

For discovered IP addresses, WiFiScope attempts reverse hostname resolution.

Example:

```text
192.168.0.108 → archlinux
192.168.0.1   → _gateway
```

If no hostname can be resolved:

```text
Hostname : Unknown
```

---

## 4. TCP Port Scanning

WiFiScope scans a configurable list of common TCP ports.

Current default ports:

```text
22
23
53
80
110
139
443
445
3306
5432
8080
```

Example:

```text
Scanning ports on 192.168.0.108...

[OPEN] 80
[OPEN] 443
```

The scanner performs bounded concurrent scanning using C++ asynchronous tasks.

Conceptually:

```text
             Port Scanner
                  |
        +---------+---------+
        |         |         |
       22        80        443
        |         |         |
      scan      scan      scan
```

The number of concurrent tasks is intentionally bounded rather than creating an unlimited number of threads.

---

# 5. Device Model

Each discovered device is represented by a `Device` object.

```cpp
struct Device {
    std::string ip;
    std::string mac;
    std::string hostname;

    bool isGateway = false;
    bool isLocalMachine = false;

    std::vector<PortInfo> ports;
};
```

This allows information from different discovery stages to be combined into one object.

---

# 6. Topology Construction

The discovered devices are converted into a graph consisting of:

```text
TopologyNode
TopologyEdge
```

Example:

```text
             Gateway
           192.168.0.1
                 |
                 |
                 v
          Local Machine
          192.168.0.108
```

The topology represents a **logical network relationship inferred from the discovered devices**.

It does not claim to determine the exact physical Wi-Fi access-point, switch, or routing path.

---

# 7. Graphviz Visualization

WiFiScope exports the topology into a Graphviz DOT file:

```text
network.dot
```

Graphviz can then generate an image:

```bash
dot -Tpng network.dot -o network.png
```

or SVG:

```bash
dot -Tsvg network.dot -o network.svg
```

Example:

```text
                 INTERNET
                     |
                     |
              +------+------+
              |   ROUTER    |
              | 192.168.0.1 |
              +------+------+
                     |
                     |
              +------+------+
              | LOCAL PC    |
              | 192.168.0.108|
              +-------------+
```

---

# Requirements

## Operating System

Currently developed and tested on:

```text
Linux
```

The project uses Linux-specific raw networking APIs, particularly:

```text
AF_PACKET
ETH_P_ARP
```

Therefore, it is not currently designed as a portable Windows application.

---

## Dependencies

Install:

### C++ compiler

```bash
sudo pacman -S gcc
```

### CMake

```bash
sudo pacman -S cmake
```

### Graphviz

```bash
sudo pacman -S graphviz
```

Verify:

```bash
g++ --version
cmake --version
dot -V
```

---

# Build

Clone the repository:

```bash
git clone <YOUR_GITHUB_REPOSITORY>
```

Enter the project:

```bash
cd wifi-topology-mapper
```

Create the build directory:

```bash
mkdir build
cd build
```

Configure:

```bash
cmake ..
```

Build:

```bash
cmake --build . -j$(nproc)
```

---

# Run

Because ARP discovery uses raw sockets, run with elevated privileges:

```bash
sudo ./wifiscope
```

Example:

```text
====================================
       WiFiScope Network Mapper
====================================

Interface       : wlp3s0
IP Address      : 192.168.0.108
Subnet Mask     : 255.255.255.0
Network Address : 192.168.0.0
Gateway         : 192.168.0.1
MAC Address     : 4c:bb:58:de:19:1a

====================================
          PORT SCANNING
====================================

Scanning ports on 192.168.0.108...
  [OPEN] 80
  [OPEN] 443

Scanning ports on 192.168.0.1...
  [OPEN] 22
  [OPEN] 80

====================================
          DEVICE DISCOVERY
====================================

[LOCAL MACHINE]
IP       : 192.168.0.108
MAC      : 4c:bb:58:de:19:1a
Hostname : archlinux
Open Ports: 80 443

[ROUTER / GATEWAY]
IP       : 192.168.0.1
MAC      : 70:4f:57:59:6c:7c
Hostname : _gateway
Open Ports: 22 80

------------------------------------
Devices discovered: 2
------------------------------------

Topology exported to: network.dot
```

Generate the visualization:

```bash
dot -Tpng network.dot -o network.png
```

Open it:

```bash
xdg-open network.png
```

---

# Security and Usage

WiFiScope is intended for **authorized networks only**.

Use it on:

* Your own home network
* Your own lab environment
* Networks where you have explicit permission to perform discovery/scanning

Do not use network scanning against systems or networks without authorization.

---

# Limitations

### Device discovery

ARP discovery only identifies devices that are reachable and respond to ARP on the local network.

Some devices may not appear because of:

* Wi-Fi client isolation
* Firewall configuration
* Network segmentation
* Sleeping devices
* VPNs
* Router configuration

Therefore:

> The number of devices discovered by WiFiScope should not automatically be interpreted as the exact number of devices connected to the Wi-Fi.

---

### Topology

The current topology is a **logical topology inference**.

It does not determine:

* Exact physical Wi-Fi access point
* Exact switch path
* Physical cable connections
* Wireless signal strength
* Full routing topology

---

# Future Improvements

Possible future versions could include:

* Real thread-pool implementation
* Non-blocking TCP connections using `poll()`/`epoll()`
* Better device classification
* Wi-Fi access-point detection
* Signal-strength information
* Service identification
* Configurable port ranges
* JSON export
* Interactive terminal UI
* Real-time device monitoring
* Topology refresh
* IPv6 support
* Unit tests
* Performance measurements

---

# Learning Objectives

This project was built to gain practical experience with:

```text
C++17
   ↓
Linux APIs
   ↓
Raw sockets
   ↓
ARP
   ↓
TCP
   ↓
Concurrency
   ↓
Synchronization
   ↓
Network discovery
   ↓
Graph construction
   ↓
Graphviz
```

---

# Resume Description

### WiFiScope — Wi-Fi Network Discovery & Topology Mapper

Developed a **C++17 Linux networking tool** for local network discovery and topology visualization. Implemented raw `AF_PACKET` ARP packet handling to discover LAN devices, hostname resolution, bounded concurrent TCP port scanning, device modeling, logical topology construction, and Graphviz-based network visualization using CMake.

### Key Technologies

```text
C++17 | Linux | Raw Sockets | ARP | TCP
Multithreading | STL | CMake | Graphviz
```

---

# Project Status

**Status: Completed — Version 1.0**

Core functionality implemented:

* [x] Network interface detection
* [x] IP/subnet/gateway detection
* [x] Raw ARP discovery
* [x] Hostname resolution
* [x] Device modeling
* [x] Concurrent TCP port scanning
* [x] Logical topology construction
* [x] Graphviz DOT export
* [x] PNG topology visualization

---

## Author

**Ashish Babu**

C++ / Linux / Embedded Software Enthusiast
