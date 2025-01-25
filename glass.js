import express from 'express';
import sharp from 'sharp';
import fs from 'fs';
import path from 'path';
import { Anthropic } from '@anthropic-ai/sdk';
import { fileURLToPath } from 'url';
import { dirname } from 'path';

const app = express();

// Initialize Anthropic client
const client = new Anthropic();

// Get the current directory name (like __dirname)
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Function to resize an image using sharp
async function resizeImage(inputPath, outputPath, targetSize = { width: 500, height: 500 }) {
    try {
        await sharp(inputPath)
            .resize(targetSize.width, targetSize.height)
            .toFile(outputPath);
        console.log(`Image resized and saved to ${outputPath}`);
    } catch (err) {
        console.error("Error resizing image:", err);
    }
}

// Function to encode image to Base64
function encodeImageToBase64(imagePath) {
    const imageData = fs.readFileSync(imagePath);
    return imageData.toString('base64');
}

// Function to send image to Claude
async function sendImageToClaude(outputImagePath, apiKey) {
    const imgBase64 = encodeImageToBase64(outputImagePath);
    const image1MediaType = 'image/jpeg';
    const message = 'I did not get anything';
    const anthropic = new Anthropic({
        apiKey: apiKey, // defaults to process.env["ANTHROPIC_API_KEY"]
      });
    try {
        // Send request to Claude AI
        const response = await anthropic.messages.stream({
            messages: [
                {
                    role: 'user',
                    content: [
                        {
                            type: 'image',
                            source: {
                                type: 'base64',
                                media_type: image1MediaType,
                                data: imgBase64,
                            },
                        },
                        {
                            type: 'text',
                            text: "You are an assistant for the visually impaired. Your job is to describe the scenery showed in the image so they can get a good idea of what their surroundings are.",
                        },
                    ],
                },
            ],
            model: 'claude-3-5-sonnet-20241022',
            max_tokens: 1024,
        }).on('text', (text) => {
            console.log(text);
            return text;
        });
    } catch (err) {
        console.error("Error sending image to Claude:", err);
    }
}

// Define routes for the Express server
app.get('/', (req, res) => {
    res.send("Welcome to the Express server!");
});

app.get('/query', async (req, res) => {
    const inputImagePath = path.join(__dirname, 'images', 'alex.jpeg');
    const outputImagePath = path.join(__dirname, 'images', 'resized_image.jpeg');

    // Resize the image
    await resizeImage(inputImagePath, outputImagePath);

    // Fetch the Claude API key from environment variables
    const apiKey = process.env.CLAUDE_KEY;  // Ensure your API key is in the environment variables

    // Send the resized image to Claude AI
    const response = await sendImageToClaude(outputImagePath, apiKey);

    res.send(response);
});

// Start the server
app.listen(80, () => {
    console.log('Server is running on port 80');
});
