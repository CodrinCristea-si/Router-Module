# Infectivity Monitor

**Infectivity Monitor** is a prototype of a network project consisting of three apps that can be described as a comprehensive network ecosystem designed to isolate clients based on their infectivity status (which it can be acquired from antivirus scans or network behaviour analysis). Here is a brief description for each app:  

* **InfectivityClient**
    - Each client connected to the network created by Infectivity Monitor should have this application installed if he wants to access the Internet and communicate with other clients.
    - It uses ClamAV free antivirus software for malware detection and is the primary way to decide the infectivity status for each client. 
    - So far it works only for devices with a Linux Distribution and it has an installer for Ubuntu.

* **InfectivityRouterModule**
    - The purpose of this app is to filter the network traffic and to clone network packages and send them to InfectivityServer for analysis.
    - It has 2 major components, a kernel module that intercepts, filters and clones the network packages (using the _Netfilter_ framework); and a user space app that detects the client connectivity via DHCP Leases, it updates the clients lists of the kernel module (using the _Netlink_ framework for communication) and receives updates from InfectivityServer.
    - This app is design for OpenWRT, but it works for almost any Linux Distribution except the web interface which is especially designed for LuCI. 

*  **InfectivityServer**
    - Is an extension of InfectivityRouterModule.
    - Its goal is to analyses the network traffic and the behaviour of each client.
    - For analysis it uses a series of heuristics of certain types that interprets the scan results returned by InfectivityClient, detects abnormal behaviour like ping storm and unusual system flows.
    - It uses MariaDB for storing data, SQLAlchemy as ORM and Flask for the web interface.
