<?php
if (!class_exists('\Pdf\PdfText')) {
    echo "PdfText doesn't exist\n";
    return;
}

if (class_exists('\Pdf\PdfWriter')) {
    $pdfWriter = new \Pdf\PdfWriter('./File.pdf','./Output.pdf');

    echo "Class exists!\n";
	$pdfWriter->writeTextToPage(0, [ new \Pdf\PdfText(78.5,640,"The text to write"),new \Pdf\PdfText(78.5,540,"More text to write"), new \Pdf\PdfText(78.5,440,"Gnat defaced this PDF!")]);
	$pdfWriter->writeTextToPage(1, [ new \Pdf\PdfText(98.5,640,"The text to write"),new \Pdf\PdfText(98.5,540,"More text to write"), new \Pdf\PdfText(98.5,440,"Gnat defaced this PDF!")]);
    $pdfWriter->save();

} else {
    echo "Class doesn't exist\n";
}

