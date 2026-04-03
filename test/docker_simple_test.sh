#!/bin/bash
# Simple Docker test for VPN - No compose needed

set -e

cd ~/Desktop/nexusvpn

echo "=== Building Docker image ==="
docker build -f Dockerfile.test -t nexusvpn:test . --no-cache

echo ""
echo "=== Cleaning up old containers ==="
docker stop vpn-server vpn-client 2>/dev/null || true
docker rm vpn-server vpn-client 2>/dev/null || true

echo ""
echo "=== Starting VPN Server ==="
docker run -d --name vpn-server \
    --cap-add NET_ADMIN \
    --cap-add SYS_MODULE \
    --device /dev/net/tun \
    --network host \
    nexusvpn:test \
    sh -c "cd /app/build && ./nexusvpn --server --port 1194 --ip 10.0.0.1/24"

sleep 3

echo ""
echo "=== Starting VPN Client ==="
docker run -d --name vpn-client \
    --cap-add NET_ADMIN \
    --cap-add SYS_MODULE \
    --device /dev/net/tun \
    --network host \
    nexusvpn:test \
    sh -c "sleep 2 && cd /app/build && ./nexusvpn --client --host 127.0.0.1 --port 1194 --ip 10.0.0.2/24"

sleep 5

echo ""
echo "=== Testing VPN Tunnel ==="
ping -c 3 10.0.0.1

echo ""
echo "=== Container Logs ==="
echo "--- Server ---"
docker logs vpn-server --tail 10
echo ""
echo "--- Client ---"
docker logs vpn-client --tail 10

echo ""
echo "=== Cleaning Up ==="
docker stop vpn-server vpn-client
docker rm vpn-server vpn-client

echo ""
echo "✅ Test complete!"
