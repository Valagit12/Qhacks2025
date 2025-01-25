"""Synthesizes speech from the input string of text or ssml.
Make sure to be working in a virtual environment.

Note: ssml must be well-formed according to:
    https://www.w3.org/TR/speech-synthesis/
"""
from google.cloud import texttospeech
import asyncio

class SpeechToText:
    def __init__(
            self, 
            lang_code="en-US", 
            voice_name="en-US-Journey-F",
            encoding=texttospeech.AudioEncoding.MP3,
            stream_encoding=texttospeech.AudioEncoding.OGG_OPUS):

        self.client = texttospeech.TextToSpeechAsyncClient()
        self.voice = texttospeech.VoiceSelectionParams(
            language_code=lang_code, name=voice_name
        )
        self.audio_config = texttospeech.AudioConfig(
            audio_encoding=encoding
        )
        self.async_config = texttospeech.StreamingSynthesizeConfig(
            voice=self.voice,
            streaming_audio_config=texttospeech.StreamingAudioConfig(
                audio_encoding=stream_encoding
            )
        )
    
    def speech_to_text(self, text):
        assert text is not None

        input = texttospeech.SynthesisInput(text=text)
        response = self.client.synthesize_speech(
            input=input, voice=self.voice, audio_config=self.audio_config
        )

        return response.audio_content

    async def speech_to_text_async(self, text_iter):
        stream = await self.client.streaming_synthesize(
            requests=self._transform_iterable(text_iter)
        )

        return stream

    async def _transform_iterable(self, str_iter):

        init_request = texttospeech.StreamingSynthesizeRequest(
            streaming_config=self.async_config,
        )
        yield init_request

        async for item in str_iter:
            request = texttospeech.StreamingSynthesizeRequest(
                input=item
            )
            print(item)
            yield request

if __name__ == "__main__":
    client = SpeechToText()
    # audio = client.speech_to_text("Hello, what can I help you with?")

    # with open("output.mp3", "wb") as out:
    #     # Write the response to the output file.
    #     out.write(audio)
    #     print('Audio content written to file "output.mp3"')

    text = ["Hello, ", "what ", "can ", "I ", "help ", "you ", "with? "]
    async def text_iter():
        for word in text:
            await asyncio.sleep(0.001)
            print(word)
            yield word
    
    async def main():
        stream = client.speech_to_text_async(text_iter())
        async for res in stream:
            print(res)

    # asyncio.run(client.speech_to_text_async(text_iter))
    asyncio.run(main())