#!/usr/bin/env bash
# Simple helper to install a MinGW toolchain on Debian-based systems
# Allows building the project with g++ on non-Windows hosts

set -e

if ! command -v apt-get >/dev/null; then
  echo "apt-get not available. Install a MinGW-w64 toolchain manually." >&2
  exit 1
fi

sudo apt-get update
sudo apt-get install -y mingw-w64 g++

echo "Toolchain installed. You can now build with g++ src/*.cpp -o screenshot.exe"
