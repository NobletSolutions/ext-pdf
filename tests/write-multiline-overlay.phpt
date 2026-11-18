--TEST--
PdfWriter saves a multiline text overlay and preserves the original PDF text
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

function printPdfText(string $path): void
{
    $document = new Pdf\PdfDocument($path);
    printf("%d pages\n", $document->getNumberOfPages());
    foreach (preg_split('/\R/', $document->asString()) as $line) {
        $line = trim($line);
        if ($line !== '') {
            echo $line, "\n";
        }
    }
}

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$output = $input . '-overlay.pdf';
$fixture = $input . '-fontconfig';
$fontDir = $fixture . '/php-pdf-fonts';
$config = $fixture . '/fonts.conf';

try {
    // PdfWriter only discovers fonts under paths containing php-pdf-fonts.
    mkdir($fontDir, 0700, true);
    copy(dirname(__DIR__) . '/fonts/arial.ttf', $fontDir . '/arial.ttf');
    copy(__DIR__ . '/fixtures/overlay-fonts.conf', $config);
    putenv("FONTCONFIG_FILE=$config");
    putenv("XDG_CACHE_HOME=$fixture");
    if (Pdf\getFonts() !== ['Arial']) {
        throw new RuntimeException('Expected only Arial from the Fontconfig fixture');
    }

    writeThreePagePdf($input);
    $writer = new Pdf\PdfWriter($input, $output);
    if ($writer->getAllFonts() !== ['Arial']) {
        throw new RuntimeException('Expected the writer to list the fixture font');
    }
    $writer->setFont('Arial');
    $writer->writeTextToPage(0, [
        new Pdf\PdfText(24, 140, "Overlay Alpha\nOverlay Beta"),
        new Pdf\PdfText(24, 95, 'Styled first page', 14, 'Arial'),
    ]);
    $writer->writeTextToPage(1, [
        new Pdf\PdfText(24, 140, 'Second page overlay'),
        new Pdf\PdfText(24, 100, 'Styled second page', 8, 'Arial'),
    ]);
    $writer->save();
    printPdfText($output);

    $writer = new Pdf\PdfWriter($input, $output);
    $writer->setFont('Arial');
    $writer->writeTextToPage(1, [new Pdf\PdfText(24, 140, 'Omitted page overlay')]);
    $writer->writeTextToPage(2, [new Pdf\PdfText(24, 140, 'Selected page overlay')]);
    $writer->save(['1', '3']);
    printPdfText($output);
} finally {
    unlink($input);
    if (is_file($output)) {
        unlink($output);
    }
    if (is_dir($fixture)) {
        $files = new RecursiveDirectoryIterator($fixture, FilesystemIterator::SKIP_DOTS);
        foreach (new RecursiveIteratorIterator($files, RecursiveIteratorIterator::CHILD_FIRST) as $file) {
            if ($file->isDir()) {
                rmdir($file->getPathname());
            } else {
                unlink($file->getPathname());
            }
        }
        rmdir($fixture);
    }
}
?>
--EXPECT--
3 pages
Page One
Overlay Alpha
Overlay Beta
Styled first page
Page Two
Second page overlay
Styled second page
Page Three
2 pages
Page One
Page Three
Selected page overlay
