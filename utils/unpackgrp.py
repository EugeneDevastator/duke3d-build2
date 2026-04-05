#!/usr/bin/env python3
import struct
import os
import sys
from pathlib import Path

class GRPExtractor:
    def __init__(self):
        self.BLOCK_SIZE = 4096
    
    def extract_grp(self, grp_file_path):
        grp_path = Path(grp_file_path)
        if not grp_path.exists():
            print(f"Error: File '{grp_file_path}' not found")
            return False
        
        # Create output directory
        output_dir = grp_path.parent / f"{grp_path.stem}-grp"
        output_dir.mkdir(exist_ok=True)
        
        try:
            with open(grp_file_path, 'rb') as f:
                # Read header block
                header_data = f.read(16)  # 12 bytes name + 4 bytes length
                if len(header_data) < 16:
                    print("Error: Invalid GRP file format")
                    return False
                
                signature = header_data[:12].rstrip(b'\x00').decode('ascii', errors='ignore')
                file_count = struct.unpack('<I', header_data[12:16])[0]
                
                if signature != "KenSilverman":
                    print("Error: Invalid GRP file format - wrong signature")
                    return False
                
                print(f"Extracting {file_count} files to '{output_dir}'...")
                
                # Read file entries
                entries = []
                for i in range(file_count):
                    entry_data = f.read(16)
                    if len(entry_data) < 16:
                        print(f"Error: Unexpected end of file at entry {i}")
                        return False
                    
                    filename = entry_data[:12].rstrip(b'\x00').decode('ascii', errors='ignore')
                    file_size = struct.unpack('<I', entry_data[12:16])[0]
                    entries.append((filename, file_size))
                
                # Extract files
                for i, (filename, file_size) in enumerate(entries):
                    if not filename:
                        filename = f"unnamed_{i}"
                    
                    output_path = output_dir / filename
                    self._extract_file(f, output_path, file_size)
                    
                    progress = ((i + 1) * 100) // file_count
                    print(f"\rProgress: {i + 1}/{file_count} files ({progress}%)", end='', flush=True)
                
                print(f"\nExtraction completed successfully!")
                return True
                
        except Exception as e:
            print(f"Error during extraction: {e}")
            return False
    
    def _extract_file(self, input_file, output_path, file_size):
        with open(output_path, 'wb') as output:
            remaining = file_size
            while remaining > 0:
                chunk_size = min(self.BLOCK_SIZE, remaining)
                data = input_file.read(chunk_size)
                if not data:
                    break
                output.write(data)
                remaining -= len(data)

def main():
    if len(sys.argv) != 2:
        print("Usage: python grp_extractor.py <grp_file>")
        print("Example: python grp_extractor.py game.grp")
        sys.exit(1)
    
    extractor = GRPExtractor()
    success = extractor.extract_grp(sys.argv[1])
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
