#include <iostream>
#include <QApplication>
#include <QPalette>
#include <QPainter>
#include <QImage>
#include "png.h"

extern struct paint_info paint_info;

extern "C"
{
#include "image.h"
    struct paint_info nomain(int, char **);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    PNGviewer *Canvas = new PNGviewer{"PNG_viewer", nomain(argc, argv)};
    QPalette pal;
    QPalette palette;
    palette.setColor(QPalette::Window, Qt::gray);
    Canvas->setPalette(palette);
    Canvas->setAutoFillBackground(true);
    Canvas->update();
    Canvas->showMaximized();
    return app.exec();
}

PNGviewer::PNGviewer(const char *title, struct paint_info info) : QWidget(NULL)
{
    setWindowTitle(title);
    draw = info;
    start_x = 960 - info.image_width / 2;
    start_y = 540 - info.image_height / 2;
    unsigned char *final_data = info.final_data;
    int old = 1;
    QImage myimage{draw.image_width, draw.image_height, QImage::Format_RGB32};
    for (int i = 0; i < info.count; i += 4)
    {
        printf("%02x %02x %02x %02x    ", final_data[i], final_data[i + 1], final_data[i + 2],
               final_data[i + 3]);
        if (old++ % 4 == 0)
            printf("\n");
    }
}

void PNGviewer::paintEvent(QPaintEvent *event)
{
    QImage myimage;
    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing, true);
    std::cout << "Reached here." << std::endl;

    int height = 0, length = 0;
    int r, g, b, a;
    printf("Value of count is : %d.", draw.count);
    int counter = 0;
    int increment;
    if (draw.image_color_type == 2)
    {
        myimage = QImage{draw.image_width, draw.image_height, QImage::Format_RGB32};
        for (int i = 0; i < draw.count; i += 3)
        {
            r = draw.final_data[i];
            g = draw.final_data[i + 1];
            b = draw.final_data[i + 2];
            // a = draw.final_data[i + 3];
            // std::cout << "r -> " << r << " g -> " << g << " b -> "
            //           << " a -> " << a << std::endl;
            // painter.setBrush(QBrush(QColor{r, g, b, a}));
            // painter.drawEllipse(QPoint{start_x + length, start_y + height}, 1, 1);
            myimage.setPixel(length, height, qRgb(r, g, b));

            length++;
            counter++;
            if (counter % (draw.image_width) == 0)
            {
                height++;
                length = 0;
            }
        }
    }
    else
    {
        myimage = QImage{draw.image_width,draw.image_height,QImage::Format_ARGB32};
        for (int i = 0; i < draw.count; i += 4)
        {
            r = draw.final_data[i];
            g = draw.final_data[i+1];
            b = draw.final_data[i+2];
            a = draw.final_data[i+3];
            myimage.setPixel(length,height,qRgba(r,g,b,a));

            length++;
            counter++;
            if (counter % (draw.image_width) == 0)
            {
                height++;
                length = 0;
            }
        }
    }
    painter.drawImage(start_x, start_y, myimage);
}