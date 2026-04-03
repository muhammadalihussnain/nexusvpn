#!/bin/bash

cd ~/Desktop/nexusvpn/build

echo "=== Quick VPN Test ==="

# Check if build exists
if [ ! -f "./nexusvpn" ]; then
    echo "❌ Build not found. Run: cd build && cmake .. && make"
    exit 1
fi

# Start server in background
echo "Starting VPN server..."
sudo ./nexusvpn --server --port 1194 --ip 10.0.0.1/24 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

sleep 2

# Start client in background
echo "Starting VPN client..."
sudo ./nexusvpn --client --host 127.0.0.1 --port 1194 --ip 10.0.0.2/24 &
CLIENT_PID=$!
echo "Client PID: $CLIENT_PID"

sleep 3

# Test ping
echo ""
echo "Pinging VPN server..."
if ping -c 3 10.0.0.1 2>/dev/null; then
    echo "✅ VPN tunnel working!"
else
    echo "❌ Ping failed"
fi

# Show TUN interfaces
echo ""
echo "TUN Interfaces:"
ip addr show | grep -E "vpn_server|vpn_client|10.0.0" || echo "No TUN interfaces found"

# Cleanup
echo ""
echo "Cleaning up..."
sudo kill $SERVER_PID $CLIENT_PID 2>/dev/null
sudo pkill nexusvpn 2>/dev/null

echo ""
echo "✅ Test complete!"
