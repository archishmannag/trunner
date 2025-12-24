# Build Script for KRunner LLM Plugin

#!/bin/bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building KRunner LLM Plugin${NC}"

# Check for required tools
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed${NC}"
        exit 1
    fi
}

echo "Checking dependencies..."
check_tool cmake
check_tool g++
check_tool pkg-config

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
echo ""
echo "To install, run:"
echo "  cd $BUILD_DIR && sudo make install"
echo ""
echo "Then restart KRunner:"
echo "  kquitapp6 krunner && krunner &"
