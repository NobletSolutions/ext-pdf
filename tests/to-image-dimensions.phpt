--TEST--
PdfDocument::toImage() reports image pixels and PDF page points separately
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$outputDirectory = $input . '-images';

try {
    writeThreePagePdf($input);
    $document = new Pdf\PdfDocument($input);
    $images = $document->toImage(Pdf\PdfDocument::IMAGE_PNG, $outputDirectory . '/page', 144);

    printf("%d images\n", count($images));
    foreach ($images as $image) {
        [$width, $height] = getimagesize($image->getPage());
        printf("Image %dx%d px, PNG %dx%d px, page %dx%d pt\n",
            $image->getImageWidth(), $image->getImageHeight(), $width, $height,
            $image->getPageWidth(), $image->getPageHeight());
    }
} finally {
    unlink($input);
    if (is_dir($outputDirectory)) {
        foreach (glob($outputDirectory . '/*.png') as $file) {
            unlink($file);
        }
        rmdir($outputDirectory);
    }
}
?>
--EXPECT--
3 images
Image 288x432 px, PNG 288x432 px, page 144x216 pt
Image 288x432 px, PNG 288x432 px, page 144x216 pt
Image 288x432 px, PNG 288x432 px, page 144x216 pt
