from OpenGL.GL import *
import numpy as np
from pathlib import Path
import cv2


class Material:
    """
        A basic texture.
    """

    
    def __init__(self, image_data=None, filepath: Path = None):
        """
        Initialize and load the texture.

        Parameters:
            image_data: Optional; a NumPy array representing the image data.
            filepath: Optional; path to the image file.
        """
        
        # Initialize the texture
        self.texture = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.texture)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)

        # Load from NumPy array or file
        if image_data is not None:
            self.load_from_array(image_data)
        elif filepath is not None:
            self.load_from_file(filepath)
        else:
            raise ValueError("Either image_data (as a NumPy array) or filepath must be provided.")

    def load_from_array(self, image_data):
        """Load texture data from a NumPy array."""
        if not isinstance(image_data, np.ndarray):
            raise ValueError("image_data must be a NumPy array.")
        
        # Get dimensions and convert to appropriate format
        image_height, image_width = image_data.shape[:2]
        img_data = np.array(image_data, dtype=np.uint8)

        # Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data)
        glGenerateMipmap(GL_TEXTURE_2D)

    def load_from_file(self, filepath: Path):
            """Load HDR texture data from a file."""
            # Use OpenCV to load the HDR image with float precision
            image = cv2.imread(str(filepath), cv2.IMREAD_UNCHANGED)

            # Convert BGR to RGB if needed
            image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

            # Get image dimensions
            image_height, image_width = image.shape[:2]

            # Ensure the image data is in float32 format (if it's not already)
            if image.dtype != np.float32:
                image = image.astype(np.float32)

            # Upload the texture data to OpenGL
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image_width, image_height, 0, GL_RGB, GL_FLOAT, image)

            # Generate mipmaps for the texture
            glGenerateMipmap(GL_TEXTURE_2D)

    def use(self) -> None:
        """
            Arm the texture for drawing.
        """

        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D,self.texture)

    def destroy(self) -> None:
        """
            Free the texture.
        """

        glDeleteTextures(1, (self.texture,))