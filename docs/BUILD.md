BUILD.md (Markdown File)
# Building NexusVPN from Source

## Prerequisites

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install -y build-essential cmake libssl-dev libsodium-dev libgtest-dev
Fedora/RHEL
sudo dnf install -y gcc-c++ cmake openssl-devel libsodium-devel gtest-devel
Arch Linux
sudo pacman -S base-devel cmake openssl libsodium gtest
Build Steps
1. Clone Repository
git clone https://github.com/yourusername/nexusvpn.git
cd nexusvpn
2. Configure and Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
3. Run Tests
ctest --output-on-failure
4. Install
sudo make install
Build Options
Option	Default	Description
-DBUILD_TESTS=ON/OFF	ON	Build unit tests
-DCMAKE_BUILD_TYPE	Release	Debug, Release, RelWithDebInfo
-DENABLE_COVERAGE=ON/OFF	OFF	Enable code coverage
Docker Build
docker build -f Dockerfile.clean -t nexusvpn:latest .
Verification

After build, verify it works:

sudo ./nexusvpn --test-tun

Expected output:

✅ TUN interface created successfully

---

# Command to Create This File

Run this command:

```bash
cd ~/Desktop/nexusvpn
mkdir -p docs
nano docs/BUILD.md

Paste the markdown content → Save → Done.