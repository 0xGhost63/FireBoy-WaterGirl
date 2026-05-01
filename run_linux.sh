#!/bin/bash

echo "====================================="
echo " Building Fireboy & Watergirl..."
echo "====================================="

# Ensure build and bin directories exist
mkdir -p build/obj build/moc build/rcc build/ui bin

# Generate Makefile using qmake
qmake FireboyWatergirl.pro

# Compile the project using all available cores
if make -j$(nproc); then
    echo ""
    echo "====================================="
    echo " Build Successful! Running Game..."
    echo "====================================="
    # Run the executable from the bin directory
    ./bin/FireboyWatergirl
else
    echo ""
    echo "====================================="
    echo " Build Failed! Please check the errors."
    echo "====================================="
    exit 1
fi
