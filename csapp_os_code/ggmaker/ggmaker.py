# Author: claude-3.7-sonnet

#!/usr/bin/env python3

import os
import sys
import yaml
import shutil

def create_game_structure(script_path):
    # Read the script file
    with open(script_path, 'r', encoding='utf-8') as file:
        script = yaml.safe_load(file)
    
    # Create game directory if it doesn't exist
    game_dir = "game"
    if not os.path.exists(game_dir):
        os.makedirs(game_dir)
    
    # Process each scene
    for scene_name, scene_data in script.items():
        # Create scene directory
        scene_dir = os.path.join(game_dir, scene_name)
        if not os.path.exists(scene_dir):
            os.makedirs(scene_dir)
        
        # Create description file
        if 'description' in scene_data:
            description_path = os.path.join(scene_dir, ".description")
            with open(description_path, 'w', encoding='utf-8') as desc_file:
                desc_file.write(scene_data['description'])

        # Create symlink to image
        if 'image' in scene_data:
            image_path = scene_data['image']
            symlink_path = os.path.join(scene_dir, ".scene.jpg")
            # If the symlink exists, remove it first
            if os.path.exists(symlink_path):
                os.remove(symlink_path)
            # Create relative symlink to the image
            try:
                os.symlink(os.path.join("..", "..", image_path), symlink_path)
            except OSError:
                print(f"Warning: Could not create symlink for {image_path}")
        
        # Create option files pointing to next scenes
        for option_num, option_data in scene_data.items():
            if isinstance(option_num, int):
                option_desc = option_data.get('description', '')
                next_scene = option_data.get('next', '')
                
                # Create option symlink with name format "1 - description"
                option_file = os.path.join(scene_dir, f"{option_num}.{option_desc}")
                # If the symlink exists, remove it first
                if os.path.exists(option_file):
                    os.remove(option_file)
                # Create relative symlink to the next scene
                try:
                    os.symlink(os.path.join("..", next_scene), option_file)
                except OSError:
                    print(f"Warning: Could not create symlink for option {option_num} to {next_scene}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python ggmaker.py <script_file>")
        sys.exit(1)
    
    script_path = sys.argv[1]
    create_game_structure(script_path)
    print(f"Game structure created successfully from {script_path}")

if __name__ == "__main__":
    main()
