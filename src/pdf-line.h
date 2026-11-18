
#ifndef PDF_LINE_H__
#define PDF_LINE_H__

#include <phpcpp.h>

class PdfLine : public Php::Base {
private:
    int64_t x = 0;
    int64_t y = 0;
    int64_t x2 = 0;
    int64_t y2 = 0;

    int width = 1;

    int red = 0;
    int green = 0;
    int blue = 0;
public:
    PdfLine();
    PdfLine(const PdfLine &obj);
    void __construct(Php::Parameters &params);
    Php::Value getX();
    Php::Value getY();
    Php::Value getX2();
    Php::Value getY2();

    void setWidth(Php::Parameters &params);
    Php::Value getWidth();

    Php::Value getRed();
    Php::Value getGreen();
    Php::Value getBlue();

    unsigned long getColor();
};

#endif /* PDF_LINE_H__ */
