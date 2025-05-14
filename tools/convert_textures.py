#!/usr/bin/env python
"""
Convert texture files to C arrays for embedding in executable
"""

import os
import sys
import base64

def bytes_to_c_array(data, name):
    """Convert bytes to a C array definition"""
    output = f"const unsigned char {name}_png[] = {{\n    "
    line_length = 0
    for i, byte in enumerate(data):
        output += f"0x{byte:02x}, "
        line_length += 1
        if line_length == 12:  # Format 12 bytes per line
            output += "\n    "
            line_length = 0
    
    # Remove trailing comma and add closing brace
    if output.endswith(", \n    "):
        output = output[:-7] + "\n};\n"
    else:
        output = output[:-2] + "\n};\n"
    
    # Add size variable
    output += f"const unsigned int {name}_png_size = {len(data)};\n"
    
    return output

def convert_file(filename, output_file):
    """Convert a file to a C array and write to output file"""
    basename = os.path.basename(filename)
    varname = os.path.splitext(basename)[0]
    
    print(f"Converting {filename} to C array as {varname}_png...")
    
    with open(filename, 'rb') as f:
        data = f.read()
    
    c_array = bytes_to_c_array(data, varname)
    
    output_file.write(f"// Generated from {basename}\n")
    output_file.write(c_array)
    output_file.write("\n")

def main():
    """Main function"""
    if len(sys.argv) < 3:
        print("Usage: python convert_textures.py <textures_dir> <output_file>")
        return 1
    
    textures_dir = sys.argv[1]
    output_file_path = sys.argv[2]
    
    if not os.path.isdir(textures_dir):
        print(f"Error: {textures_dir} is not a directory")
        return 1
    
    with open(output_file_path, 'w') as output_file:
        output_file.write("/* Auto-generated texture data - DO NOT EDIT */\n\n")
        output_file.write("#include \"embedded_textures.h\"\n\n")
        
        for filename in sorted(os.listdir(textures_dir)):
            if filename.endswith('.png'):
                full_path = os.path.join(textures_dir, filename)
                convert_file(full_path, output_file)
    
    print(f"Successfully wrote texture data to {output_file_path}")
    return 0

if __name__ == "__main__":
    sys.exit(main()) 