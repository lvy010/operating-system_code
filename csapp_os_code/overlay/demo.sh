#!/bin/bash

sudo rm -rf lower upper work overlay
mkdir -p lower upper work overlay

# Mount the overlay filesystem
mount -t overlay overlay \
    -o lowerdir=lower,upperdir=upper,workdir=work \
    overlay/

echo 'lower' > lower/lower.txt
echo 'upper' > upper/upper.txt
