const textToSpeech = require("@google-cloud/text-to-speech");
const fs = require("fs");
const util = require("util");

class TextToSpeech {
    constructor(langCode = "en-US", encoding = "MP3", voiceName = "en-US-Journey-F", streamEncoding = "OGG_OPUS") {
        this.client = new textToSpeech.TextToSpeechClient();
        this.langCode = langCode;
        this.encoding = encoding;
        this.voiceName = voiceName;
        this.streamEncoding = streamEncoding;
    }

    async synthTextToSpeech (text) {
        const request = {
            input: { text: text },
            // Select the language and SSML voice gender (optional)
            voice: { languageCode: this.langCode, name: this.voiceName },
            // select the type of audio encoding
            audioConfig: { audioEncoding: this.encoding, sampleRateHertz: 16000 },
        };

        try {
            const [response] = await this.client.synthesizeSpeech(request);

            return response.audioContent;
        } catch (error) {
            console.log(error);
            throw "Failed to get text-to-speech";
        }
    }

    async streamTextToSpeech (textStream, onData) {
        const streamConfig = {
            voice: { languageCode: this.langCode, name: this.voiceName },
            streamingAudioConfig: { audioEncoding: this.streamEncoding }
        };
        const request = {
            streamingConfig: streamConfig
        };

        const stream = await this.client.streamingSynthesize();

        stream.write(request);

        textStream.on("text", (text) => {
            console.log(text);
            stream.write({
                input: { text: text }
            });
        });
        textStream.done().then(() => {
            stream.end();
        });

        return stream
    }
}

module.exports = TextToSpeech;