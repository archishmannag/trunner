#!/bin/sh

#Uninstall Script for KRunner LLM Plugin
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Uninstalling KRunner LLM Plugin${NC}"
sudo make uninstall
echo -e "${GREEN}Uninstallation complete${NC}"
kquitapp6 krunner && krunner &
