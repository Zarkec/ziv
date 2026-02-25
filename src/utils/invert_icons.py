from PIL import Image, ImageOps
import os
import shutil

input_dir = r"D:\Code\cppcodes\qtcodes\ziv\resources\icons\temp"
light_dir = r"D:\Code\cppcodes\qtcodes\ziv\resources\icons\light"
dark_dir = r"D:\Code\cppcodes\qtcodes\ziv\resources\icons\dark"

os.makedirs(light_dir, exist_ok=True)
os.makedirs(dark_dir, exist_ok=True)

for filename in os.listdir(input_dir):
    if filename.lower().endswith((".png", ".jpg", ".jpeg", ".bmp")):
        path = os.path.join(input_dir, filename)

        img = Image.open(path)

        # Copy original to light folder
        shutil.copy(path, os.path.join(light_dir, filename))

        # Create inverted version for dark folder
        if img.mode == "RGBA":
            r, g, b, a = img.split()
            rgb = Image.merge("RGB", (r, g, b))
            inverted_rgb = ImageOps.invert(rgb)
            inverted = Image.merge("RGBA", (*inverted_rgb.split(), a))
        else:
            inverted = ImageOps.invert(img.convert("RGB"))

        inverted.save(os.path.join(dark_dir, filename))
        print(f"Processed: {filename}")

print("Done!")