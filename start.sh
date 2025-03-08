#!/bin/bash

# Check for the presence of gcc
if ! command -v gcc &> /dev/null
then
    echo "Error: gcc is not installed."
    exit 1
fi

# Check for the presence of make
if ! command -v make &> /dev/null
then
    echo "Error: make is not installed."
    exit 1
fi

# Run the make command
make

# Check if the make command was successful
if [ $? -ne 0 ]; then
    echo "Error: the make command failed."
    exit 1
fi

# Open two new terminal windows and run ./client 8080
gnome-terminal -- bash -c "./client 8080; exec bash"
gnome-terminal -- bash -c "./client 8080; exec bash"

# Run the ./server command in the current terminal tab
./server