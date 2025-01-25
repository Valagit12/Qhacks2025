const express = require("express");
const dotnev = require("dotenv");
const axios = require("axios");
const fs = require("fs");
const path = require("path");
const TextToSpeech = require("./speech.js");
const ClaudeClient = require("./claude.js");

dotnev.config();

const app = express();
const port = process.env.PORT;
const claudeAPIKey = process.env.CLAUDE_API_KEY;

const textToSpeech = new TextToSpeech();
const claude = new ClaudeClient(claudeAPIKey);

app.use(express.json());
app.use(express.raw({ type: "image/jpg", limit: "5mb" }));

app.get("/connect", async (req, res) => {
    const ip = req.body.ip;
    const port = req.body.port;

    const options = {
        'method': 'GET',
        'url': `http://${ip}:${port}`,
        'headers': {
          'Content-Type': 'application/json'
        },
        data: {}
      };
    
    try {
        console.log(`Attempting connection with ${options.url}`);
        const result = await axios(options);
        if (result.status === 200) {
            console.log("Connection Successful");
            res.status(result.status).send("Connection Successful");
        } else {
            console.log("Connection Unsuccessful");
            res.status(result.status).send("Connection Unsuccessful");
        }
    } catch (e) {
        console.log(`Connection Unsuccessful: ${e?.response?.status ?? ""}`);
        res.status(500).send("Connection Unsuccessful");
    }
});

app.post("/describe", async (req, res) => {
    console.log(`Incoming request to describe.`);
    const img = req.body;
    
    try {
        const description = await claude.requestImageDescription(img);

        console.log(description);
        const text = description.content[0].text;
        const audio = await textToSpeech.synthTextToSpeech(text);
        
        const outputFilePath = path.join(__dirname, "audio.mp3");

        // Write the audio file to disk
        fs.writeFileSync(outputFilePath, audio);

        res.set({
            "Content-Type": "audio/mpeg"
        });

        res.send(audio);
    } catch (error) {
        res.status(500).send(error);
    }
});

app.get("/audio.mp3", async (req, res) => {
    const filePath = path.join(__dirname, "audio.mp3"); // Path to your MP3 file

    // Check if the file exists
    if (!fs.existsSync(filePath)) {
        return res.status(404).send("MP3 file not found");
    }

    // Set headers
    res.set({
        "Content-Type": "audio/mpeg",
        "Content-Disposition": "inline", // Optional: "attachment" for download
    });

    // Stream the MP3 file
    const readStream = fs.createReadStream(filePath);
    readStream.pipe(res);

    // Handle errors
    readStream.on("error", (err) => {   
        console.error("Error streaming audio:", err);
        res.status(500).send("Error playing audio");
    });
});

app.post("/transcribe", async (req, res) => {
    console.log(`Incoming request to transcribe.`);
    const img = req.body;
    
    try {
        const description = await claude.requestTranscription(img);

        console.log(description);
        const text = description.content[0].text;
        const audio = await textToSpeech.synthTextToSpeech(text);

        res.set({
            "Content-Type": "audio/mpeg"
        });

        res.send(audio);
    } catch (error) {
        res.status(500).send(error);
    }
});

app.listen(port, () => {
    console.log(`[server]: Server is running at http://localhost:${port}`);
});