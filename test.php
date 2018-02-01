<?php
if (!class_exists('\Pdf\PdfText')) {
    echo "PdfText doesn't exist\n";
    return;
}
echo "Fonts: ".print_r(\Pdf\getFonts(),true)."\n";

/*
if (class_exists('\Pdf\PdfWriter')) {
    echo "Writer exists\n";
    $pdfWriter = new \Pdf\PdfWriter('./File.pdf', './Output.pdf');
    echo "Set font...\n";
	$pdfWriter->setFont("Arial");
    echo "Font set\n";
    echo "All Fonts: ".print_r($pdfWriter->getAllFonts(),true)."\n";
    $pdfWriter->writeTextToPage(0, [
    	new \Pdf\PdfText(88.5,640,"The text to write\nTwo lines"),
    	new \Pdf\PdfText(88.5,540,"PLAN 7711449
BLOCK 8
LOT 39
EXCEPTING THEREOUT ALL MINES AND MINERALS"), 
    	new \Pdf\PdfText(88.5,440,"Gnat defaced this PDF!")
	]);
    
    $pdfWriter->writeTextToPage(1, [
    	new \Pdf\PdfText(98.5,640,"The text to write"),
    	new \Pdf\PdfText(98.5,540,"More text to write"),
    	new \Pdf\PdfText(98.5,440,"Gnat defaced this PDF!")
	]);

//    $pdfWriter->writeTextToPage(0, [
//    	new \Pdf\PdfText(98.5, 640, "20 - The text to write - Archivo", 20, "Archivo"), 
//    	new \Pdf\PdfText(98.5, 540, "9 - More text to write - Cormorant", 8,"Cormorant Garamond"), 
//    	new \Pdf\PdfText(98.5, 440, "30-Gnat defaced this PDF! - Faustina", 30,"Faustina")
//	]);

    echo "Wrote text\n";
    $pdfWriter->save();
    echo "Saved\n";

} else {
    echo "Class doesn't exist\n";
}
*/
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
    $path = getcwd()."/Output";

//    $path = "/home/gnat/Projects/Web/prolegis/web/Output";

    echo "Outputting as jpg...\n";
    $res = $pdfDoc->toImage(\Pdf\PdfDocument::IMAGE_JPEG, $path, 300);
    echo "Done...\n";
    if ($res === false) {
        echo "FAILED\n";
    } else {
       foreach($res as $pageNum => $pdfImageRest) {
            echo "Page: $pageNum\n";
            echo "\tImage: " . $pdfImageRest->getImageWidth() . " x " . $pdfImageRest->getImageHeight() . "\n";
            echo "\tPage: " . $pdfImageRest->getPageWidth() . " x " . $pdfImageRest->getPageHeight() . "\n";
            echo "\tPage: " . $pdfImageRest->getPage() . "\n";
       }
    }

    echo ".. done\n";
} else {
    echo "Class doesn't exist\n";
}

