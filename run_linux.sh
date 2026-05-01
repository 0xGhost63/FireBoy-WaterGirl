#!/bin/bash

echo "====================================="
echo " Building Fireboy & Watergirl..."
echo "====================================="

# Ensure build and bin directories exist
mkdir -p build/obj build/moc build/rcc build/ui bin

# Sync root styles to the assets folder and force resource re-compilation
if [ -f "styles.qss" ]; then
    cp styles.qss assets/styles/game.qss
    touch resources.qrc
fi

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
