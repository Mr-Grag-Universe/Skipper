import os
import sys
import hashlib

def get_file_hash(filepath):
    """Возвращает хэш содержимого файла."""
    hasher = hashlib.md5()
    with open(filepath, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hasher.update(chunk)
    return hasher.hexdigest()

def find_unique_files(directory):
    """Находит и возвращает уникальные файлы в директории."""
    seen_hashes = {}
    unique_files = []

    for root, _, files in os.walk(directory):
        for file in files:
            filepath = os.path.join(root, file)
            file_hash = get_file_hash(filepath)
            if file_hash not in seen_hashes:
                seen_hashes[file_hash] = filepath
                unique_files.append(filepath)
            else:
                print(f"Duplicate found: {filepath} (same as {seen_hashes[file_hash]})")

    return unique_files

def main():
    if len(sys.argv) != 2:
        print("Usage: python unique_files.py <directory>")
        sys.exit(1)

    directory = sys.argv[1]

    if not os.path.isdir(directory):
        print(f"Error: {directory} is not a valid directory")
        sys.exit(1)

    unique_files = find_unique_files(directory)
    
    if unique_files:
        print("Unique files:")
        for file in unique_files:
            print(file)
    else:
        print("No unique files found.")

if __name__ == "__main__":
    main()
