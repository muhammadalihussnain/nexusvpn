#include "nexusvpn/core/tun_interface.hpp"
#include "nexusvpn/core/constants.hpp"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>

namespace nexusvpn {

TUNInterface::TUNInterface(const std::string& name, int mtu)
    : name_(name)
    , mtu_(mtu)
    , fd_(-1)
    , is_up_(false)
    , async_running_(false) {
    createTUN();
}

TUNInterface::~TUNInterface() {
    stopAsyncRead();
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

TUNInterface::TUNInterface(TUNInterface&& other) noexcept
    : name_(std::move(other.name_))
    , mtu_(other.mtu_)
    , fd_(other.fd_)
    , is_up_(other.is_up_)
    , async_running_(false) {
    other.fd_ = -1;
    other.is_up_ = false;
}

TUNInterface& TUNInterface::operator=(TUNInterface&& other) noexcept {
    if (this != &other) {
        if (fd_ >= 0) close(fd_);
        name_ = std::move(other.name_);
        mtu_ = other.mtu_;
        fd_ = other.fd_;
        is_up_ = other.is_up_;
        other.fd_ = -1;
        other.is_up_ = false;
    }
    return *this;
}

void TUNInterface::createTUN() {
    const char* tun_device = "/dev/net/tun";
    fd_ = open(tun_device, O_RDWR);
    if (fd_ < 0) {
        throw std::runtime_error("Failed to open /dev/net/tun. Run with sudo?");
    }
    
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(fd_, TUNSETIFF, &ifr) < 0) {
        close(fd_);
        fd_ = -1;
        throw std::runtime_error("Failed to create TUN interface: " + name_);
    }
    
    std::cout << "[TUN] Created interface: " << name_ 
              << " (fd=" << fd_ << ", mtu=" << mtu_ << ")" << std::endl;
}

std::vector<uint8_t> TUNInterface::readPacket() {
    if (fd_ < 0) {
        throw std::runtime_error("TUN interface not open");
    }
    
    // Use buffer size from constants (MTU + IP header + UDP header + margin)
    const size_t buffer_size = static_cast<size_t>(mtu_) + 
                               IP_HEADER_MIN_SIZE + 
                               UDP_HEADER_SIZE + 
                               ETHERNET_HEADER_SIZE;
    
    std::vector<uint8_t> buffer(buffer_size);
    ssize_t n = read(fd_, buffer.data(), buffer.size());
    
    if (n < 0) {
        throw std::runtime_error("Failed to read from TUN: " + 
                                 std::string(std::strerror(errno)));
    }
    
    buffer.resize(static_cast<size_t>(n));
    return buffer;
}

std::vector<uint8_t> TUNInterface::readPacketWithTimeout(int timeout_ms) {
    if (fd_ < 0) {
        throw std::runtime_error("TUN interface not open");
    }
    
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd_, &read_fds);
    
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    
    int ret = select(fd_ + 1, &read_fds, nullptr, nullptr, &timeout);
    
    if (ret < 0) {
        throw std::runtime_error("select() failed: " + std::string(std::strerror(errno)));
    }
    
    if (ret == 0) {
        return {};  // Timeout - return empty packet
    }
    
    return readPacket();
}

void TUNInterface::writePacket(const std::vector<uint8_t>& packet) {
    if (fd_ < 0) {
        throw std::runtime_error("TUN interface not open");
    }
    
    if (packet.empty()) {
        return;
    }
    
    ssize_t n = write(fd_, packet.data(), packet.size());
    
    if (n < 0) {
        throw std::runtime_error("Failed to write to TUN: " + 
                                 std::string(std::strerror(errno)));
    }
    
    if (static_cast<size_t>(n) != packet.size()) {
        throw std::runtime_error("Partial write to TUN: wrote " + 
                                 std::to_string(n) + " of " + std::to_string(packet.size()));
    }
}

void TUNInterface::startAsyncRead(std::function<void(const std::vector<uint8_t>&)> callback) {
    if (async_running_) return;
    
    async_running_ = true;
    async_thread_ = std::make_unique<std::thread>([this, callback]() {
        while (async_running_) {
            try {
                auto packet = readPacket();
                if (callback && !packet.empty()) {
                    callback(packet);
                }
            } catch (const std::exception& e) {
                std::cerr << "[TUN] Async read error: " << e.what() << std::endl;
                break;
            }
        }
    });
}

void TUNInterface::stopAsyncRead() {
    async_running_ = false;
    if (async_thread_ && async_thread_->joinable()) {
        async_thread_->join();
    }
    async_thread_.reset();
}

void TUNInterface::setIPAddress(const std::string& cidr) {
    std::string cmd = "ip addr add " + cidr + " dev " + name_;
    executeCommand(cmd);
    std::cout << "[TUN] Set IP " << cidr << " on " << name_ << std::endl;
}

void TUNInterface::bringUp() {
    std::string cmd = "ip link set dev " + name_ + " up";
    executeCommand(cmd);
    is_up_ = true;
    std::cout << "[TUN] Interface " << name_ << " is UP" << std::endl;
}

void TUNInterface::bringDown() {
    std::string cmd = "ip link set dev " + name_ + " down";
    executeCommand(cmd);
    is_up_ = false;
    std::cout << "[TUN] Interface " << name_ << " is DOWN" << std::endl;
}

void TUNInterface::addRoute(const std::string& network) {
    std::string cmd = "ip route add " + network + " dev " + name_;
    executeCommand(cmd);
    std::cout << "[TUN] Added route: " << network << " via " << name_ << std::endl;
}

void TUNInterface::removeRoute(const std::string& network) {
    std::string cmd = "ip route del " + network + " dev " + name_;
    executeCommand(cmd);
    std::cout << "[TUN] Removed route: " << network << std::endl;
}

void TUNInterface::executeCommand(const std::string& cmd) {
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "[TUN] Warning: Command failed (code " << result << "): " << cmd << std::endl;
    }
}

void TUNInterface::setNonBlocking() {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        throw std::runtime_error("Failed to get TUN fd flags");
    }
    if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::runtime_error("Failed to set TUN fd to non-blocking");
    }
}

} // namespace nexusvpn
