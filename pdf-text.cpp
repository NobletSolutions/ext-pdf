/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <string.h>
#include "pdf-text.h"

PdfText::PdfText() = default;

void PdfText::__construct(Php::Parameters &params) {
    x = params[0].numericValue();
    y = params[1].numericValue();
    text = params[2].stringValue();

    if (params.size() >= 4) {
        fontSize = params[3].numericValue();
    }

    if (params.size() == 5) {
        font = params[4].stringValue();
    }
}

Php::Value PdfText::getX() {
    return x;
}

Php::Value PdfText::getY() {
    return y;
}

Php::Value PdfText::getText() {
    return text;
}

Php::Value PdfText::getFontSize() {
    return fontSize;
}

Php::Value PdfText::getFont() {
    return font;
}

