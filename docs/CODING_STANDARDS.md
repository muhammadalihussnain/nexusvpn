# NexusVPN Coding Standards

## Standards Followed

### 1. C++ Core Guidelines
- Enum.1: Prefer enums over macros for constants
- I.13: Use constexpr for compile-time constants
- R.3: Use RAII for resource management

### 2. MISRA C++ 2008
- Rule 5-0-3: No magic numbers allowed
- Rule 5-2-1: Use constexpr for constants

### 3. SEI CERT C++
- DCL00-CPP: Use constexpr instead of define
- DCL50-CPP: Don't define macros for constants

## Constants Usage

### DO NOT use magic numbers:
```cpp
// WRONG
TUNInterface tun("tun0", 1500);
udp.bind(1194);

// CORRECT
TUNInterface tun("tun0", DEFAULT_MTU);
udp.bind(IANAPorts::OPENVPN_DEFAULT);