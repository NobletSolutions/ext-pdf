--TEST--
PdfWriter::complexCombine() copies every page in a final PdfCopy range in order
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$output = $input . '-output.pdf';

try {
    writeThreePagePdf($input);
    foreach (['2-3', '1, 2-3', '2-3, 1'] as $pages) {
        if (!Pdf\PdfWriter::complexCombine($output, [new Pdf\PdfCopy($input, 1, $pages)])) {
            throw new RuntimeException('Unable to combine the selected pages');
        }
        $document = new Pdf\PdfDocument($output);
        preg_match_all('/Page (One|Two|Three)/', $document->asString(), $matches);
        printf("%s -> %d pages: %s\n", $pages, $document->getNumberOfPages(), implode(', ', $matches[1]));
    }

    try {
        Pdf\PdfWriter::complexCombine($output, [new Pdf\PdfCopy($input, 1, '2-4')]);
        echo "Accepted a page outside the PDF\n";
    } catch (Exception $exception) {
        echo $exception->getMessage(), "\n";
    }
} finally {
    unlink($input);
    if (file_exists($output)) {
        unlink($output);
    }
}
?>
--EXPECT--
2-3 -> 2 pages: Two, Three
1, 2-3 -> 3 pages: One, Two, Three
2-3, 1 -> 3 pages: Two, Three, One
Selected page is outside the PDF
