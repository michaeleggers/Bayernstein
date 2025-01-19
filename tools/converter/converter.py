import os
from PIL import Image

def convert_png_to_tga(folder_path: str):
    """
    Converts all PNG images in the specified folder to TGA format.

    Args:
        folder_path (str): Path to the folder containing PNG images.

    Raises:
        FileNotFoundError: If the specified folder does not exist.
    """
    if not os.path.exists(folder_path):
        raise FileNotFoundError(f"The folder '{folder_path}' does not exist.")
    
    # Create a subdirectory for TGA files
    tga_folder = os.path.join(folder_path, "tga_files")
    os.makedirs(tga_folder, exist_ok=True)

    for file_name in os.listdir(folder_path):
        if file_name.lower().endswith(".png"):
            file_path = os.path.join(folder_path, file_name)
            tga_name = os.path.splitext(file_name)[0] + ".tga"
            tga_path = os.path.join(tga_folder, tga_name)
            
            # Convert PNG to TGA
            try:
                with Image.open(file_path) as img:
                    img.save(tga_path, format="TGA")
                    print(f"Converted: {file_name} -> {tga_name}")
            except Exception as e:
                print(f"Failed to convert {file_name}: {e}")
            print(tga_path)

if __name__ == "__main__":
    folder = input("Enter the folder path containing PNG images: ").strip()
    convert_png_to_tga(folder)