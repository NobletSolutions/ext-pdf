--TEST--
PDF hashing preserves legacy SHA-256 digests and excludes only the legacy Info byte range
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$path = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
try {
    writeThreePagePdf($path);
    $document = new Pdf\PdfDocument($path);
    echo $document->hashContent(), "\n";
    var_dump($document->hashContent() === hash_file('sha256', $path));
    unset($document);

    writeThreePagePdf($path, 'Page', true);
    $hash = Pdf\PdfDocument::hashDocument($path);
    echo $hash, "\n";

    // A same-length metadata edit leaves the cross-reference offsets intact.
    writeThreePagePdf($path, 'Page', true, 'Mock Fixture');
    var_dump(Pdf\PdfDocument::hashDocument($path) === $hash);
    writeThreePagePdf($path, 'Book', true);
    var_dump(Pdf\PdfDocument::hashDocument($path) !== $hash);

    writeThreePagePdf($path, 'Page', true, 'PHPT Fixture', 4096);
    var_dump(Pdf\PdfDocument::hashDocument($path) === hash_file('sha256', $path));

    // The legacy scan stops at a literal match inside a metadata value.
    writeThreePagePdf($path, 'Page', true, 'Before endobj After');
    $bytes = file_get_contents($path);
    $creator = strpos($bytes, '<< /Creator');
    $start = strrpos(substr($bytes, 0, $creator - 1), "\n") + 1;
    $end = strpos($bytes, 'endobj', $start) + strlen('endobj');
    var_dump(Pdf\PdfDocument::hashDocument($path) === hash('sha256', substr_replace($bytes, '', $start, $end - $start)));

    // Generated from the metadata fixture with qpdf --static-id
    // --object-streams=generate --stream-data=uncompress.
    echo Pdf\PdfDocument::hashDocument(__DIR__ . '/fixtures/compressed-info.pdf.fixture'), "\n";
} finally {
    unlink($path);
}
?>
--EXPECT--
547341e61ab511dba05e3ce09829d8add4da7e2f9d4ae3cd4c3c18c58549918f
bool(true)
d35c5fcaf2e7ddc98c5ad5f05f6edfa21bba583254d0ebfb94cf2c5836a1f1a6
bool(true)
bool(true)
bool(true)
bool(true)
7000909a05b1a0ca3a3336b371fa808fcad7ae1fe0510a29d1510d578bb83c62
