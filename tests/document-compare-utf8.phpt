--TEST--
PdfDocument::compare() detects text differences after multibyte characters
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$leftPath = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$rightPath = $leftPath . '-right.pdf';

try {
    // The fixture uses WinAnsiEncoding, where byte E9 maps to U+00E9.
    $encodedPrefix = str_repeat("\xE9", 10);
    writeThreePagePdf($leftPath, $encodedPrefix . ' A');
    writeThreePagePdf($rightPath, $encodedPrefix . ' B');

    $left = new Pdf\PdfDocument($leftPath);
    $right = new Pdf\PdfDocument($rightPath);
    $utf8Prefix = str_repeat("\u{00E9}", 10);
    var_dump(str_contains($left->asString(), "$utf8Prefix A One"));
    var_dump(str_contains($right->asString(), "$utf8Prefix B One"));

    var_dump($left->compare($left));
    var_dump($left->compare($right));
    var_dump($right->compare($left));
} finally {
    unlink($leftPath);
    if (is_file($rightPath)) {
        unlink($rightPath);
    }
}
?>
--EXPECT--
bool(true)
bool(true)
int(0)
int(13)
int(13)
