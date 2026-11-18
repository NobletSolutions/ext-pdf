--TEST--
PDF JPEG previews expose absolute and relative paths and reject an unrelated root
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$root = $input . '-output';
$previewDir = $root . '/previews';

try {
    writeThreePagePdf($input);
    $document = new Pdf\PdfDocument($input);
    $images = $document->toImage(Pdf\PdfDocument::IMAGE_JPEG, $previewDir . '/page', 300);

    printf("%d previews\n", count($images));
    foreach ($images as $index => $image) {
        $absolutePath = $image->getPage();
        if ($absolutePath !== "$previewDir/page-$index.jpg") {
            throw new RuntimeException("Unexpected preview path: $absolutePath");
        }
        $imageInfo = getimagesize($absolutePath);
        if ($imageInfo === false || $imageInfo[2] !== IMAGETYPE_JPEG) {
            throw new RuntimeException("Expected a readable JPEG at $absolutePath");
        }
        $relativePath = $image->getRelativePage($root . '/');
        if ($image->getRelativePage($root) !== '/' . $relativePath) {
            throw new RuntimeException('Unexpected path for root without a trailing slash');
        }
        printf("%s %dx%d px\n", $relativePath, $imageInfo[0], $imageInfo[1]);
    }

    try {
        $images[0]->getRelativePage($input . '-unrelated/');
        echo "Unrelated root accepted\n";
    } catch (Exception $exception) {
        echo "Unrelated root rejected\n";
    }
} finally {
    unlink($input);
    if (is_dir($previewDir)) {
        foreach (glob($previewDir . '/*.jpg') as $file) {
            unlink($file);
        }
        rmdir($previewDir);
    }
    if (is_dir($root)) {
        rmdir($root);
    }
}
?>
--EXPECT--
3 previews
previews/page-0.jpg 600x900 px
previews/page-1.jpg 600x900 px
previews/page-2.jpg 600x900 px
Unrelated root rejected
