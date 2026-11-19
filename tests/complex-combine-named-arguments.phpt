--TEST--
PdfWriter::complexCombine() advertises destination first and accepts named arguments
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$method = new ReflectionMethod(Pdf\PdfWriter::class, 'complexCombine');
foreach ($method->getParameters() as $parameter) {
    printf("%s %s\n", $parameter->getType(), $parameter->getName());
}

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$output = $input . '-combined.pdf';
try {
    writeThreePagePdf($input);
    if (!Pdf\PdfWriter::complexCombine(
        documents: [new Pdf\PdfCopy($input, 1, '2')],
        destination: $output,
    )) {
        throw new RuntimeException('Unable to combine the selected page');
    }
    $document = new Pdf\PdfDocument($output);
    printf("%d page\n", $document->getNumberOfPages());
    echo trim($document->asString(), " \t\r\n\x0c"), "\n";
} finally {
    unlink($input);
    if (is_file($output)) {
        unlink($output);
    }
}
?>
--EXPECT--
string destination
array documents
1 page
Page Two
