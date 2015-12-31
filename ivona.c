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
#include <sysexits.h>
#include "ivona.h"

void ivona_key_to_string(unsigned char *hash, char *out)
{
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(out + (i * 2), "%02x", hash[i]);
    }
    out[SHA256_ASCII_LENGTH] = 0;
}

// http://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c
void ivona_sha256(char *in, char *out)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, in, strlen(in));
    SHA256_Final(hash, &sha256);
    ivona_key_to_string(hash, out);
}

// time_t -> YYYYMMDDThhmmssZ
void ivona_ISO8601_date(time_t t, char *out)
{
    strftime(out, ISO8601_LENGTH + 1, "%Y%m%dT%H%M%SZ", gmtime(&t));
}

// time_t -> YYYYMMDD
void ivona_utc_yyyymmdd(time_t t, char *out)
{
    strftime(out, YYYYMMDD_LENGTH + 1, "%Y%m%d", gmtime(&t));
}

void ivona_request(char *uri_path, char *host, char *region, char *service, char *payload, char *secret, char *key, char *file, time_t time)
{
    FILE* bodyfile = NULL;
    bool stdout = strcmp("-", file) == 0;
    if (!stdout)
    {
        if ((bodyfile = fopen(file, "wb")) == NULL)
        {
            printf("File couldn't open.\n");
            exit(EX_IOERR);
        }
    }

    //Task 1: Create a canonical request.
    char sha256_payload[SHA256_ASCII_LENGTH + 1];
    ivona_sha256(payload, sha256_payload);

    char iso_time[ISO8601_LENGTH + 1];
    ivona_ISO8601_date(time, iso_time);

    char canonical_request[150 + sizeof(uri_path) + sizeof(host) + sizeof(region) + sizeof(service) + SHA256_ASCII_LENGTH + ISO8601_LENGTH + SHA256_ASCII_LENGTH];
    sprintf(canonical_request,
            "POST\n/%s\n\ncontent-length:%d\ncontent-type:application/json\nhost:%s.%s.%s\nx-amz-date:%s\n\ncontent-length;content-type;host;x-amz-date\n%s",
            uri_path, (int) strlen(payload), service, region, host, iso_time, sha256_payload);

    //Task 2: Create a string to sign
    char yyyymmdd[YYYYMMDD_LENGTH + 1];
    ivona_utc_yyyymmdd(time, yyyymmdd);

    char sha256_canonical_request[SHA256_ASCII_LENGTH + 1];
    ivona_sha256(canonical_request, sha256_canonical_request);

    char temp[1000]; //40 + SHA256_ASCII_LENGTH + ISO8601_LENGTH + YYYYMMDD_LENGTH + sizeof(region) + sizeof(service)
    sprintf(temp, "AWS4-HMAC-SHA256\n%s\n%s/%s/%s/aws4_request\n%s", iso_time, yyyymmdd, region, service, sha256_canonical_request);

    char string_to_sign[strlen(temp) + 1];
    strcpy(string_to_sign, temp);

    //Task 3: Create a signature
    char secret_aws4[5 + sizeof(secret)];
    strcpy(secret_aws4, "AWS4");
    strcat(secret_aws4, secret);

    unsigned char* date_k = HMAC(EVP_sha256(), secret_aws4, (int) strlen(secret_aws4), (unsigned char*)yyyymmdd, strlen(yyyymmdd), NULL, NULL);
    unsigned char* region_k = HMAC(EVP_sha256(), date_k, SHA256_DIGEST_LENGTH, (unsigned char*)region, strlen(region), NULL, NULL);
    unsigned char* service_k = HMAC(EVP_sha256(), region_k, SHA256_DIGEST_LENGTH, (unsigned char*)service, strlen(service), NULL, NULL);
    unsigned char* signing_k = HMAC(EVP_sha256(), service_k, SHA256_DIGEST_LENGTH, (unsigned char*)"aws4_request", strlen("aws4_request"), NULL, NULL);
    unsigned char* signature_k = HMAC(EVP_sha256(), signing_k, SHA256_DIGEST_LENGTH, (unsigned char*)string_to_sign, strlen(string_to_sign), NULL, NULL);

    char signature[SHA256_ASCII_LENGTH + 1];
    ivona_key_to_string(signature_k, signature);

    // Task 4: Prepare a signed request
    sprintf(temp,
            "Authorization: AWS4-HMAC-SHA256 Credential=%s/%s/%s/%s/aws4_request, SignedHeaders=content-length;content-type;host;x-amz-date, Signature=%s",
            key, yyyymmdd, region, service, signature);

    char header[strlen(temp) + 1];
    strcpy(header, temp);

    // Task 5: Make request
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl == NULL)
    {
        printf("Curl doesn't seem to exist.\n");
        exit(EX_SOFTWARE);
    }

    char url[sizeof(host) + sizeof(region) + sizeof(service) + sizeof(uri_path) + 15] = "https://";
    strcat(url, service);
    strcat(url, ".");
    strcat(url, region);
    strcat(url, ".");
    strcat(url, host);
    strcat(url, "/");
    strcat(url, uri_path);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(payload));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    //curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "Accept:");
    chunk = curl_slist_append(chunk, "Content-type: application/json");
    char date[15 + ISO8601_LENGTH] = "X-Amz-Date: ";
    strcat(date, iso_time);
    chunk = curl_slist_append(chunk, date);
    chunk = curl_slist_append(chunk, header);
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    if(res != CURLE_OK) fprintf(stderr, "curl_easy_setopt(CURLOPT_HTTPHEADER) failed: %s\n",  curl_easy_strerror(res));

    if (!stdout) curl_easy_setopt(curl, CURLOPT_WRITEDATA, bodyfile);
    res = curl_easy_perform(curl);

    if (!stdout)
    {
        if (fclose(bodyfile) != 0)
        {
            printf("File couldn't open.\n");
            exit(EX_IOERR);
        }
    }

    if(res != CURLE_OK) fprintf(stderr, "curl_easy_perform() failed: %s\n",  curl_easy_strerror(res));

    // ! cleanup !
    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);
}
