#!/bin/bash

while IFS= read -r file; do
    echo "Processing $file"
    include-what-you-use "$file" -Iheaders
    read -n 1 -s -r -p "Press any key to continue..."
    echo
done < <(find headers sources -type f \( -name "*.hpp" -o -name "*.cpp" \))

