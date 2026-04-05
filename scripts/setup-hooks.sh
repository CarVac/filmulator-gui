#!/bin/bash
# Ensure the scripts are executable
chmod +x scripts/pre-commit
# Copy the hook
cp scripts/pre-commit .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
echo "Pre-commit hook installed successfully."
