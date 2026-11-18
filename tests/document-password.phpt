--TEST--
PdfDocument opens an encrypted PDF using constructor passwords
--FILE--
<?php
// The synthetic fixture was encrypted with qpdf --encrypt fixture-user fixture-owner 256.
$path = __DIR__ . '/fixtures/password-protected.pdf.fixture';
$cases = [
    'Owner password' => ['fixture-owner'],
    'User password' => ['', 'fixture-user'],
];
foreach ($cases as $label => $passwords) {
    echo $label, "\n";
    $document = new Pdf\PdfDocument($path, ...$passwords);
    var_dump($document->isEncrypted(), $document->isLocked());
    if (!$document->isLocked()) {
        printf("%d pages\n", $document->getNumberOfPages());
    }
}
?>
--EXPECT--
Owner password
bool(true)
bool(false)
3 pages
User password
bool(true)
bool(false)
3 pages
