
#ifndef PDF_RECTANGLE_H__
#define PDF_RECTANGLE_H__

#include <phpcpp.h>

class PdfRectangle : public Php::Base {
private:
    int64_t x = 0;
    int64_t y = 0;
    int64_t width = 0;
    int64_t height = 0;
    int red = 0;
    int green = 0;
    int blue = 0;
public:
    PdfRectangle();
    PdfRectangle(const PdfRectangle &obj);
    void __construct(Php::Parameters &params);
    Php::Value getX();
    Php::Value getY();
    Php::Value getWidth();
    Php::Value getHeight();
    Php::Value getRed();
    Php::Value getGreen();
    Php::Value getBlue();

    unsigned long getColor();
};

#endif /* PDF_RECTANGLE_H__ */
