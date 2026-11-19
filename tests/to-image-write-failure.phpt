--TEST--
PdfDocument::toImage() throws when it cannot write an image
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$outputDir = $input . '-images';
$prefix = $outputDir . '/page';
$blockedPath = $prefix . '-0.png';

try {
    writeThreePagePdf($input);
    mkdir($blockedPath, 0700, true);
    $document = new Pdf\PdfDocument($input);

    try {
        $document->toImage(Pdf\PdfDocument::IMAGE_PNG, $prefix);
        echo "Unexpected success\n";
    } catch (Exception $exception) {
        echo str_replace($outputDir, 'output', $exception->getMessage()), "\n";
    }
    var_dump(file_exists($prefix . '-1.png'));

    rmdir($blockedPath);
    $images = $document->toImage(Pdf\PdfDocument::IMAGE_PNG, $prefix);
    printf("Retry wrote %d images\n", count($images));
} finally {
    unlink($input);
    if (is_dir($blockedPath)) {
        rmdir($blockedPath);
    }
    if (is_dir($outputDir)) {
        foreach (glob($outputDir . '/*.png') as $image) {
            unlink($image);
        }
        rmdir($outputDir);
    }
}
?>
--EXPECT--
Unable to write image: output/page-0.png
bool(false)
Retry wrote 3 images
