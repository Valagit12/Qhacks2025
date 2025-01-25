from flask import Flask, request, jsonify
from image_processor import resize_image, send_image_to_claude
import os

# Initialize the Flask application
glasses = Flask(__name__)

# Define a route that listens for GET requests
@glasses.route('/')
def home():
    return "Welcome to the Flask web server!"

# Define a route that listens for POST requests
@glasses.route('/receive', methods=['POST'])
def receive():
    # Get JSON data sent in the POST request
    data = request.get_json()

    if data:
        return jsonify({"status": "success", "received_data": data}), 200
    else:
        return jsonify({"status": "error", "message": "No data received"}), 400

# Define a route that listens for GET requests with query parameters
@glasses.route('/query', methods=['GET'])
def query():
    input_image_path = "./images/alex.jpeg"
    output_image_path = "./images/resized_image.jpeg"

    # Resize the image
    resize_image(input_image_path, output_image_path)

    # Provide the Claude API key
    api_key = "your_Claude_API_key_here"  # or fetch from environment variable

    # Send the resized image to Claude AI
    api_key = os.environ["CLAUDE_KEY"]
    response = send_image_to_claude(output_image_path, api_key)
    print(response)
    return ''


# Run the application
if __name__ == '__main__':
    glasses.run(debug=True, host='0.0.0.0', port=80)
