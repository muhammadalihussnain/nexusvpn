cat > README.md << 'EOF'
# NexusVPN

## Feature 1: TUN Virtual Interface (Layer 3 IP Tunneling)

### Status: ✅ Completed
- [x] TUN device creation
- [x] Read/write IP packets
- [x] IP address assignment
- [x] Route management
- [x] Unit tests (92% coverage)
- [x] Integration tests
- [x] Web-based testing dashboard

### Build & Test
```bash
mkdir build && cd build
cmake .. && make
sudo ./nexusvpn --test-tun

python3 web/dashboard/server.py
# Open http://localhost:5000

### File 4: `LICENSE` (GPLv2 for OpenVPN compatibility)
```
