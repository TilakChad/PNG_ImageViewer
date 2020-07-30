#ifndef IMAGE_H
#define IMAGE_H
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