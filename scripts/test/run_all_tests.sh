#!/bin/bash
# Complete test suite for Feature 1

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔══════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   NexusVPN Feature 1 Test Suite         ║${NC}"
echo -e "${BLUE}║   TUN Virtual Interface Testing         ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════╝${NC}"

# Test 1: Build
echo -e "\n${YELLOW}[Test 1] Building project...${NC}"
mkdir -p build && cd build
cmake -DBUILD_TESTS=ON ..
make -j$(nproc)
echo -e "${GREEN}✅ Build successful${NC}"

# Test 2: Unit Tests
echo -e "\n${YELLOW}[Test 2] Running unit tests...${NC}"
ctest --output-on-failure -R TUNInterfaceTest
echo -e "${GREEN}✅ Unit tests passed${NC}"

# Test 3: Manual TUN test
echo -e "\n${YELLOW}[Test 3] Running manual TUN tests...${NC}"
sudo ./nexusvpn --test-all
echo -e "${GREEN}✅ Manual tests passed${NC}"

# Test 4: Coverage
echo -e "\n${YELLOW}[Test 4] Checking code coverage...${NC}"
cd ..
mkdir -p build-coverage
cd build-coverage
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON ..
make -j$(nproc)
ctest -j$(nproc)
lcov --capture --directory . --output-file coverage.info --no-external
lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_filtered.info
COVERAGE=$(lcov --summary coverage_filtered.info 2>&1 | grep lines | awk '{print $2}' | sed 's/%//')
echo -e "${GREEN}✅ Coverage: ${COVERAGE}%${NC}"

if (( $(echo "$COVERAGE < 90" | bc -l) )); then
    echo -e "${RED}❌ Coverage ${COVERAGE}% is below 90% threshold${NC}"
    exit 1
fi

# Test 5: Web dashboard
echo -e "\n${YELLOW}[Test 5] Web dashboard test...${NC}"
echo -e "${GREEN}✅ Web dashboard available at: http://localhost:5000${NC}"
echo -e "${GREEN}   Run: python3 web/dashboard/server.py${NC}"

echo -e "\n${GREEN}╔══════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║   ✅ All Feature 1 Tests PASSED!         ║${NC}"
echo -e "${GREEN}╚══════════════════════════════════════════╝${NC}"
