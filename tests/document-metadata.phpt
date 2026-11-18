--TEST--
PdfDocument reads the creator, PDF dates, version and document flags
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$plain = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$withMetadata = $plain . '-metadata.pdf';

try {
    writeThreePagePdf($plain);
    $document = new Pdf\PdfDocument($plain);
    var_dump($document->getCreationDate(), $document->getModifiedDate());

    writeThreePagePdf($withMetadata, 'Page', true);
    $document = new Pdf\PdfDocument($withMetadata);
    printf("Creator: %s\n", $document->getCreator());
    printf("Created: %s\n", $document->getCreationDate()->format(DATE_ATOM));
    printf("Modified: %s\n", $document->getModifiedDate()->format(DATE_ATOM));
    printf("PDF %d.%d, %d pages\n", $document->getMajorVersion(), $document->getMinorVersion(), $document->getNumberOfPages());
    var_dump($document->isEncrypted(), $document->isLinear());

    // These methods are registered only when the build's Poppler supports them.
    if (method_exists($document, 'hasJS') || method_exists($document, 'hasXFA')) {
        if ($document->hasJS() !== false || $document->hasXFA() !== false) {
            throw new RuntimeException('The synthetic PDF contains neither JavaScript nor XFA');
        }
    }
} finally {
    unlink($plain);
    if (is_file($withMetadata)) {
        unlink($withMetadata);
    }
}
?>
--EXPECT--
NULL
NULL
Creator: PHPT Fixture
Created: 2024-01-02T03:04:05+00:00
Modified: 2024-02-03T04:05:06+00:00
PDF 1.4, 3 pages
bool(false)
bool(false)
