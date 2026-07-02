#!/bin/sh

# Build Script for KRunner LLM Plugin
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building KRunner LLM Plugin${NC}"

echo "Checking dependencies..."
deps="cmake
g++
pkg-config
make
"

if command -v dnf >/dev/null 2>/dev/null; then
        sudo dnf install -y $deps
elif command -v zypper >/dev/null 2>/dev/null; then
        sudo zypper install -y $deps
elif command -v apt >/dev/null 2>/dev/null; then
        sudo apt update -y
        sudo apt install -y $deps
else
        echo 'WARNING: Could not find known package manager; dependencies may not be satisfied.'
fi

# Check GCC version
GCC_VERSION=$(g++ -dumpversion | cut -d. -f1)
if [ "$GCC_VERSION" -lt 14 ]; then
    echo -e "${RED}Error: GCC 14 or later is required (found GCC $GCC_VERSION)${NC}"
    exit 1
fi

echo -e "${GREEN}All dependencies found${NC}"

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Build directory exists. Cleaning...${NC}"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo -e "${GREEN}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_CXX_STANDARD=23

# Build
echo -e "${GREEN}Building...${NC}"
make -j$(nproc)
echo -e "${GREEN}Build successful!${NC}"
echo -e "${GREEN}Installing...${NC}"
sudo make install
echo -e "${GREEN}Install successful!${NC}"
kquitapp6 krunner && krunner &
