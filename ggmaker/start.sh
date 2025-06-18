#!/bin/bash

# Start interactive shell with custom cd function
# Create a temporary bash function file
TEMP_RC=$(mktemp)

# Define our custom cd function in the temporary file
cat > "$TEMP_RC" << 'EOF'

cd() {
    builtin cd "$@"
    clear
    viu .scene.jpg
    if [ -f .description ]; then
        cat .description | python3 -c 'import sys,time;[print(c,end="",flush=True) or time.sleep(0.03) for c in sys.stdin.read()]'
    fi
    read
    echo
    echo "---"
    ls | cat
    echo
}
alias q=exit
cd game/start
export PS1='(galgame) '
EOF

# Source the temporary file and start bash with it
bash --rcfile "$TEMP_RC"

# Clean up the temporary file when done
rm "$TEMP_RC"
