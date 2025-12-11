#!/usr/bin/env python3

# Author: Claude-3.7-sonnet

import os
import subprocess
import time
from rich.console import Console
from rich.layout import Layout
from rich.panel import Panel
from rich.tree import Tree
from rich.text import Text

# Directories to watch
DIRS = {
    "lower": "lower",
    "upper": "upper",
    "overlay": "overlay"
}

# Number of characters to preview from text files
PREVIEW_CHARS = 30

console = Console()

def get_file_preview(file_path):
    """Get a preview of the file content if it's a text file."""
    try:
        if os.path.isfile(file_path) and os.path.getsize(file_path) > 0:
            with open(file_path, 'r') as f:
                try:
                    content = f.read(PREVIEW_CHARS)
                    return f": \"{content}{'...' if len(content) == PREVIEW_CHARS else ''}\""
                except UnicodeDecodeError:
                    return ": [binary file]"
        return ""
    except Exception as e:
        return f": [error: {str(e)}]"

def build_directory_tree(directory):
    """Build a rich Tree representation of the directory."""
    tree = Tree(f"📁 {directory}")
    
    if not os.path.exists(directory):
        tree.add("[red]Directory does not exist")
        return tree
    
    for root, dirs, files in os.walk(directory):
        # Get the relative path from the base directory
        rel_path = os.path.relpath(root, directory)
        
        # Skip the root directory as it's already the tree root
        if rel_path == '.':
            current_node = tree
        else:
            # Find or create parent nodes as needed
            parts = rel_path.split(os.sep)
            current_node = tree
            for i, part in enumerate(parts):
                # Look for existing node
                found = False
                for child in current_node.children:
                    if isinstance(child, Tree) and child.label.plain.endswith(part):
                        current_node = child
                        found = True
                        break
                
                if not found:
                    # Create new node
                    new_node = Tree(f"📁 {part}")
                    current_node.add(new_node)
                    current_node = new_node
        
        # Add files to the current node
        for file in sorted(files):
            file_path = os.path.join(root, file)
            preview = get_file_preview(file_path).replace('\n', '↲')
            file_size = os.path.getsize(file_path)
            file_node = Text(f"📄 {file} ({file_size} bytes){preview}")
            current_node.add(file_node)
        
        # Sort directories
        dirs.sort()
    
    return tree

def render():
    # Create a layout that takes up the screen height minus 2 lines
    console_height = console.height
    layout = Layout(size=console_height - 2)
    
    # I did vibe coding; this is NOT correct ;)
    layout.split(
        Layout(name="banner"),
        Layout(name="upper_row", ratio=100),
    )

    layout["banner"].update(Panel("Overlay Filesystem Monitor", title="Overlay Filesystem Monitor"))
    
    layout["upper_row"].split(
        Layout(name="lower_dir", ratio=1),
        Layout(name="upper_dir", ratio=1),
        Layout(name="overlay_dir", ratio=1),
    )
    
    # Build trees for each directory
    lower_tree = build_directory_tree(DIRS["lower"])
    upper_tree = build_directory_tree(DIRS["upper"])
    overlay_tree = build_directory_tree(DIRS["overlay"])
    
    # Create panels with the trees
    layout["lower_dir"].update(Panel(lower_tree, title="Lower Directory"))
    layout["upper_dir"].update(Panel(upper_tree, title="Upper Directory"))
    layout["overlay_dir"].update(Panel(overlay_tree, title="Overlay Directory"))
    
    # Clear the screen and render the layout
    console.clear()
    console.print(layout)

def main():
    """Main function to continuously monitor directories."""
    try:
        while True:
            render()
            time.sleep(1)
    except KeyboardInterrupt:
        subprocess.run(["sudo", "umount", "overlay"])
        print("\nExiting file system watcher...")

if __name__ == "__main__":
    main()
