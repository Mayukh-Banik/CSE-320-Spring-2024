#!/bin/bash

# Maximum file size in bytes
n=100

# Directory to save files that cause errors
error_input_dir="error_inputs"
mkdir -p "$error_input_dir"

# Loop from 1 byte to n bytes
for size in $(seq 1 $n); do
  # Generate a random binary file named "input.txt" of size "size"
  dd if=/dev/urandom of=input.txt bs=1 count=$size 2>/dev/null

  # Compress input.txt to compressed.bin, checking for errors
  if ! bin/huff -c < input.txt > compressed.bin; then
    exit_status=$?
    echo "Error detected with exit status $exit_status for file of size $size bytes. Saving input file."
    cp input.txt "$error_input_dir/input_${size}_error_$exit_status.bin"
    # Optionally, break or continue depending on desired behavior
    # break # Stop the loop if you want to halt at the first error
    # continue # Skip to the next file size
  fi

  # Optional: Additional logic to read the second byte and compare values could go here

  # Cleanup temporary files for successful operations
  rm -f input.txt compressed.bin
done

echo "Processing completed. Check '$error_input_dir' for any input files that caused errors."
