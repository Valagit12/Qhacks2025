import os
import anthropic
from PIL import Image
 
api_key = os.environ["CLAUDE_KEY"]


client = anthropic.Anthropic(
    # defaults to os.environ.get("ANTHROPIC_API_KEY")
    api_key=api_key,
)
# message = client.messages.create(
#     model="claude-3-5-sonnet-20241022",
#     max_tokens=1024,
#     messages=[
#         {"role": "user", "content": "Hello, Claude"}
#     ]
# )

def resize_image(input_path, output_path, target_size=(200, 200)):
    """
    Resizes an image to the target size and saves it.

    :param input_path: Path to the input image.
    :param output_path: Path to save the resized image.
    :param target_size: Tuple specifying the target dimensions (width, height).
    """
    # Open the input image
    with Image.open(input_path) as img:
        # Resize the image to the target size
        img_resized = img.resize(target_size)

        # Save the resized image to the output path
        img_resized.save(output_path)

    print(f"Image resized and saved to {output_path}")

# Example usage:
input_image_path = "./images/alexflex.jpg"
output_image_path = "./images/resized_image.jpg"
resize_image(input_image_path, output_image_path)


