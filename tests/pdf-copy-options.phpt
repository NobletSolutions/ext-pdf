--TEST--
PdfCopy exposes its selection and rejects non-numeric page syntax
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');

try {
    writeThreePagePdf($input);
    $default = new Pdf\PdfCopy($input);
    var_dump($default->getFilePath() === $input, $default->getCopies(), $default->getPages());

    $selection = new Pdf\PdfCopy($input, 4, '1');
    var_dump($selection->getCopies(), $selection->getPages());
    echo str_replace($input, 'fixture.pdf', (string) $selection), "\n";

    // PdfCopy emits a warning before throwing. Assert both without a path or line number.
    set_error_handler(function ($severity, $message) {
        echo "Warning: $message\n";
        return true;
    }, E_WARNING);
    try {
        new Pdf\PdfCopy($input, 1, '1,2,5-6a');
        echo "Invalid selection accepted\n";
    } catch (Exception $exception) {
        echo "Exception: ", $exception->getMessage(), "\n";
    } finally {
        restore_error_handler();
    }
} finally {
    unlink($input);
}
?>
--EXPECT--
bool(true)
int(1)
string(0) ""
int(4)
string(1) "1"
fixture.pdf Copies: 4 Pages: 1
Warning: Only 0-9, '-' and ',' characters are allowed
Exception: Only 0-9, '-' and ',' characters are allowed
