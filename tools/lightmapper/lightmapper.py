from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import cv2
from tqdm import tqdm
from PIL import Image


from renderer import Renderer
from data_structures.scene import Scene
from data_structures.color import Color
import util.hemicube as hemicube


class Lightmapper:

    def __init__(self, scene: Scene, renderer: Renderer):

        self.scene = scene
        self.renderer = renderer
        self.base_path = Path(__file__).resolve().parent

    def generate_lightmap(self,  lightmap_path: Path, iterations=5):

        

        for iteration in range(iterations):
            a = 0
            correction_map = hemicube.generate_correction_map(self.renderer.viewport_size)

            new_lightmap = self.scene.light_map.copy()
            for i, patch in tqdm(enumerate(self.scene.patches), total=len(self.scene.patches), desc=f'Iteration {iteration + 1}/{iterations}'):
                
                if patch.is_emissive == False:

                    position = patch.worldspace_coord
                    normal = patch.normal
                    views = hemicube.generate_hemicube_views(normal)

                    images = []
                    for view in views:
                        
                        image_array = self.renderer.render_single_image(position, view['direction'], view['up_vector'])
                        cut_image = hemicube.cut_image_with_frustum(image_array, view['frustum'])
                        images.append(cut_image)
                    hc = hemicube.merge_views_hemicube(images[0], images[1], images[2], images[3], images[4])
                    hc_corrected = hc * correction_map

                    sum_r = np.sum(hc_corrected[:, :, 0])  # Sum of the Red channel
                    sum_g = np.sum(hc_corrected[:, :, 1])  # Sum of the Green channel
                    sum_b = np.sum(hc_corrected[:, :, 2])  # Sum of the Blue channel
                    
                    new_lightmap[patch.x_tex_coord, patch.y_tex_coord, 0] = sum_r
                    new_lightmap[patch.x_tex_coord, patch.y_tex_coord, 1] = sum_g
                    new_lightmap[patch.x_tex_coord, patch.y_tex_coord, 2] = sum_b


                    #if a < 300 and views[0]['direction'][2] == 1.0 and position.z == 0.0:
                    #    a = a+1

                    #    print(i, position, views)
                    #    # Save hemicube hc as an image
                    #    hemicube_image = (hc * 255).astype(np.uint8)  # Convert to 8-bit per channel
                    #    hemicube_image_pil = Image.fromarray(hemicube_image)
                    #    hemicube_image_pil.save(f'hemicube_{i}.png')  # Save the hemicube image

            new_lightmap = self.fill_in_illegal_pixels(new_lightmap)
            self.scene.light_map = new_lightmap
            #temporary_lightmap_path = Path(self.base_path / 'temp' / 'lightmap.hdr')
            #self.scene.generate_light_map(temporary_lightmap_path)
            self.renderer.update_ligth_map()
            

        self.scene.generate_light_map(lightmap_path)
        #self.print_and_convert_hdr_image(lightmap_path, "debug_lightmap.png")
        self.save_lightmap_as_png(lightmap_path)

    def save_lightmap_as_png(self, lightmap_path: Path):
        # Load the HDR image with floating-point precision
        hdr_image = cv2.imread(str(lightmap_path), cv2.IMREAD_UNCHANGED)

        if hdr_image is None:
            print(f"Failed to load image from {str(lightmap_path)}")
            return
        
        # Check if it's loaded in BGR format and convert to RGB
        if hdr_image.shape[2] == 3:  # Only convert if it's a 3-channel image
            hdr_image = cv2.cvtColor(hdr_image, cv2.COLOR_BGR2RGB)

        # Convert HDR image (float32) back to 8-bit image for PNG saving
        # We scale the float values back into the 0-255 range
        normalized_image = cv2.normalize(hdr_image, None, 0, 255, cv2.NORM_MINMAX)
        png_image = np.clip(normalized_image, 0, 255).astype(np.uint8)

        # Convert back to BGR format for saving as PNG
        png_image_bgr = cv2.cvtColor(png_image, cv2.COLOR_RGB2BGR)

        # Save the PNG image
        lightmap_filename = lightmap_path.stem
        output_path = lightmap_path.parent / f'{lightmap_filename}.png'
        cv2.imwrite(str(output_path), png_image_bgr)

    def fill_in_illegal_pixels(self, lightmap):
        for illegal_pixel in self.scene.illegal_pixels:
            lightmap[illegal_pixel[0], illegal_pixel[1]] = lightmap[illegal_pixel[2], illegal_pixel[3]]
        return lightmap

    def print_and_convert_hdr_image(self, filepath: Path, output_png: str, sample_size: int = 5) -> None:
        # Load the HDR image with floating-point precision
        hdr_image = cv2.imread(str(filepath), cv2.IMREAD_UNCHANGED)
        
        if hdr_image is None:
            print(f"Failed to load image from {str(filepath)}")
            return

        # Check if it's loaded in BGR format and convert to RGB
        if hdr_image.shape[2] == 3:  # Only convert if it's a 3-channel image
            hdr_image = cv2.cvtColor(hdr_image, cv2.COLOR_BGR2RGB)
        
        # Print image dimensions and data type
        print(f"Image dimensions: {hdr_image.shape}")
        print(f"Image data type: {hdr_image.dtype}")
        
        # Print a sample of the pixel values for inspection
        #print(f"Sample pixel values (first {sample_size} rows and columns):")
        #print(hdr_image[:sample_size, :sample_size])

        # Optional: Print the min and max values to see the value range
        print(f"Min pixel value: {np.min(hdr_image)}")
        print(f"Max pixel value: {np.max(hdr_image)}")

        # Convert HDR image (float32) back to 8-bit image for PNG saving
        # We scale the float values back into the 0-255 range
        normalized_image = cv2.normalize(hdr_image, None, 0, 255, cv2.NORM_MINMAX)
        png_image = np.clip(normalized_image, 0, 255).astype(np.uint8)

        # Convert back to BGR format for saving as PNG
        png_image_bgr = cv2.cvtColor(png_image, cv2.COLOR_RGB2BGR)

        # Save the PNG image
        cv2.imwrite(output_png, png_image_bgr)
        print(f"Saved image as {output_png}")
        plt.imshow(hdr_image)
        plt.axis('off') 
        plt.show()

    def quit(self):
        self.renderer.destroy()
        pass


if __name__ == '__main__':

    lightmapper = Lightmapper()
    
    lightmapper.generate_lightmap()
    lightmapper.quit()