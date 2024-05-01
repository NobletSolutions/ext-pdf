/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <string.h>
#include <iostream>
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

PdfText::PdfText(const PdfText &obj) {
	x = obj.x;
	y = obj.y;
	text = obj.text;
    color = obj.color;

	if (obj.fontSize) {
		fontSize = obj.fontSize;
	}

	if (!obj.font.empty()) {
		font = obj.font;
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

Php::Value PdfText::__toString() {
    return getText();
}

Php::Value PdfText::getFontSize() {
    return fontSize;
}

Php::Value PdfText::getFont() {
    return font;
}

Php::Value PdfText::getColor() {
    return color;
}

void PdfText::setColor(Php::Parameters &params) {
    if (params.size() == 1) {
        color = params[0].numericValue();
        return;
    }

    if (params.size() == 3) {
        color =  65536*params[0].numericValue() + 256*params[1].numericValue() + params[2].numericValue();
        return;
    }

    Php::warning << "Invalid color: Pass a color as a single value or RGB" << std::flush;
    throw Php::Exception("Invalid color: Pass a color as a single value or RGB");
}