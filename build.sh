#!/usr/bin/env bash
set -e

echo "==> Building tidy_langchain..."

make clean
make

echo ""
echo "==> Build finished."

echo ""
echo "Available binaries:"
echo "  - ./example_mock"
echo "  - ./example_posix"

echo ""
echo "Run examples:"
echo "  ./example_mock"
echo "  ./example_posix (requires OPENAI_API_KEY)"