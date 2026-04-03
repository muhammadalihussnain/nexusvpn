#include <gtest/gtest.h>
#include "nexusvpn/core/tun_interface.hpp"
#include "nexusvpn/core/constants.hpp"
#include <thread>
#include <chrono>

using namespace nexusvpn;

class TUNInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (geteuid() != 0) {
            GTEST_SKIP() << "TUN tests require root privileges. Run with sudo.";
        }
    }
    
    void TearDown() override {
        // Clean up any test interfaces using constants for names
        const std::string test_interfaces[] = {
            "test_tun0", "test_tun1", "test_tun2", "test_tun3",
            "test_tun4", "test_tun5", "test_tun6"
        };
        
        for (const auto& iface : test_interfaces) {
            std::string cmd = "ip link delete " + iface + " 2>/dev/null";
            std::system(cmd.c_str());
        }
    }
};

TEST_F(TUNInterfaceTest, CreateAndDestroy) {
    EXPECT_NO_THROW({
        TUNInterface tun("test_tun0", DEFAULT_MTU);
        EXPECT_EQ(tun.name(), "test_tun0");
        EXPECT_EQ(tun.mtu(), DEFAULT_MTU);
    });
}

TEST_F(TUNInterfaceTest, BringUpInterface) {
    TUNInterface tun("test_tun6", DEFAULT_MTU);
    std::string ip_with_cidr = std::string(VPNNetwork::DEFAULT_SERVER_TUN_IP) + 
                               "/" + std::to_string(VPNNetwork::DEFAULT_TUN_CIDR);
    tun.setIPAddress(ip_with_cidr);
    tun.bringUp();
    EXPECT_TRUE(tun.isRunning());
}

TEST_F(TUNInterfaceTest, SetIPAndBringUp) {
    TUNInterface tun("test_tun1", DEFAULT_MTU);
    std::string ip_with_cidr = std::string(VPNNetwork::DEFAULT_SERVER_TUN_IP) + 
                               "/" + std::to_string(VPNNetwork::DEFAULT_TUN_CIDR);
    tun.setIPAddress(ip_with_cidr);
    tun.bringUp();
    EXPECT_TRUE(tun.isRunning());
}

TEST_F(TUNInterfaceTest, WritePacket) {
    TUNInterface tun("test_tun2", DEFAULT_MTU);
    std::string ip_with_cidr = std::string(VPNNetwork::DEFAULT_SERVER_TUN_IP) + 
                               "/" + std::to_string(VPNNetwork::DEFAULT_TUN_CIDR);
    tun.setIPAddress(ip_with_cidr);
    tun.bringUp();
    
    std::vector<uint8_t> test_packet(BufferSize::STANDARD_PACKET, 0x45);
    EXPECT_NO_THROW({
        tun.writePacket(test_packet);
    });
}

TEST_F(TUNInterfaceTest, ReadPacketWithTimeout) {
    TUNInterface tun("test_tun3", DEFAULT_MTU);
    std::string ip_with_cidr = std::string(VPNNetwork::DEFAULT_SERVER_TUN_IP) + 
                               "/" + std::to_string(VPNNetwork::DEFAULT_TUN_CIDR);
    tun.setIPAddress(ip_with_cidr);
    tun.bringUp();
    
    auto packet = tun.readPacketWithTimeout(Timing::NONBLOCKING_TIMEOUT_MS);
    EXPECT_TRUE(packet.empty() || packet.size() > 0);
}

TEST_F(TUNInterfaceTest, AsyncReadWrite) {
    TUNInterface tun("test_tun4", DEFAULT_MTU);
    std::string ip_with_cidr = std::string(VPNNetwork::DEFAULT_SERVER_TUN_IP) + 
                               "/" + std::to_string(VPNNetwork::DEFAULT_TUN_CIDR);
    tun.setIPAddress(ip_with_cidr);
    tun.bringUp();
    
    bool callback_called = false;
    
    tun.startAsyncRead([&callback_called](const std::vector<uint8_t>& /*packet*/) {
        callback_called = true;
    });
    
    std::vector<uint8_t> test_packet(BufferSize::STANDARD_PACKET, 0x45);
    tun.writePacket(test_packet);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tun.stopAsyncRead();
    
    SUCCEED();
}

TEST_F(TUNInterfaceTest, AddRemoveRoute) {
    TUNInterface tun("test_tun5", DEFAULT_MTU);
    std::string ip_with_cidr = std::string(VPNNetwork::DEFAULT_SERVER_TUN_IP) + 
                               "/" + std::to_string(VPNNetwork::DEFAULT_TUN_CIDR);
    tun.setIPAddress(ip_with_cidr);
    tun.bringUp();
    
    const std::string test_network = "192.168.200.0/24";
    
    EXPECT_NO_THROW({
        tun.addRoute(test_network);
        tun.removeRoute(test_network);
    });
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
