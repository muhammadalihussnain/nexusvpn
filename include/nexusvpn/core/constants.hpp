#pragma once

#include <cstdint>
#include <chrono>

namespace nexusvpn {

/**
 * @brief VPN Standards-Compliant Constants
 * 
 * Following:
 * - C++ Core Guidelines: Enum.1 Prefer enums over macros
 * - MISRA C++ 2008 Rule 5-0-3: No magic numbers
 * - SEI CERT DCL00-CPP: Use constexpr for constants
 * - IETF RFC standards for networking values
 */

// ============================================================================
// NETWORK LAYER CONSTANTS (RFC 894, RFC 1191, RFC 791)
// ============================================================================

/**
 * @brief Ethernet / IP MTU values (RFC 894)
 * Standard Ethernet maximum transmission unit is 1500 bytes
 */
constexpr int DEFAULT_MTU = 1500;
constexpr int MINIMUM_MTU = 576;           // RFC 1191 - IPv4 minimum MTU
constexpr int MAXIMUM_MTU = 9000;          // Jumbo frames
constexpr int IP_HEADER_MIN_SIZE = 20;     // RFC 791 - IPv4 header minimum
constexpr int UDP_HEADER_SIZE = 8;         // RFC 768
constexpr int ETHERNET_HEADER_SIZE = 14;   // RFC 894

/**
 * @brief VPN tunnel IP address ranges (RFC 1918)
 * Private IPv4 address spaces
 */
namespace RFC1918 {
    constexpr const char* CLASS_A_NETWORK = "10.0.0.0";
    constexpr int CLASS_A_CIDR = 8;
    
    constexpr const char* CLASS_B_NETWORK = "172.16.0.0";
    constexpr int CLASS_B_CIDR = 12;
    
    constexpr const char* CLASS_C_NETWORK = "192.168.0.0";
    constexpr int CLASS_C_CIDR = 16;
}

/**
 * @brief Default VPN network configuration
 */
namespace VPNNetwork {
    constexpr const char* DEFAULT_SERVER_TUN_IP = "10.0.0.1";
    constexpr const char* DEFAULT_CLIENT_TUN_IP = "10.0.0.2";
    constexpr int DEFAULT_TUN_CIDR = 24;           // 255.255.255.0 netmask
    constexpr const char* DEFAULT_TUN_NETMASK = "255.255.255.0";
}

// ============================================================================
// TRANSPORT LAYER CONSTANTS (RFC 768, RFC 793)
// ============================================================================

/**
 * @brief IANA assigned port numbers
 */
namespace IANAPorts {
    constexpr int OPENVPN_DEFAULT = 1194;          // IANA assigned
    constexpr int IPSEC_NAT_T = 4500;              // IPsec NAT traversal
    constexpr int L2TP = 1701;                     // Layer 2 Tunneling
    constexpr int PPTP = 1723;                     // Point-to-Point Tunneling
    constexpr int MIN_USER_PORT = 1024;            // Unprivileged ports start
    constexpr int MAX_USER_PORT = 65535;
}

/**
 * @brief Buffer sizes and packet limits
 */
namespace BufferSize {
    constexpr size_t STANDARD_PACKET = 1500;       // Standard Ethernet frame
    constexpr size_t JUMBO_PACKET = 9000;          // Jumbo frame
    constexpr size_t MAX_UDP_PACKET = 65507;       // RFC 768 (65535 - 8 header)
    constexpr size_t RECEIVE_BUFFER = 65536;       // 64KB standard receive buffer
    constexpr size_t SEND_BUFFER = 65536;          // 64KB standard send buffer
}

// ============================================================================
// TIMEOUT AND INTERVAL CONSTANTS (RFC 6298, RFC 7323)
// ============================================================================

/**
 * @brief Timing constants for VPN operations
 */
namespace Timing {
    // Connection timers (RFC 6298 - TCP Retransmission)
    constexpr std::chrono::seconds HANDSHAKE_TIMEOUT{10};      // 10 seconds
    constexpr std::chrono::seconds IDLE_TIMEOUT{300};          // 5 minutes
    constexpr std::chrono::seconds KEEPALIVE_INTERVAL{25};     // 25 seconds (WireGuard)
    constexpr std::chrono::seconds KEEPALIVE_TIMEOUT{90};      // 3 * interval
    
    // Reconnection timers (exponential backoff)
    constexpr std::chrono::seconds INITIAL_RECONNECT_DELAY{1};  // Start at 1 second
    constexpr std::chrono::seconds MAX_RECONNECT_DELAY{300};    // Max 5 minutes
    
    // Testing and heartbeat
    constexpr std::chrono::seconds HEARTBEAT_INTERVAL{10};      // Status every 10 seconds
    constexpr std::chrono::seconds TEST_TIMEOUT{5};             // 5 seconds for test echo
    
    // Socket timeouts
    constexpr int SELECT_TIMEOUT_MS = 1000;                     // 1 second for select()
    constexpr int NONBLOCKING_TIMEOUT_MS = 100;                 // 100ms for non-blocking reads
}

// ============================================================================
// CRYPTOGRAPHY CONSTANTS (NIST SP 800-57, RFC 5288)
// ============================================================================

/**
 * @brief Encryption standards (NIST SP 800-57)
 */
namespace Crypto {
    // AES key sizes (bits)
    constexpr int AES_128_KEY_BITS = 128;
    constexpr int AES_192_KEY_BITS = 192;
    constexpr int AES_256_KEY_BITS = 256;    // NIST recommended
    
    // AES key sizes (bytes)
    constexpr int AES_128_KEY_BYTES = 16;
    constexpr int AES_192_KEY_BYTES = 24;
    constexpr int AES_256_KEY_BYTES = 32;
    
    // GCM mode constants (RFC 5288)
    constexpr int GCM_NONCE_SIZE = 12;       // 96 bits - recommended
    constexpr int GCM_TAG_SIZE = 16;         // 128 bits - authentication tag
    
    // Key exchange (RFC 7748)
    constexpr int X25519_PUBLIC_KEY_BYTES = 32;
    constexpr int X25519_PRIVATE_KEY_BYTES = 32;
    constexpr int X25519_SHARED_SECRET_BYTES = 32;
    
    // Key lifetimes (NIST recommendation)
    constexpr std::chrono::seconds REKEY_INTERVAL{3600};     // 1 hour
    constexpr uint64_t REKEY_BYTES = 1000000000;              // 1 GB
}

// ============================================================================
// PROTOCOL CONSTANTS
// ============================================================================

/**
 * @brief VPN Protocol message types
 */
enum class MessageType : uint8_t {
    DATA = 0x01,           // Normal data packet
    HANDSHAKE = 0x02,      // Connection establishment
    KEEPALIVE = 0x03,      // Connection keep-alive
    AUTH_REQUEST = 0x04,   // Authentication request
    AUTH_RESPONSE = 0x05,  // Authentication response
    CLOSE = 0x06           // Connection close
};

/**
 * @brief Protocol versioning
 */
namespace Protocol {
    constexpr uint8_t CURRENT_VERSION = 1;
    constexpr uint8_t MIN_SUPPORTED_VERSION = 1;
    constexpr uint16_t MAX_PAYLOAD_SIZE = 1400;    // Safe for most MTUs
}

// ============================================================================
// QUALITY OF SERVICE (Linux tc, RFC 2474)
// ============================================================================

/**
 * @brief DSCP values for QoS (RFC 2474)
 */
enum class DSCPValue : uint8_t {
    DEFAULT = 0x00,
    CS1 = 0x08,      // 001000 - Lower priority
    AF21 = 0x28,     // 010100 - Assured forwarding
    AF31 = 0x30,     // 011000
    EF = 0x2E        // 101110 - Expedited forwarding for VoIP
};

// ============================================================================
// ERROR CODES
// ============================================================================

/**
 * @brief VPN-specific error codes
 */
enum class ErrorCode : int {
    SUCCESS = 0,
    TUN_CREATION_FAILED = 1001,
    TUN_READ_FAILED = 1002,
    TUN_WRITE_FAILED = 1003,
    SOCKET_CREATION_FAILED = 2001,
    SOCKET_BIND_FAILED = 2002,
    SOCKET_CONNECT_FAILED = 2003,
    AUTHENTICATION_FAILED = 3001,
    ENCRYPTION_FAILED = 4001,
    DECRYPTION_FAILED = 4002,
    TIMEOUT = 5001,
    INTERNAL_ERROR = 9999
};

} // namespace nexusvpn
