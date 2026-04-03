#pragma once

#include "constants.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>

namespace nexusvpn {

/**
 * @brief TUN Virtual Interface - Layer 3 IP Tunneling
 * 
 * Implements a virtual network interface that operates at IP level.
 * Following RFC 894, RFC 1191, and C++ Core Guidelines.
 */
class TUNInterface {
public:
    /**
     * @brief Create a TUN interface
     * @param name Interface name (e.g., "vpn0")
     * @param mtu Maximum Transmission Unit (default: DEFAULT_MTU from RFC 894)
     * @throws std::runtime_error if TUN device cannot be created
     */
    explicit TUNInterface(const std::string& name, 
                          int mtu = DEFAULT_MTU);
    
    ~TUNInterface();
    
    TUNInterface(const TUNInterface&) = delete;
    TUNInterface& operator=(const TUNInterface&) = delete;
    
    TUNInterface(TUNInterface&& other) noexcept;
    TUNInterface& operator=(TUNInterface&& other) noexcept;
    
    /**
     * @brief Read a raw IP packet from the TUN interface
     * @return Vector containing the IP packet bytes
     */
    std::vector<uint8_t> readPacket();
    
    /**
     * @brief Write a raw IP packet to the TUN interface
     * @param packet The IP packet to inject into network stack
     */
    void writePacket(const std::vector<uint8_t>& packet);
    
    /**
     * @brief Read packet with timeout (non-blocking)
     * @param timeout_ms Timeout in milliseconds (default: NONBLOCKING_TIMEOUT_MS)
     * @return Packet bytes (empty if timeout)
     */
    std::vector<uint8_t> readPacketWithTimeout(
        int timeout_ms = Timing::NONBLOCKING_TIMEOUT_MS);
    
    /**
     * @brief Start asynchronous packet reading
     * @param callback Function called for each packet
     */
    void startAsyncRead(std::function<void(const std::vector<uint8_t>&)> callback);
    
    /**
     * @brief Stop asynchronous reading
     */
    void stopAsyncRead();
    
    /**
     * @brief Set IP address on the interface
     * @param cidr CIDR notation (e.g., "10.0.0.1/24")
     */
    void setIPAddress(const std::string& cidr);
    
    /**
     * @brief Bring interface up
     */
    void bringUp();
    
    /**
     * @brief Bring interface down
     */
    void bringDown();
    
    /**
     * @brief Add a route through this interface
     * @param network Network in CIDR (e.g., "192.168.1.0/24")
     */
    void addRoute(const std::string& network);
    
    /**
     * @brief Remove a route
     */
    void removeRoute(const std::string& network);
    
    /**
     * @brief Get interface name
     */
    const std::string& name() const { return name_; }
    
    /**
     * @brief Get MTU
     */
    int mtu() const { return mtu_; }
    
    /**
     * @brief Check if interface is running
     */
    bool isRunning() const { return fd_ >= 0 && is_up_; }
    
    /**
     * @brief Get file descriptor for event loops
     */
    int fileDescriptor() const { return fd_; }
    
private:
    std::string name_;
    int mtu_;
    int fd_;
    bool is_up_;
    bool async_running_;
    std::unique_ptr<std::thread> async_thread_;
    
    void createTUN();
    void executeCommand(const std::string& cmd);
    void setNonBlocking();
};

} // namespace nexusvpn
