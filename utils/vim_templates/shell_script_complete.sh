#!/bin/bash

# Check if the script is called with at least one argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 <arg1> <arg2> ..."
    exit 1
fi

# Loop through the command-line arguments
while [ $# -gt 0 ]; do
    case "$1" in
        -a|--optionA)
            # Handle option A
            echo "Option A is enabled"
            ;;
        -b|--optionB)
            # Handle option B
            echo "Option B is enabled"
            ;;
        -c|--optionC)
            # Handle option C
            echo "Option C is enabled"
            ;;
        *)
            # Handle unrecognized options or arguments
            echo "Unrecognized option or argument: $1"
            ;;
    esac
    shift
done
