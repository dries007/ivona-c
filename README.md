IVONA-c
=======

CLI interface for IVONA speech cloud.

Can pipe the mp3 to stdout or to a file.

TODO
----

- Error catching

Limitations
-----------

- The only supported action is 'CreateSpeech'.
- Has no tested support for utf-8.
- Only supports plain text, no ssml.
- No SpeechMarks or Lexicons.

License
-------

Project under MIT License, see [LICENSE](LICENSE) file.

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
