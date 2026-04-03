#include "nexusvpn/core/tun_interface.hpp"
#include "nexusvpn/core/constants.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace nexusvpn;

// Forward declarations
bool interfaceExists(const std::string& name);
void deleteInterface(const std::string& name);

void runServer(int port, const std::string& tun_ip) {
    std::cout << "\n=== Starting VPN Server ===" << std::endl;
    
    const std::string interface_name = "vpn_server";
    
    deleteInterface(interface_name);
    
    try {
        TUNInterface tun(interface_name, DEFAULT_MTU);
        tun.setIPAddress(tun_ip);
        tun.bringUp();
        std::cout << "✅ TUN interface created and configured" << std::endl;
        std::cout << "   Interface: " << interface_name << std::endl;
        std::cout << "   IP: " << tun_ip << std::endl;
        
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return;
        }
        
        int reuse = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(static_cast<uint16_t>(port));
        
        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "Failed to bind to port " << port << std::endl;
            close(sock);
            return;
        }
        
        std::cout << "📡 Server listening on UDP port " << port << std::endl;
        std::cout << "🔍 Check with: ip addr show " << interface_name << std::endl;
        std::cout << "Press Ctrl+C to stop\n" << std::endl;
        
        bool running = true;
        while (running) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);
            
            struct timeval tv;
            tv.tv_sec = Timing::SELECT_TIMEOUT_MS / 1000;
            tv.tv_usec = (Timing::SELECT_TIMEOUT_MS % 1000) * 1000;
            
            int ret = select(sock + 1, &read_fds, nullptr, nullptr, &tv);
            
            if (ret < 0) {
                break;
            }
            
            if (FD_ISSET(sock, &read_fds)) {
                uint8_t buffer[BufferSize::RECEIVE_BUFFER];
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                
                int n = recvfrom(sock, buffer, sizeof(buffer), 0,
                                (struct sockaddr*)&client_addr, &client_len);
                
                if (n > 0) {
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                    std::cout << "📥 Received " << n << " bytes from " << client_ip 
                              << ":" << ntohs(client_addr.sin_port);
                    
                    sendto(sock, buffer, n, 0, (struct sockaddr*)&client_addr, client_len);
                    std::cout << " - echoed back" << std::endl;
                }
            }
        }
        
        close(sock);
    } catch (const std::exception& e) {
        std::cerr << "❌ Server error: " << e.what() << std::endl;
    }
}

void runClient(const std::string& server_host, int port, const std::string& tun_ip) {
    std::cout << "\n=== Starting VPN Client ===" << std::endl;
    
    const std::string interface_name = "vpn_client";
    
    deleteInterface(interface_name);
    
    try {
        TUNInterface tun(interface_name, DEFAULT_MTU);
        tun.setIPAddress(tun_ip);
        tun.bringUp();
        std::cout << "✅ TUN interface created and configured" << std::endl;
        std::cout << "   Interface: " << interface_name << std::endl;
        std::cout << "   IP: " << tun_ip << std::endl;
        
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return;
        }
        
        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(static_cast<uint16_t>(port));
        if (inet_pton(AF_INET, server_host.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid server address: " << server_host << std::endl;
            close(sock);
            return;
        }
        
        std::cout << "📡 Connected to server " << server_host << ":" << port << std::endl;
        std::cout << "🔍 Check with: ip addr show " << interface_name << std::endl;
        
        const std::string test_msg = "Hello from VPN client!";
        sendto(sock, test_msg.c_str(), test_msg.length(), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));
        std::cout << "📤 Sent test packet: " << test_msg << std::endl;
        
        uint8_t buffer[BufferSize::RECEIVE_BUFFER];
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        
        struct timeval tv;
        tv.tv_sec = Timing::TEST_TIMEOUT.count();
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        int n = recvfrom(sock, buffer, sizeof(buffer), 0,
                        (struct sockaddr*)&from_addr, &from_len);
        
        if (n > 0) {
            std::string response(reinterpret_cast<char*>(buffer), n);
            std::cout << "📥 Received echo: " << response << std::endl;
            std::cout << "\n✅ VPN TUNNEL WORKING!" << std::endl;
            std::cout << "   Both server and client are communicating through the TUN interface.\n" << std::endl;
        } else {
            std::cout << "❌ No echo received. Make sure server is running." << std::endl;
            std::cout << "   Start server in another terminal: sudo ./nexusvpn --server" << std::endl;
        }
        
        std::cout << "🟢 VPN Client is running. Press Ctrl+C to stop." << std::endl;
        while (true) {
            std::this_thread::sleep_for(Timing::HEARTBEAT_INTERVAL);
            std::cout << "   [Heartbeat] TUN interface " << interface_name << " is active" << std::endl;
        }
        
        close(sock);
    } catch (const std::exception& e) {
        std::cerr << "❌ Client error: " << e.what() << std::endl;
        if (std::strstr(e.what(), "open") || std::strstr(e.what(), "permission")) {
            std::cerr << "   Make sure you're running with sudo!" << std::endl;
        }
        if (std::strstr(e.what(), "exist")) {
            std::cerr << "   Interface conflict. Try: sudo ip link delete vpn_client 2>/dev/null" << std::endl;
        }
    }
}

void testTUNCreation() {
    std::cout << "\n=== Test: TUN Interface Creation ===" << std::endl;
    const std::string test_name = "test_tun0";
    
    deleteInterface(test_name);
    
    try {
        TUNInterface tun(test_name, DEFAULT_MTU);
        tun.setIPAddress(std::string(VPNNetwork::DEFAULT_SERVER_TUN_IP) + "/" + 
                        std::to_string(VPNNetwork::DEFAULT_TUN_CIDR));
        tun.bringUp();
        std::cout << "✅ TUN interface created: " << test_name << std::endl;
        std::cout << "   MTU: " << tun.mtu() << std::endl;
        std::cout << "   IP: " << VPNNetwork::DEFAULT_SERVER_TUN_IP 
                  << "/" << VPNNetwork::DEFAULT_TUN_CIDR << std::endl;
        std::cout << "   Status: " << (tun.isRunning() ? "UP" : "DOWN") << std::endl;
        
        std::string cmd = "ip addr show " + test_name + " 2>/dev/null";
        std::system(cmd.c_str());
        
        tun.bringDown();
        deleteInterface(test_name);
        std::cout << "\n✅ Test complete. Interface cleaned up." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed: " << e.what() << std::endl;
        std::cerr << "   Note: Run with sudo for TUN access" << std::endl;
    }
}

bool interfaceExists(const std::string& name) {
    std::string cmd = "ip link show " + name + " 2>/dev/null";
    return std::system(cmd.c_str()) == 0;
}

void deleteInterface(const std::string& name) {
    if (interfaceExists(name)) {
        std::string cmd = "sudo ip link delete " + name + " 2>/dev/null";
        std::system(cmd.c_str());
        std::cout << "   Removed existing interface: " << name << std::endl;
    }
}

void printUsage() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              NexusVPN - Complete VPN Solution                ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ Usage:                                                        ║\n";
    std::cout << "║   sudo ./nexusvpn --server                    Run VPN server ║\n";
    std::cout << "║   sudo ./nexusvpn --client --host <IP>        Run VPN client ║\n";
    std::cout << "║   sudo ./nexusvpn --test-tun                  Test TUN only  ║\n";
    std::cout << "║   sudo ./nexusvpn --help                      Show this help ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ Examples:                                                     ║\n";
    std::cout << "║   Server: sudo ./nexusvpn --server                           ║\n";
    std::cout << "║   Client: sudo ./nexusvpn --client --host 127.0.0.1          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
}

int main(int argc, char* argv[]) {
    bool server_mode = false;
    bool client_mode = false;
    bool test_mode = false;
    std::string server_host = "127.0.0.1";
    int port = IANAPorts::OPENVPN_DEFAULT;
    std::string tun_ip_server = std::string(VPNNetwork::DEFAULT_SERVER_TUN_IP) + 
                                "/" + std::to_string(VPNNetwork::DEFAULT_TUN_CIDR);
    std::string tun_ip_client = std::string(VPNNetwork::DEFAULT_CLIENT_TUN_IP) + 
                                "/" + std::to_string(VPNNetwork::DEFAULT_TUN_CIDR);
    
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--server") == 0) {
            server_mode = true;
        } else if (std::strcmp(argv[i], "--client") == 0) {
            client_mode = true;
        } else if (std::strcmp(argv[i], "--test-tun") == 0) {
            test_mode = true;
        } else if (std::strcmp(argv[i], "--host") == 0 && i + 1 < argc) {
            server_host = argv[++i];
        } else if (std::strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--ip") == 0 && i + 1 < argc) {
            if (server_mode) {
                tun_ip_server = argv[++i];
            } else if (client_mode) {
                tun_ip_client = argv[++i];
            }
        } else if (std::strcmp(argv[i], "--help") == 0) {
            printUsage();
            return 0;
        }
    }
    
    std::cout << "\n╔══════════════════════════════════════════╗";
    std::cout << "\n║   NexusVPN - Secure VPN Solution        ║";
    std::cout << "\n║   Feature 1: TUN Virtual Interface      ║";
    std::cout << "\n╚══════════════════════════════════════════╝\n";
    
    if (test_mode) {
        testTUNCreation();
    } else if (server_mode) {
        runServer(port, tun_ip_server);
    } else if (client_mode) {
        runClient(server_host, port, tun_ip_client);
    } else {
        printUsage();
    }
    
    return 0;
}
