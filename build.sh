#!/bin/bash
# Build script using compatible Python version

export PATH="/opt/homebrew/bin:$PATH"
/opt/homebrew/bin/python3.13 ~/.platformio/penv/bin/platformio run
