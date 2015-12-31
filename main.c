#include <argp.h>
#include <time.h>
#include <stdlib.h>
#include "ivona.h"
#include <sysexits.h>

/*
 * Copyright (c) 2016 Dries007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

static char doc[] =
        "(c) 2016 - Dries007.net - Under MIT License - CLI IVONA interface. \n"
        "For valid argument options: http://developer.ivona.com/en/speechcloud/datatypes.html\n"
        "Return codes follow sysexits specifications.";
static char args_doc[] = "[string up to 8192 characters]";
static struct argp_option options[] = {
    { "format",         'f', "format", 0, "Output format (MP3) 'MP3', 'MP4', 'OGG'", 1},
    { "samplerate",     'q', "quality", 0, "Sample rate in Hz. (22050)", 3},
    { "rate",           'r', "speed", 0, "Reading speed (default) 'x-slow', 'slow', 'medium', 'fast', 'x-fast', 'default'", 3},
    { "volume",         'v', "volume", 0, "Reading volume (default) 'silent', 'x-soft', 'soft', 'medium', 'loud', 'x-loud', 'default'", 3},
    { "sentence",       's', "time", 0, "Sentence break in ms (400)", 3},
    { "paragraph",      'p', "time", 0, "Paragraph break in ms (650)", 3},
    { "name",           'n', "name", 0, "Voice name ($IVONA_NAME || Salli)", 2},
    { "language",       'l', "lang-code", 0, "Voice language code ($IVONA_LANG || en-US)", 2},
    { "gender",         'g', "gender", 0, "Voice gender ($IVONA_GENDER || Female)", 2},
    { "region",         'R', "region", 0, "IVONA Speech Cloud endpoint ($IVONA_ENDPOINT || eu-west-1)", 1},
    { "key",            'K', "key", 0, "Key. ($IVONA_KEY)", 1},
    { "secret",         'S', "secret", 0, "Secret. ($IVONA_SECRET)", 1},
    { "file",           'F', "file", 0, "Filename to output to. (stdout)", 1},
    { "debug",          'd', 0, 0, "Print debug output to stdout.", 4},
    { 0 }
};

struct args {
    int datasize;
    char data[IVONA_MAX_DATA + 1];
    char* format;
    int sample_rate;
    char* rate;
    char* volume;
    int sentence_break;
    int paragraph_break;
    char* name;
    char* language;
    char* gender;
    char* region;
    char* key;
    char* secret;
    char* file;
    bool debug;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct args *args = state->input;
    switch (key)
    {
        case 'f': args->format = arg; break;
        case 'q': args->sample_rate = atoi(arg); break;
        case 'r': args->rate = arg; break;
        case 'v': args->volume = arg; break;
        case 's': args->sentence_break = atoi(arg); break;
        case 'p': args->paragraph_break = atoi(arg); break;
        case 'n': args->name = arg; break;
        case 'l': args->language = arg; break;
        case 'g': args->gender = arg; break;
        case 'R': args->region = arg; break;
        case 'K': args->key = arg; break;
        case 'S': args->secret = arg; break;
        case 'F': args->file = arg; break;
        case 'd': args->debug = true; break;
        case ARGP_KEY_ARG:
            args->datasize += strlen(arg);
            if (args->datasize > IVONA_MAX_DATA) return ARGP_KEY_ERROR;
            strcat(args->data, arg);
            strcat(args->data, " ");
            break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc};

int main(int argc, char* argv[])
{
    struct args a;

    a.datasize = 0;
    a.data[0] = 0;
    a.format = "MP3";
    a.sample_rate = 22050;
    a.rate = "default";
    a.volume = "default";
    a.sentence_break = 400;
    a.paragraph_break = 650;
    a.name = getenv("IVONA_NAME");
    if (a.name == NULL) a.name = "Salli";
    a.language = getenv("IVONA_LANG");
    if (a.language == NULL) a.language = "en-US";
    a.gender = getenv("IVONA_GENDER");
    if (a.gender == NULL) a.gender = "Female";
    a.region = getenv("IVONA_ENDPOINT");
    if (a.region == NULL) a.region = "eu-west-1";
    a.file = "-";
    a.key = getenv("IVONA_KEY");
    a.secret = getenv("IVONA_SECRET");
    a.debug = false;

    if (argp_parse(&argp, argc, argv, 0, 0, &a) != 0)
    {
        if (a.datasize > IVONA_MAX_DATA)
        {
            printf("Data size > %d\n", IVONA_MAX_DATA);
            exit(EX_DATAERR);
        }
        exit(EX_USAGE);
    }

    if (a.format == NULL || a.sample_rate == 0 || a.rate == NULL || a.volume == NULL || a.sentence_break == 0
        || a.paragraph_break == 0 || a.name == NULL || a.language == NULL || a.gender == NULL || a.region == NULL
        || a.file == NULL || a.key == NULL || a.secret == NULL) exit(EX_USAGE);

    if (a.datasize == 0)
    {
        int c;
        while (EOF != (c = fgetc(stdin)))
        {
            a.data[a.datasize++] = (char) c;
            if (a.datasize == IVONA_MAX_DATA)
            {
                printf("Data size > %d\n", IVONA_MAX_DATA);
                exit(EX_DATAERR);
            }
        }
        if (a.data[a.datasize - 1] == '\n')
        {
            a.data[a.datasize - 1] = '\0';
            a.datasize --;
        }
        if (a.datasize == 0)
        {
            printf("No input data.\n");
            exit(EX_DATAERR);
        }
    }

    char payload[300 + strlen(a.data)];
    sprintf(payload,
            "{\"Input\":{\"Data\":\"%s\",\"Type\":\"text/plain\"},\"OutputFormat\":{\"Codec\":\"%s\",\"SampleRate\":%d},"
                    "\"Parameters\":{\"Rate\":\"%s\",\"Volume\":\"%s\",\"SentenceBreak\":%d,\"ParagraphBreak\":%d},"
                    "\"Voice\":{\"Name\":\"%s\",\"Language\":\"%s\",\"Gender\":\"%s\"}}",
            a.data, a.format, a.sample_rate, a.rate, a.volume, a.sentence_break, a.paragraph_break,
            a.name, a.language, a.gender);

    if (a.debug) printf("Payload: \n%s\n", payload);

    ivona_request("CreateSpeech", "ivonacloud.com", a.region, "tts", payload, a.secret, a.key, a.file, time(NULL), a.debug);
    return EX_OK;
}
