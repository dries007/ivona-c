IVONA-c
=======

CLI interface for the [IVONA](https://www.ivona.com) speech cloud.
This program is useless without credentials to there service.

Can pipe the audio to stdout or to a file.

[ivona-cached](ivona-cached) is a bash script that can be used to break up requests into chunks. The chunks get stored
on disk, to be reused later. Be aware that that may not be allowed by IVONA, depending on your license agreement.

Limitations
-----------

- The only supported action is 'CreateSpeech'.
- Has no tested support for utf-8.
- Only supports plain text, no ssml.
- No SpeechMarks or Lexicons.

License
-------

Project under MIT License, see [LICENSE](LICENSE) file.

As this program uses the IVONA Speach Cloud HTTP API, you must also agree to there ToS.

Example usage
-------------

Set up the following enviroment variables:

- ```IVONA_NAME``` (Optional)
- ```IVONA_LANG``` (Optional)
- ```IVONA_GENDER``` (Optional)
- ```IVONA_ENDPOINT``` (Optional)
- ```IVONA_KEY```
- ```IVONA_SECRET```

```ivona "Hello, this is your computer speaking." | mplayer - -cache 1024```

```cat textfile | ivona | mplayer - -cache 1024```
