NexusVPN User Guide (Markdown)
# NexusVPN User Guide

## Overview

NexusVPN is a secure, high-performance VPN solution that creates an encrypted tunnel between your device and a remote network.

---

## Quick Start

### Starting the VPN Client
```bash
sudo nexusvpn --client --host vpn.yourcompany.com --port 1194
Connection Options
Option	Description	Example
--host	VPN server address	--host vpn.company.com
--port	Server port (default: 1194)	--port 443
--ip	Local TUN IP	--ip 10.0.0.2/24
--config	Config file path	--config ./client.yaml
Using a Configuration File

Create ~/.config/nexusvpn/client.yaml:

client:
  tun_name: vpn_client
  tun_ip: 10.0.0.2/24
  mtu: 1500

server:
  host: vpn.yourcompany.com
  port: 1194

advanced:
  killswitch: true
  dns_leak_protection: true

Then connect:

sudo nexusvpn --config ~/.config/nexusvpn/client.yaml
Verifying Connection

Check your TUN interface:

ip addr show vpn_client
ping 10.0.0.1
Disconnecting

Press Ctrl+C in the terminal, or:

sudo pkill nexusvpn
Troubleshooting
Issue	Solution
Permission denied	Run with sudo
TUN device not found	sudo modprobe tun
Connection timeout	Check server is running
Support
GitHub Issues: https://github.com/yourusername/nexusvpn/issues
Documentation: https://docs.nexusvpn.com

---
