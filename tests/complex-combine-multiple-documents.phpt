--TEST--
PdfWriter::complexCombine() repeats selected pages and appends a second PDF in order
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$first = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$second = $first . '-appendix.pdf';
$output = $first . '-combined.pdf';

try {
    writeThreePagePdf($first);
    writeThreePagePdf($second, 'Appendix');

    $cases = [
        [new Pdf\PdfCopy($first, 4, '1'), new Pdf\PdfCopy($second, 1, '1-3')],
        [new Pdf\PdfCopy($first)],
    ];
    foreach ($cases as $copies) {
        if (!Pdf\PdfWriter::complexCombine($output, $copies)) {
            throw new RuntimeException('Unable to combine the PDFs');
        }

        $document = new Pdf\PdfDocument($output);
        printf("%d pages\n", $document->getNumberOfPages());
        preg_match_all('/\b(?:Page|Appendix) [A-Za-z]+\b/', $document->asString(), $matches);
        echo implode("\n", $matches[0]), "\n";
    }
} finally {
    foreach ([$first, $second, $output] as $file) {
        if (file_exists($file)) {
            unlink($file);
        }
    }
}
?>
--EXPECT--
7 pages
Page One
Page One
Page One
Page One
Appendix One
Appendix Two
Appendix Three
3 pages
Page One
Page Two
Page Three
