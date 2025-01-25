import os
import base64
import anthropic
from PIL import Image

def resize_image(input_path, output_path, target_size=(500, 500)):
    """
    Resizes an image to the target size and saves it.

    :param input_path: Path to the input image.
    :param output_path: Path to save the resized image.
    :param target_size: Tuple specifying the target dimensions (width, height).
    """
    with Image.open(input_path) as img:
        img_resized = img.resize(target_size)
        img_resized.save(output_path)
    print(f"Image resized and saved to {output_path}")

def encode_image_to_base64(image_path):
    """
    Encodes an image to base64 format.
    
    :param image_path: Path to the image file.
    :return: Base64 encoded string of the image.
    """
    with open(image_path, "rb") as img_file:
        img_data = img_file.read()
        return base64.standard_b64encode(img_data).decode("utf-8")

def send_image_to_claude(output_image_path, api_key):
    """
    Sends the image to Claude AI for description.
    
    :param output_image_path: Path to the resized image.
    :param api_key: Claude API key.
    :return: Response from Claude AI.
    """
    img_base64 = encode_image_to_base64(output_image_path)
    image1_media_type = 'image/jpeg'
    message = 'I did not get anything'
    client = anthropic.Anthropic(api_key=api_key)
    with client.messages.stream(
        max_tokens=1024,
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
        model="claude-3-5-sonnet-20241022",
    ) as stream:
        for text in stream.text_stream:
            print(text, end="", flush=True)
            if (message == 'I did not get anything'):
                message = text
            else:
                message += text
    print()
    print()
    for i in stream:
        print('this is the stuff you want')
    
    return message

# Example usage:
def main():
    print('I ran')

if __name__ == "__main__":
    main()
