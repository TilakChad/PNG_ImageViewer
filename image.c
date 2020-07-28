#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "image.h"

typedef void (*handle_ptr)(unsigned char *, int);

uint32_t adler32_checksum(unsigned char *, int);

struct idat_data
{
    unsigned char *data;
    unsigned len;
    unsigned char *outdata;
} image;

struct image_info
{
    unsigned int height;
    unsigned int width;
    int color_type;
    int interlaced;
    int bit_depth;
    int filter_type;
} image_info;

struct handler
{
    const char *type;
    handle_ptr func;
};

struct paint_info paint_data;
struct paint_info nomain(int argc, char **argv);

void idat_data();
void time_handler(unsigned char *, int);
void header_handler(unsigned char *, int);
void background_handler(unsigned char *, int);
void pixelXY_handler(unsigned char *, int);
void utftxt_handler(unsigned char *, int);
void palette_generator(unsigned char *, int);
void image_data(unsigned char *, int);
void gama_handler(unsigned char *, int);
void reverse_filter();
unsigned char average(unsigned char, unsigned char);
unsigned char paethPredictor(unsigned char, unsigned char, unsigned char);
uint32_t CRC_check(unsigned char *, int);

extern int deflate(unsigned char *, unsigned, unsigned char *, unsigned *);
void validate(int val, const char *msg);
void check_header(const char *buf);
unsigned int getBigEndian(char *);

#define MAX_SIZE (1 * 1024 * 1024)
struct paint_info nomain(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments to the image : Exiting\n");
        exit(EXIT_FAILURE);
    }
    image.data = (unsigned char *)malloc((MAX_SIZE));
    image.len = 0;
    FILE *image_file;
    if (!(image_file = fopen(argv[1], "rb")))
    {
        printf("Failed to open %s.. Exiting..", argv[1]);
        exit(1);
    }

    const struct handler handlers[] = {
        {"IHDR", header_handler},
        {"bKGD", background_handler},
        {"tIME", time_handler},
        {"pHYs", pixelXY_handler},
        {"iTXt", utftxt_handler},
        {"PLTE", palette_generator}, // Remember the memory address error here.. Yes sir
        {"IDAT", image_data},
        {"gAMA", gama_handler},
        {NULL, NULL}};

    char *buf = (char *)malloc(sizeof(char) * MAX_SIZE);
    int size = fread(buf, 1, MAX_SIZE, image_file);
    for (int i = 0; i < size; ++i)
    {
        // printf("%02x   ", (unsigned char)buf[i]);
        // if ((i + 1) % 30 == 0)
        //     putchar('\n');
    }
    check_header(buf);

    // Read the 4 byte length -> PNG use big-endian ordering while our system is little-endian

    int i = 0;
    int pos = 8;
    while (1)
    {
        unsigned char lengthbuf[4];
        memcpy(lengthbuf, buf + pos, 4);
        unsigned int len = getBigEndian(lengthbuf);
        unsigned char chunkbuf[5] = {'\0'};
        memcpy(chunkbuf, buf + pos + 4, 4);
        printf("chunk : %s & len : %u(%u).\n", chunkbuf, len, size - (len + pos + 12));
        for (int i = 0; handlers[i].type != NULL; ++i)
        {
            if (!strcmp(handlers[i].type, chunkbuf))
            {
                handlers[i].func(buf + pos + 8, len);
                break;
            }
        }
        if (CRC_check(buf + pos + 4, len + 4) == getBigEndian(buf + pos + 8 + len))
            printf("\tCRC check :- 1\n");
        else
        {
            printf("\tCRC check :- -1\n");
        }
        pos += len + 12;
        if (!strcmp(chunkbuf, "IEND"))
            break;
    }
    idat_data();
    reverse_filter();
    fclose(image_file);
    free(buf);
    free(image.outdata);
    paint_data.image_color_type = image_info.color_type;
    paint_data.image_bit_depth = image_info.bit_depth;
    paint_data.image_height = image_info.height;
    paint_data.image_width = image_info.width;
    return paint_data;
}

void check_header(const char *buf)
{
    validate((unsigned char)buf[0] == 0x89, "header byte 1");
    validate((unsigned char)buf[1] == 'P', "header byte 2");
}

void validate(int val, const char *msg)
{
    if (!val)
    {
        fprintf(stderr, "Invalid file : Corrupted %s", msg);
        exit(EXIT_FAILURE);
    }
}

unsigned int getBigEndian(char *lenbuf)
{
    unsigned int v = 0, temp = 0;
    unsigned char ch;
    int k = 24;
    for (int i = 0; i < 4; ++i)
    {
        ch = lenbuf[i];
        temp = ch << k;
        k = k - 8;
        v = v | temp;
    }
    return v;
}

void header_handler(unsigned char *buffer, int len)
{
    validate(len == 13, "Header must be 13 bytes long.");
    printf("\tWidth  : %u.\n", image_info.width = getBigEndian(buffer));
    printf("\tHeight : %u.\n", image_info.height = getBigEndian(buffer + 4));
    printf("\tBit depth : %d.\n", image_info.bit_depth = buffer[8]);
    printf("\tColor type : %d.\n", image_info.color_type = *((unsigned char *)(buffer + 9)));
    printf("\tCompression method : %d.\n", (unsigned char)buffer[10]);
    printf("\tFilter method : %d.\n", image_info.filter_type = buffer[11]);
    printf("\tInterlaced method : %d.\n", image_info.interlaced = buffer[12]);
}

void background_handler(unsigned char *buffer, int len)
{
    printf("Background Color info :  \n");
    int r = 0, g = 0, b = 0;
    r |= (buffer[0] << 8);
    r |= buffer[1];
    g |= buffer[2] << 8;
    g |= buffer[3];
    b |= buffer[4] << 8;
    b |= buffer[5];
    printf("\tRed color is : %02x.\n", r);
    printf("\tGreen color is : %02x.\n", g);
    printf("\tBlue color is %02x.\n", b);
}

void time_handler(unsigned char *buffer, int len)
{
    printf("\tYear   : %u.\n", ((int)0 | (buffer[0] << 8) | (buffer[1])));
    printf("\tMonth  : %u.\n", buffer[2]);
    printf("\tDay    : %u.\n", buffer[3]);
    printf("\tHour   : %u.\n", buffer[4]);
    printf("\tMinute : %u.\n", buffer[5]);
    printf("\tSecond : %u.\n", buffer[6]);
}

void pixelXY_handler(unsigned char *buffer, int len)
{
    printf("\tPixel per unit, X axis : %u.\n", getBigEndian(buffer));
    printf("\tPixel per unit, Y axis : %u.\n", getBigEndian(buffer + 4));
    printf("\tUnit specifier : %u.\n", (unsigned char)*(buffer + 8));
}

void utftxt_handler(unsigned char *buffer, int len)
{
}

void palette_generator(unsigned char *buffer, int len)
{
    for (int i = 0; i < len; i += 3)
    {
        printf("%02x %02x %02x   ", (unsigned char)buffer[i], (unsigned char)buffer[i + 1], (unsigned char)buffer[i + 2]);
    }
}

void image_data(unsigned char *buffer, int len)
{
    putchar('\n');
    // for (int i = 0; i < len; ++i)
    // {
    //     printf("%02x  ", buffer[i]);
    //     if ((i + 1) % 25 == 0)
    //         putchar('\n');
    // }
    // // printf("\tCompression method used is : %d.\n", (*buffer & 0x0F));
    // printf("\tCompression info is : %d.\n", ((*buffer) >> 4 & 0xFF));
    // unsigned char *adlerbuffer = buffer;

    // buffer++;
    // printf("\tFDICT bit is : %d.", (*buffer >> 5) & 0x01);
    // printf("\tCompression level used is : %d.", (*buffer >> 6) & 0x03);
    // unsigned char output[100000];
    // unsigned outlen = 100000;
    // for (int i = 0; i < outlen; ++i)
    // {
    //     output[i] = '\0';
    // }

    // unsigned long inlen = len - 4;
    // outlen = 100000;

    // Concatenate all idat data
    memcpy(image.data + image.len, buffer, len);
    image.len += len;

    // buffer++;
    // deflate(buffer,len,output,&outlen);
    // int k = 0;
    // printf("\n");
    // printf("Total uncompressed size is : %d.",outlen);
    // while(k<1000)
    // {
    //     printf("%02x   ",(unsigned char) output[k++]);
    //     if(k%25==0)
    //         putchar('\n');
    // }

    // unsigned char output2[100000] = {'\0'};
    // printf("\n\n");
    // unsigned long outlen = 100000;

    // if (puff(output2, &outlen, buffer, &inlen) > 0)
    //     printf("Error in decoding");
    // else
    // {
    //     printf("NO error");
    // }
    // printf("\nOutlen is : %d.\n",outlen);
    // uint32_t adler32 = adler32_checksum(output, outlen);
    // printf("Alder32 is : %x",adler32_checksum(output,outlen));

    // uint32_t stored_adler32 = adlerbuffer[len-4] << 24 | adlerbuffer[len-3] << 16 | adlerbuffer[len-2] << 8 | adlerbuffer[len-1];
    // if(adler32 == stored_adler32)
    // {
    //     printf("\nAdler32 checksum passed.\n");
    // }
    // else
    // {
    //     printf("\nError.. Adler 32 checksum failed.\n");
    // }
}

void gama_handler(unsigned char *buffer, int len)
{
    unsigned int gama_value = getBigEndian(buffer);
    printf("\tGama value of this image is : %u and actual data is : %f.\n", gama_value, (float)gama_value / 100000);
}

uint32_t CRC_check(unsigned char *buf, int len)
{
    const uint32_t POLY = 0xEDB88320;
    const unsigned char *buffer = (const unsigned char *)buf;
    uint32_t crc = -1;

    while (len--)
    {
        crc = crc ^ *buffer++;
        for (int bit = 0; bit < 8; bit++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ POLY;
            else
                crc = (crc >> 1);
        }
    }
    return ~crc;
}

uint32_t adler32_checksum(unsigned char *buffer, int len)
{
    const uint32_t adler_mod = 65521; // samllest prime less than 2^16-1
    uint16_t a = 1;
    uint16_t b = 0;
    for (int i = 0; i < len; ++i)
    {
        a = (a + *(buffer + i)) % adler_mod;
        b = (b + a) % adler_mod;
    }
    return (b << 16) | a;
}

void idat_data(void)
{
    // for(int i = 0; i < image.len; ++i)
    // {
    //     printf("%02x  ",image.data[i]);
    //     if((i+1)%25==0)
    //         putchar('\n');
    // }

    printf("\tCompression method used is : %d.\n", (*image.data & 0x0F));
    printf("\tCompression info is : %d.\n", ((*image.data) >> 4 & 0xFF));
    unsigned char *adlerbuffer = image.data;

    image.data++;
    printf("\tFDICT bit is : %d.", (*image.data >> 5) & 0x01);
    printf("\tCompression level used is : %d.", (*image.data >> 6) & 0x03);

    unsigned outlen = 1000000;
    unsigned char *deflate_out = malloc(sizeof(char) * outlen);
    image.data++;
    deflate(image.data, image.len, deflate_out, &outlen);

    uint32_t adler32 = adler32_checksum(deflate_out, outlen);
    printf("Alder32 is : %x", adler32_checksum(deflate_out, outlen));

    printf("\nFile size is %d bytes.\n", outlen);
    uint32_t stored_adler32 = adlerbuffer[image.len - 4] << 24 | adlerbuffer[image.len - 3] << 16 | adlerbuffer[image.len - 2] << 8 | adlerbuffer[image.len - 1];
    if (adler32 == stored_adler32)
        printf("\nAdler32 checksum passed.\n");
    else
        printf("\nError.. Adler 32 checksum failed.\n");
    image.outdata = deflate_out;
    // for (int i = 0; i < outlen; ++i)
    // {
    //     printf("%02x  ", image.outdata[i]);
    //     if ((i + 1) % (32 * 4 + 1) == 0)
    //         putchar('\n');
    // }
}

void reverse_filter(void)
{
    // Demo for only color_type 6 i.e true color with alpha channel
    assert(image_info.color_type == 6);

    unsigned char *prev_scanline = malloc(sizeof(char) * image_info.width * 4);
    for (int i = 0; i < image_info.width; ++i)
        prev_scanline[i] = 0;

    unsigned char *final_data = malloc(sizeof(char) * image_info.width * image_info.height * 4);
    unsigned char *original_data = final_data;

    unsigned char *filter_data = image.outdata;

    int byteDepth = 4; // For true color with alpha channel
    unsigned char a = 0, b = 0, c = 0, x;
    int pos = 0;
    int line_byte = image_info.width * byteDepth;
    unsigned char ch;

    int count = 0;
    // for(int i = 0; i < 80; ++i)
    // {
    //     printf("%02x  ",filter_data[i]);
    //     if((i+1)%20==0)
    //         putchar('\n');
    // }
    for (int height = 0; height < image_info.height; ++height)
    {
        // Read the first byte to know filter type.
        ch = *filter_data;
        pos = 1;
        // printf("\nValue of ch is : %d.\n\n", ch);
        a = b = c = x = 0;
        for (int seek = 0; seek < line_byte; ++seek)
        {
            count++;
            if (pos <= 4)
            {
                c = 0;
                a = 0;
            }
            else
            {
                c = prev_scanline[pos - 5];
                a = *(original_data - 4);
            }
            b = prev_scanline[pos - 1];
            x = filter_data[pos];
            if (ch == 0)
            {
                x = x;
            }
            else if (ch == 1)
            {
                x = x + a;
                // printf("Value of a is : %0x .and x is %0x.   ",a,x);
            }
            else if (ch == 2)
            {
                x = x + b;
            }
            else if (ch == 3)
            {
                x = x + average(a, b);
            }
            else if (ch == 4)
            {
                x = x + paethPredictor(a, b, c);
            }
            else
            {
                printf("Invalid filter type");
                exit(1);
            }
            pos++;
            *(original_data++) = x;
        }
        memcpy(prev_scanline, original_data - line_byte, line_byte);
        filter_data += line_byte + 1;
    }
    printf("\n\nTotal count is : %d.\n", count);
    // int old = 1;
    // for (int i = 0; i < count; i += 4)
    // {
    //     printf("%02x %02x %02x %02x    ", final_data[i], final_data[i + 1], final_data[i + 2],
    //            final_data[i + 3]);
    //     if (old++ % 4 == 0)
    //         printf("\n");
    // }
    paint_data.final_data = final_data;
    paint_data.count = count;
    free(prev_scanline);
}

unsigned char average(unsigned char a, unsigned char b)
{
    int c = a + b;
    return c / 2;
}

unsigned char paethPredictor(unsigned char a, unsigned char b, unsigned char c)
{
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    char pr;

    if (pa <= pb && pa <= pc)
        pr = a;
    else if (pb <= pc)
        pr = b;
    else
        pr = c;
    return pr;
}
