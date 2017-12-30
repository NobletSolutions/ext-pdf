<?php
if (!class_exists('\Pdf\PdfText')) {
    echo "PdfText doesn't exist\n";
    return;
}

if (class_exists('\Pdf\PdfWriter')) {
    echo "Writer exists\n";
    $pdfWriter = new \Pdf\PdfWriter('./File.pdf', './Output.pdf');
    echo "Set font...\n";
    $pdfWriter->setFont("arial");
    echo "Font set\n";
    $pdfWriter->writeTextToPage(0, [ new \Pdf\PdfText(78.5,640,"The text to write"),new \Pdf\PdfText(78.5,540,"More text to write"), new \Pdf\PdfText(78.5,440,"Gnat defaced this PDF!")]);
    $pdfWriter->writeTextToPage(1, [ new \Pdf\PdfText(98.5,640,"The text to write"),new \Pdf\PdfText(98.5,540,"More text to write"), new \Pdf\PdfText(98.5,440,"Gnat defaced this PDF!")]);
    $pdfWriter->writeTextToPage(0, [new \Pdf\PdfText(98.5, 640, "20 - The text to write", 20, "DejaVuSansMono-BoldOblique"), new \Pdf\PdfText(98.5, 540, "9 - More text to write", 8), new \Pdf\PdfText(98.5, 440, "30-Gnat defaced this PDF!", 30)]);
    echo "Wrote text\n";
    $pdfWriter->save();
    echo "Saved\n";

} else {
    echo "Class doesn't exist\n";
}

if (class_exists('\Pdf\PdfDocument')) {
    //$pdfDocument = new \Pdf\PdfDocument('./File.pdf');
    $pdfDoc = new \Pdf\PdfDocument('./File.pdf', 'user', 'password');

    echo "PDF Version: " . $pdfDoc->getMajorVersion() . '.' . $pdfDoc->getMinorVersion() . "\n";
    echo "Is Encrypted: " . (($pdfDoc->isEncrypted() === true) ? 'Yes' : 'No') . "\n";
    echo "Is Linear: " . (($pdfDoc->isLinear() === true) ? 'Yes' : 'No') . "\n";
    echo "NumPages: " . $pdfDoc->getNumberOfPages() . "\n";

    $txt = $pdfDoc->asString();

    echo "TXT\n$txt\n\n";

    echo "ORD: " . ord($txt) . "\n";
    file_put_contents('./Output.txt', $pdfDoc->asString());
    $path = "/home/gnat/Projects/Web/prolegis/web/Output";
    echo "Outputting as jpg...\n";
    $res = $pdfDoc->toImage(\Pdf\PdfDocument::IMAGE_JPEG, $path, 300);
    if ($res === false) {
        echo "FAILED\n";
    } else {
        echo "Image: " . $res->getImageWidth() . " x " . $res->getImageHeight() . "\n";
        echo "Page: " . $res->getPageWidth() . " x " . $res->getPageHeight() . "\n";
        echo "NumPages: " . $res->getNumberOfPages() . "\n";
        echo "Pages: " . print_r($res->getPages(), true) . "\n";
    }

    echo ".. done\n";
} else {
    echo "Class doesn't exist\n";
}

