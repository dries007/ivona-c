#include <argp.h>
#include <time.h>
#include <stdlib.h>
#include "ivona.h"

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

static char doc[] = "(c) 2016 - Dries007.net - Under MIT License - CLI IVONA interface. \nFor valid argument options: http://developer.ivona.com/en/speechcloud/datatypes.html";
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
    { 0 }
};

struct arguments {
    char data[1024 * 8];
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
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;
    switch (key)
    {
        case 'f': arguments->format = arg; break;
        case 'q': arguments->sample_rate = atoi(arg); break;
        case 'r': arguments->rate = arg; break;
        case 'v': arguments->volume = arg; break;
        case 's': arguments->sentence_break = atoi(arg); break;
        case 'p': arguments->paragraph_break = atoi(arg); break;
        case 'n': arguments->name = arg; break;
        case 'l': arguments->language = arg; break;
        case 'g': arguments->gender = arg; break;
        case 'R': arguments->region = arg; break;
        case 'K': arguments->key = arg; break;
        case 'S': arguments->secret = arg; break;
        case 'F': arguments->file = arg; break;
        case ARGP_KEY_ARG:
            strcat(arguments->data, arg);
            strcat(arguments->data, " ");
            break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc};

int main(int argc, char* argv[])
{
    struct arguments args;

    args.data[0] = 0;
    args.format = "MP3";
    args.sample_rate = 22050;
    args.rate = "default";
    args.volume = "default";
    args.sentence_break = 400;
    args.paragraph_break = 650;
    args.name = getenv("IVONA_NAME");
    if (args.name == NULL) args.name = "Salli";
    args.language = getenv("IVONA_LANG");
    if (args.language == NULL) args.language = "en-US";
    args.gender = getenv("IVONA_GENDER");
    if (args.gender == NULL) args.gender = "Female";
    args.region = getenv("IVONA_ENDPOINT");
    if (args.region == NULL) args.region = "eu-west-1";
    args.file = "-";
    args.key = getenv("IVONA_KEY");
    args.secret = getenv("IVONA_SECRET");

    argp_parse(&argp, argc, argv, 0, 0, &args);

    char payload[300 + strlen(args.data)];
    sprintf(payload, "{\"Input\":{\"Data\":\"%s\",\"Type\":\"text/plain\"},\"OutputFormat\":{\"Codec\":\"%s\",\"SampleRate\":%d},\"Parameters\":{\"Rate\":\"%s\",\"Volume\":\"%s\",\"SentenceBreak\":%d,\"ParagraphBreak\":%d},\"Voice\":{\"Name\":\"%s\",\"Language\":\"%s\",\"Gender\":\"%s\"}}",
            args.data, args.format, args.sample_rate, args.rate, args.volume, args.sentence_break, args.paragraph_break, args.name, args.language, args.gender);
    ivona_request("CreateSpeech", "ivonacloud.com", args.region, "tts", payload, args.secret, args.key, args.file, time(NULL));
}
