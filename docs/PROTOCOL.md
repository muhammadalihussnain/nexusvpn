# NexusVPN Protocol

## Overview

NexusVPN uses a custom lightweight protocol for secure communication.

---

## Packet Structure


| Header | Encrypted Payload | Authentication Tag |


---

## Workflow

1. Client encrypts data
2. Data sent over network
3. Server decrypts data
4. Response sent back

---

## Key Concepts

- Symmetric encryption for speed
- Authentication for integrity
- Packet-based communication