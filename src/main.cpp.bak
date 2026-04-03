#include "nexusvpn/core/tun_interface.hpp"
#include <iostream>
#include <cstring>
#include <getopt.h>

using namespace nexusvpn;

void testTUNCreation() {
    std::cout << "\n=== Test 1: TUN Interface Creation ===\n";
    try {
        TUNInterface tun("test_tun0", 1500);
        std::cout << "✅ TUN interface created: " << tun.name() << "\n";
        std::cout << "   MTU: " << tun.mtu() << "\n";
        std::cout << "   Running: " << (tun.isRunning() ? "yes" : "no") << "\n";
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed: " << e.what() << "\n";
        std::cerr << "   Note: Run with sudo for TUN access\n";
    }
}

void testIPAssignment() {
    std::cout << "\n=== Test 2: IP Address Assignment ===\n";
    try {
        TUNInterface tun("test_tun1", 1500);
        tun.setIPAddress("10.0.100.1/24");
        tun.bringUp();
        std::cout << "✅ IP address set and interface up\n";
        
        // Verify with ip command
        system("ip addr show test_tun1 2>/dev/null | grep -q '10.0.100.1' && echo '   Verified: IP 10.0.100.1 assigned'");
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed: " << e.what() << "\n";
    }
}

void testPacketReadWrite() {
    std::cout << "\n=== Test 3: Packet Read/Write ===\n";
    try {
        TUNInterface tun("test_tun2", 1500);
        tun.setIPAddress("10.0.100.1/24");
        tun.bringUp();
        
        // Create a test packet (simplified IP header)
        std::vector<uint8_t> test_packet(64, 0x45);
        test_packet[0] = 0x45;  // IP version 4, header length 5
        
        tun.writePacket(test_packet);
        std::cout << "✅ Wrote " << test_packet.size() << " bytes to TUN\n";
        
        // Try reading with timeout
        auto read_packet = tun.readPacketWithTimeout(100);
        if (!read_packet.empty()) {
            std::cout << "✅ Read " << read_packet.size() << " bytes from TUN\n";
        } else {
            std::cout << "ℹ️  No packet received (expected - no traffic routed)\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed: " << e.what() << "\n";
    }
}

void testRoutes() {
    std::cout << "\n=== Test 4: Route Management ===\n";
    try {
        TUNInterface tun("test_tun3", 1500);
        tun.setIPAddress("10.0.100.1/24");
        tun.bringUp();
        
        tun.addRoute("192.168.200.0/24");
        std::cout << "✅ Route added\n";
        
        tun.removeRoute("192.168.200.0/24");
        std::cout << "✅ Route removed\n";
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed: " << e.what() << "\n";
    }
}

void printUsage() {
    std::cout << "NexusVPN - Feature 1: TUN Interface Test Suite\n\n";
    std::cout << "Usage: sudo ./nexusvpn [OPTIONS]\n";
    std::cout << "  --test-all    Run all tests\n";
    std::cout << "  --test-tun    Test TUN creation only\n";
    std::cout << "  --test-ip     Test IP assignment\n";
    std::cout << "  --test-packet Test packet read/write\n";
    std::cout << "  --test-route  Test route management\n";
    std::cout << "  --help        Show this help\n";
}

int main(int argc, char* argv[]) {
    std::cout << "\n╔══════════════════════════════════════════╗\n";
    std::cout << "║   NexusVPN - Feature 1: TUN Interface   ║\n";
    std::cout << "║   Layer 3 Virtual Network Device        ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";
    
    if (argc < 2) {
        printUsage();
        return 0;
    }
    
    std::string option = argv[1];
    
    if (option == "--test-all" || option == "--test-tun") testTUNCreation();
    if (option == "--test-all" || option == "--test-ip") testIPAssignment();
    if (option == "--test-all" || option == "--test-packet") testPacketReadWrite();
    if (option == "--test-all" || option == "--test-route") testRoutes();
    if (option == "--help") printUsage();
    
    std::cout << "\n✅ Feature 1 testing complete!\n";
    return 0;
}
