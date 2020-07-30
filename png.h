#ifndef PNG_H
#define PNG_H
extern "C" {
    #include "image.h"
}

#include <QWidget>
class QPaintEvent;

class PNGviewer : public QWidget
{
    struct paint_info draw;
public:
    PNGviewer(const char *,struct paint_info );
    void paintEvent(QPaintEvent*);
    int start_x = 960;
    int start_y = 500;
};

#endif