const { Anthropic } = require("@anthropic-ai/sdk");
const { descriptionPrompt, transcriptionPrompt } = require("./prompts.js");

class ClaudeClient {
    constructor(apiKey) {
        this.client = new Anthropic({
            apiKey: apiKey
        });
    }

    async requestTranscription (rawImg) {
        const imgBase64 = rawImg.toString("base64");
        const imageMediaType = 'image/jpeg';
        try {
            const response = await this.client.messages.create({
                messages: [
                    {
                        role: 'user',
                        content: [
                            {
                                type: 'image',
                                source: {
                                    type: 'base64',
                                    media_type: imageMediaType,
                                    data: imgBase64,
                                },
                            },
                            {
                                type: 'text',
                                text: transcriptionPrompt,
                            },
                        ],
                    },
                ],
                model: 'claude-3-5-sonnet-20241022',
                max_tokens: 1024,
            });
            
            return response;
        } catch (error) {
            console.log(error);
            throw "Failed to get description from Claude";
        }
    }

    async requestImageDescription (rawImg) {
        const imgBase64 = rawImg.toString("base64");
        const imageMediaType = 'image/jpeg';
        try {
            const response = await this.client.messages.create({
                messages: [
                    {
                        role: 'user',
                        content: [
                            {
                                type: 'image',
                                source: {
                                    type: 'base64',
                                    media_type: imageMediaType,
                                    data: imgBase64,
                                },
                            },
                            {
                                type: 'text',
                                text: descriptionPrompt,
                            },
                        ],
                    },
                ],
                model: 'claude-3-5-sonnet-20241022',
                max_tokens: 1024,
            });
            
            return response;
        } catch (error) {
            console.log(error);
            throw "Failed to get description from Claude";
        }
    }

    async requestImageDescriptionStream (rawImg) {
        const imgBase64 = rawImg.toString("base64");
        const imageMediaType = 'image/jpeg';
        try {
            const response = await this.client.messages.stream({
                messages: [
                    {
                        role: 'user',
                        content: [
                            {
                                type: 'image',
                                source: {
                                    type: 'base64',
                                    media_type: imageMediaType,
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
            });

            return response;
        } catch (error) {
            console.log(error);
            throw "Failed to get description from Claude";
        }
    }
}

module.exports = ClaudeClient;