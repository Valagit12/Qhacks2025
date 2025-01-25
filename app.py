import os
import anthropic
from PIL import Image
import base64
 
def resize_image(input_path, output_path, target_size=(500, 500)):
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

input_image_path = "./images/alex.jpeg"
output_image_path = "./images/resized_image.jpeg"
resize_image(input_image_path, output_image_path)

api_key = os.environ["CLAUDE_KEY"]


client = anthropic.Anthropic(
    # defaults to os.environ.get("ANTHROPIC_API_KEY")
    api_key=api_key,
)
image1_media_type = 'image/jpeg'
with open(output_image_path, "rb") as img_file:
        # Read the image as binary
        img_data = img_file.read()

        # Encode the binary data to base64
        img_base64 = base64.standard_b64encode(img_data).decode("utf-8")
        message = client.messages.create(
            model="claude-3-5-sonnet-20241022",
            max_tokens=2000,
            messages=[
                {
                    "role": "user",
                    "content": [
                        {
                            "type": "image",
                            "source": {
                                "type": "base64",
                                "media_type": image1_media_type,
                                "data": img_base64,
                            },
                        },
                        {
                            "type": "text",
                            "text": "You are an assistant for the visually impaired. Your job is to describe the scenery showed in the image so they can get a good idea of what their surroundings are."
                        }
                    ],
                }
            ],
        )
print(message)


# # Example usage:


