# Infectivity Monitor

**Infectivity Monitor** is a prototype of a network project consisting of three primary components(applications) that can be described as a comprehensive network ecosystem designed to isolate clients based on their infectivity status (which it can be acquired from antivirus scans or network behaviour analysis). Here is a brief description for each app:  

* **InfectivityClient**
    - Each client connected to the network created by Infectivity Monitor should have this application installed if it wants to access the Internet and communicate with other clients.
    - It uses ClamAV free antivirus software for malware detection and is the primary way to decide the infectivity status for each client. 
    - So far it works only for devices operating on a Linux Distribution with a specific installer for ``Ubuntu Desktop``.
    - Besides the ``ClamAV`` CLI and Signiture Database, other technologies used are ``Python 3.11.0`` and ``Linux Shell``.
  
* **InfectivityRouterModule**
    - The purpose of this router module is to filter the network traffic, to clone network packages and send them to InfectivityServer for analysis.
    - It has 2 major components, a Linux kernel module that intercepts, filters and clones the network packages (using the _Netfilter_ framework), a userspace application that detects the client connectivity via DHCP Leases, it updates the clients register of the kernel module (using the _Netlink_ framework for communication) and receives updates from InfectivityServer.
    - This application is design for ``OpenWRT`` systems, but it works on almost any Linux Distribution except the web interface which is designed for ``LuCI``. 
    - Technologies used:
        - for the Linux Kernel Module: ``C`` (more precisely ``C11``) programming language and frameworks such as ``Netfilter`` (package filtering), ``Netlink`` (kernelspace and userspace communication).
        - for userspace application: programming languages(``C`` or ``C11``), client-side (``HTML``, ``CSS``, ``JAVASCRIPT``), server-side (``Lua``, ``C``), and ``Linux Shell`` scripts.
    
*  **InfectivityServer**
    - Is an extension of InfectivityRouterModule.
    - Its goal is to analyses the network traffic and the behaviour of each client.
    - For analysis it uses a series of heuristics of certain types that interprets the scan results returned by InfectivityClient, detects abnormal behaviour such as ping storm and other unusual system flows.
    - Technologies used: ``Python 3.11.0`` (the same one as InfectivityClient for network communication), ``MariaDB`` (Database), ``SQLAlchemy`` (ORM), ``Flask`` (Web framework) with ``Jinja2`` for server-side, and ``HTML``, ``CSS``, ``JAVASCRIPT`` for client-size.
   
> [!NOTE]
> The [pictures](https://github.com/CodrinCristea-si/Router-Module/tree/main/pictures) folder contains screenshots of the applications user interface.  
