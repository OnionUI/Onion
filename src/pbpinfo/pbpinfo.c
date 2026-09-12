/*  The MIT License (MIT)
 *
 *  Copyright (c) 2026 OnionUI contributors
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

// Reads a PlayStation EBOOT.PBP container: prints the game title and serial
// id stored in the embedded PARAM.SFO, or extracts the embedded ICON0.PNG /
// PIC1.PNG image. Used by the scraper to match PSX (PBP) games by their real
// metadata instead of their filename.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PBP_MAGIC 0x50425000u // "\0PBP" as little-endian u32
#define PBP_HEADER_SIZE 40
#define SFO_HEADER_SIZE 20
#define SFO_ENTRY_SIZE 16
#define SFO_MAX_ENTRIES 256
#define SFO_MAX_SIZE (1024 * 1024)
#define PNG_SIG_SIZE 8

static const uint8_t png_sig[PNG_SIG_SIZE] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};

static uint16_t read_u16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }

static uint32_t read_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint8_t *read_section(FILE *fh, uint32_t offset, uint32_t size, long file_size)
{
    uint8_t *data;

    if (size == 0 || offset >= (uint32_t)file_size || size > SFO_MAX_SIZE)
        return NULL;
    if (offset > (uint32_t)file_size - size)
        size = (uint32_t)file_size - offset;
    data = malloc(size);
    if (data == NULL)
        return NULL;
    if (fseek(fh, (long)offset, SEEK_SET) != 0 || fread(data, 1, size, fh) != size) {
        free(data);
        return NULL;
    }
    return data;
}

// Parses a PARAM.SFO buffer, calls cb(key, value, value_len) for each entry.
// Returns 0 on success.
static int sfo_parse(const uint8_t *sfo, uint32_t size,
                     int (*cb)(const char *key, const char *value, uint32_t len, void *user), void *user)
{
    uint32_t key_start, data_start, count, i;

    if (size < SFO_HEADER_SIZE || memcmp(sfo, "\0PSF", 4) != 0)
        return -1;
    key_start = read_u32(sfo + 8);
    data_start = read_u32(sfo + 12);
    count = read_u32(sfo + 16);
    if (count > SFO_MAX_ENTRIES || key_start >= size || data_start > size || key_start < SFO_HEADER_SIZE)
        return -1;

    for (i = 0; i < count; i++) {
        const uint8_t *entry = sfo + SFO_HEADER_SIZE + i * SFO_ENTRY_SIZE;
        uint32_t key_off, data_len, data_off;
        uint16_t fmt;
        const char *key, *value;
        uint32_t remaining;

        if (SFO_HEADER_SIZE + (i + 1) * SFO_ENTRY_SIZE > size)
            return -1;
        key_off = read_u16(entry);
        fmt = read_u16(entry + 2);
        data_len = read_u32(entry + 4);
        data_off = read_u32(entry + 12);

        if (key_start + key_off >= size)
            continue;
        key = (const char *)sfo + key_start + key_off;
        remaining = size - (key_start + key_off);
        if (strnlen(key, remaining) == remaining)
            continue; // unterminated key

        if (fmt != 0x0204 && fmt != 0x0404)
            continue; // only string entries are of interest here
        if (data_off >= size || data_start > size - data_off)
            continue;
        if (data_start + data_off + data_len > size)
            data_len = size - data_start - data_off;
        value = (const char *)sfo + data_start + data_off;
        if (cb(key, value, data_len, user) != 0)
            return -1;
    }
    return 0;
}

static void sanitize(char *s, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\0') {
            s[i] = '\0';
            return;
        }
        if (c < 0x20 || c == 0x7f)
            s[i] = ' ';
    }
    s[len] = '\0';
}

static int print_entry(const char *key, const char *value, uint32_t len, void *user)
{
    char clean[512];
    uint32_t n = len < sizeof(clean) - 1 ? len : sizeof(clean) - 1;
    (void)user;
    if (strcmp(key, "TITLE") != 0 && strcmp(key, "DISC_ID") != 0 && strcmp(key, "TITLE_ID") != 0)
        return 0;
    memcpy(clean, value, n);
    clean[n] = '\0';
    sanitize(clean, n);
    if (clean[0] == '\0')
        return 0;
    printf("%s=%s\n", key, clean);
    return 0;
}

static int extract_image(const char *pbp_path, int index, const char *out_path)
{
    FILE *fh = fopen(pbp_path, "rb");
    uint8_t header[PBP_HEADER_SIZE];
    uint32_t offsets[8], start, size;
    long file_size;
    uint8_t *data;
    FILE *out;
    int ok = 0;

    if (fh == NULL)
        return 1;
    if (fseek(fh, 0, SEEK_END) != 0 || (file_size = ftell(fh)) < 0)
        goto fail;
    if (fseek(fh, 0, SEEK_SET) != 0 || fread(header, 1, PBP_HEADER_SIZE, fh) != PBP_HEADER_SIZE ||
        read_u32(header) != PBP_MAGIC)
        goto fail;

    for (int i = 0; i < 8; i++)
        offsets[i] = read_u32(header + 8 + i * 4);
    start = offsets[index];
    size = (index < 7 && offsets[index + 1] > start) ? offsets[index + 1] - start
                                                     : (uint32_t)file_size - start;
    if (start == 0 || size < PNG_SIG_SIZE)
        goto fail;
    data = read_section(fh, start, size, file_size);
    if (data == NULL)
        goto fail;
    if (memcmp(data, png_sig, PNG_SIG_SIZE) != 0) {
        free(data);
        goto fail;
    }

    out = fopen(out_path, "wb");
    if (out == NULL) {
        free(data);
        goto fail;
    }
    ok = fwrite(data, 1, size, out) == size;
    fclose(out);
    free(data);

fail:
    fclose(fh);
    return ok ? 0 : 1;
}

int main(int argc, char *argv[])
{
    FILE *fh;
    uint8_t header[PBP_HEADER_SIZE];
    uint32_t offsets[8], sfo_size;
    uint8_t *sfo;
    long file_size;

    if (argc == 4 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "-p") == 0))
        return extract_image(argv[2], argv[1][1] == 'i' ? 1 : 4, argv[3]);

    if (argc != 2) {
        fprintf(stderr, "usage: pbpinfo <file.pbp>\n"
                        "       pbpinfo -i <file.pbp> <out.png>  (extract ICON0.PNG)\n"
                        "       pbpinfo -p <file.pbp> <out.png>  (extract PIC1.PNG)\n");
        return 2;
    }

    fh = fopen(argv[1], "rb");
    if (fh == NULL) {
        fprintf(stderr, "pbpinfo: cannot open %s\n", argv[1]);
        return 1;
    }
    if (fseek(fh, 0, SEEK_END) != 0 || (file_size = ftell(fh)) < 0 ||
        fseek(fh, 0, SEEK_SET) != 0) {
        fclose(fh);
        return 1;
    }
    if (file_size < PBP_HEADER_SIZE || fread(header, 1, PBP_HEADER_SIZE, fh) != PBP_HEADER_SIZE ||
        read_u32(header) != PBP_MAGIC) {
        fprintf(stderr, "pbpinfo: %s is not a PBP file\n", argv[1]);
        fclose(fh);
        return 1;
    }

    for (int i = 0; i < 8; i++)
        offsets[i] = read_u32(header + 8 + i * 4);
    sfo_size = (offsets[0] && offsets[1] > offsets[0]) ? offsets[1] - offsets[0] : 0;
    sfo = sfo_size ? read_section(fh, offsets[0], sfo_size, file_size) : NULL;
    fclose(fh);
    if (sfo == NULL || sfo_parse(sfo, sfo_size, print_entry, NULL) != 0) {
        free(sfo);
        fprintf(stderr, "pbpinfo: no PARAM.SFO metadata in %s\n", argv[1]);
        return 1;
    }
    free(sfo);
    return 0;
}
