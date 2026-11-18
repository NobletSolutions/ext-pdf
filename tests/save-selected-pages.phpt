--TEST--
PdfWriter::save() writes selected pages in the requested order
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$output = $input . '-output.pdf';

try {
    writeThreePagePdf($input);
    $cases = [
        [false, null],
        [false, []],
        [false, ['2-3']],
        [false, ['3', '1-2']],
        [false, ['2', '2-3']],
        [true, ['3']],
    ];
    foreach ($cases as [$flatten, $ranges]) {
        $writer = new Pdf\PdfWriter($input, $output, $flatten);
        if ($ranges === null) {
            $writer->save();
        } else {
            $writer->save($ranges);
        }
        $document = new Pdf\PdfDocument($output);
        preg_match_all('/Page (One|Two|Three)/', $document->asString(), $matches);
        printf("%d pages: %s\n", $document->getNumberOfPages(), implode(', ', $matches[1]));
    }
} finally {
    unlink($input);
    if (file_exists($output)) {
        unlink($output);
    }
}
?>
--EXPECT--
3 pages: One, Two, Three
3 pages: One, Two, Three
2 pages: Two, Three
3 pages: Three, One, Two
3 pages: Two, Two, Three
1 pages: Three
