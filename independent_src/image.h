#ifndef IMAGE_H
#define IMAGE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "image.h"

struct paint_info;

 // Forward declaration
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


struct paint_info
{
    unsigned char* final_data;
    unsigned  count;
    unsigned image_width;
    unsigned image_height;
    unsigned image_color_type;
    unsigned image_bit_depth;
};
#endif