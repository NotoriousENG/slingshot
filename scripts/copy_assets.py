import os
import shutil
import fnmatch
from pathlib import Path

def load_ignore_patterns(ignore_file):
    """
    Load ignore patterns from the .assetignore file.
    """
    if not os.path.exists(ignore_file):
        return []

    with open(ignore_file, "r") as file:
        patterns = [line.strip() for line in file if line.strip() and not line.startswith("#")]
    return patterns

def should_ignore(file_path, ignore_patterns, base_dir):
    """
    Check if a file should be ignored based on the patterns.
    """
    relative_path = os.path.relpath(file_path, base_dir)
    for pattern in ignore_patterns:
        if fnmatch.fnmatch(relative_path, pattern):
            return True
    return False

def needs_update(source_file, target_file):
    """
    Check if source file is newer than target file.
    Returns True if target doesn't exist or source is newer.
    """
    if not os.path.exists(target_file):
        return True
    
    source_mtime = os.path.getmtime(source_file)
    target_mtime = os.path.getmtime(target_file)
    return source_mtime > target_mtime

def copy_assets(source_dir, target_dir):
    """
    Recursively copy the assets folder while ignoring files based on .assetignore.
    Only copies files that are newer than their target counterparts.
    """
    ignore_file = os.path.join(source_dir, ".assetignore")
    ignore_patterns = load_ignore_patterns(ignore_file)

    if not os.path.exists(source_dir):
        print(f"Error: Source directory '{source_dir}' does not exist.")
        return

    for root, dirs, files in os.walk(source_dir):
        relative_root = os.path.relpath(root, source_dir)
        target_root = os.path.join(target_dir, relative_root)

        # Create target directory
        os.makedirs(target_root, exist_ok=True)

        for file in files:
            source_file = os.path.join(root, file)
            if should_ignore(source_file, ignore_patterns, source_dir):
                print(f"Ignored: {source_file}")
                continue

            target_file = os.path.join(target_root, file)
            if needs_update(source_file, target_file):
                shutil.copy2(source_file, target_file)
                print(f"Copied: {source_file} -> {target_file}")
            else:
                print(f"Skipped (up to date): {source_file}")

def copy_and_update_html(source_file, target_file, project_name):
    """
    Copies the index.html file to the build directory and replaces 'index.js' with '{project_name}.js'.
    Only copies if source is newer than target.
    """
    # Check if we need to copy/update
    if not os.path.exists(source_file):
        print(f"Error: Source file '{source_file}' does not exist.")
        return

    if not needs_update(source_file, target_file):
        print(f"Skipped (up to date): {source_file}")
        return

    # Copy and update the HTML file
    shutil.copy(source_file, target_file)
    print(f"Copied '{source_file}' to '{target_file}'.")

    try:
        with open(target_file, "r") as file:
            content = file.read()

        updated_content = content.replace("index.js", f"{project_name}.js")

        with open(target_file, "w") as file:
            file.write(updated_content)

        print(f"Updated '{target_file}' to replace 'index.js' with '{project_name}.js'.")
    except Exception as e:
        print(f"Error updating '{target_file}': {e}")

if __name__ == "__main__":
    import sys
    list_dir = sys.argv[1] if len(sys.argv) > 1 else "./"
    list_dir = sys.argv[2] if len(sys.argv) > 2 else "./build"
    project_name = sys.argv[3] if len(sys.argv) > 3 else "ProjectName"

    copy_assets("./assets", "build/assets")

    # Check if Emscripten environment
    if os.environ.get("CMAKE_SYSTEM_NAME") == "Emscripten":
        copy_and_update_html("./web/index.html", f"./build/{project_name}.html", project_name)
