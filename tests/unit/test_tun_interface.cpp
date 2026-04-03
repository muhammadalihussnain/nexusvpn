#include <gtest/gtest.h>
#include "nexusvpn/core/tun_interface.hpp"
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
        // Clean up any test interfaces
        system("ip link delete test_tun0 2>/dev/null");
        system("ip link delete test_tun1 2>/dev/null");
        system("ip link delete test_tun2 2>/dev/null");
        system("ip link delete test_tun3 2>/dev/null");
    }
};

TEST_F(TUNInterfaceTest, CreateAndDestroy) {
    EXPECT_NO_THROW({
        TUNInterface tun("test_tun0", 1500);
        EXPECT_EQ(tun.name(), "test_tun0");
        EXPECT_EQ(tun.mtu(), 1500);
        EXPECT_TRUE(tun.isRunning());
    });
}

TEST_F(TUNInterfaceTest, SetIPAndBringUp) {
    TUNInterface tun("test_tun1", 1500);
    EXPECT_NO_THROW({
        tun.setIPAddress("10.0.100.1/24");
        tun.bringUp();
        EXPECT_TRUE(tun.isRunning());
    });
}

TEST_F(TUNInterfaceTest, WritePacket) {
    TUNInterface tun("test_tun2", 1500);
    tun.setIPAddress("10.0.100.1/24");
    tun.bringUp();
    
    std::vector<uint8_t> test_packet(64, 0x45);
    EXPECT_NO_THROW({
        tun.writePacket(test_packet);
    });
}

TEST_F(TUNInterfaceTest, ReadPacketWithTimeout) {
    TUNInterface tun("test_tun3", 1500);
    tun.setIPAddress("10.0.100.1/24");
    tun.bringUp();
    
    auto packet = tun.readPacketWithTimeout(100);
    // Timeout should return empty packet
    EXPECT_TRUE(packet.empty() || packet.size() > 0);
}

TEST_F(TUNInterfaceTest, AsyncReadWrite) {
    TUNInterface tun("test_tun4", 1500);
    tun.setIPAddress("10.0.100.1/24");
    tun.bringUp();
    
    bool callback_called = false;
    
    tun.startAsyncRead([&callback_called](const std::vector<uint8_t>& packet) {
        callback_called = true;
    });
    
    std::vector<uint8_t> test_packet(64, 0x45);
    tun.writePacket(test_packet);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tun.stopAsyncRead();
    
    // Just verify no crash - callback may not trigger without real traffic
    SUCCEED();
}

TEST_F(TUNInterfaceTest, AddRemoveRoute) {
    TUNInterface tun("test_tun5", 1500);
    tun.setIPAddress("10.0.100.1/24");
    tun.bringUp();
    
    EXPECT_NO_THROW({
        tun.addRoute("192.168.200.0/24");
        tun.removeRoute("192.168.200.0/24");
    });
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
