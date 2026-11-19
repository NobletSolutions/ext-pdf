--TEST--
PDF hashing handles long Info objects and reports invalid metadata references and input paths
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$path = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
try {
    writeThreePagePdf($path, 'Page', true, str_repeat('x', 5000));
    $bytes = file_get_contents($path);
    preg_match('/^\d+ 0 obj\n<< \/Creator \([^\n]*\) [^\n]*\nendobj/m', $bytes, $info, PREG_OFFSET_CAPTURE);
    [$object, $start] = $info[0];
    $expected = hash('sha256', substr_replace($bytes, '', $start, strlen($object)));
    var_dump(Pdf\PdfDocument::hashDocument($path) === $expected);

    // Keep byte offsets intact while removing the Info object's terminator.
    file_put_contents($path, substr_replace($bytes, '      ', $start + strlen($object) - strlen('endobj'), 6));
    try {
        Pdf\PdfDocument::hashDocument($path);
        echo "Unexpected success\n";
    } catch (Exception $exception) {
        echo $exception->getMessage(), "\n";
    }

    // Keep the xref and its offset intact while replacing the trailer reference.
    file_put_contents($path, preg_replace('/\/Info \d+ 0 R/', '/Info /invalid', $bytes));
    foreach ([$path, $path . '-missing', sys_get_temp_dir()] as $input) {
        try {
            Pdf\PdfDocument::hashDocument($input);
            echo "Unexpected success\n";
        } catch (Exception $exception) {
            echo $exception->getMessage(), "\n";
        }
    }

    file_put_contents($path, 'Readable data without a PDF trailer');
    var_dump(Pdf\PdfDocument::hashDocument($path) === hash_file('sha256', $path));
} finally {
    unlink($path);
}
?>
--EXPECT--
bool(true)
Unable to read the PDF Info object
Invalid PDF Info reference
Unable to open file for PDF hashing
PDF hashing requires a regular file
bool(true)
