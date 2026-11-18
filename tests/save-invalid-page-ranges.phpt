--TEST--
PdfWriter::save() rejects invalid page ranges without replacing the destination
--FILE--
<?php
require __DIR__ . '/fixtures/three-page.php';

$input = tempnam(sys_get_temp_dir(), 'ext-pdf-test-');
$output = $input . '-output.pdf';

try {
    writeThreePagePdf($input);
    file_put_contents($output, 'existing destination');
    $writer = new Pdf\PdfWriter($input, $output);
    foreach (['0', '2-', '1-2-3', '3-2', '2-4'] as $range) {
        try {
            $writer->save([$range]);
            echo "Accepted $range\n";
        } catch (Exception $exception) {
            printf("Rejected %s, destination preserved: %s\n", $range,
                file_get_contents($output) === 'existing destination' ? 'yes' : 'no');
        }
    }
    $writer->save([2]);
    $document = new Pdf\PdfDocument($output);
    printf("Retry saved %d page: %s\n", $document->getNumberOfPages(), trim($document->asString(), " \r\n\t\x0c"));
} finally {
    unlink($input);
    if (file_exists($output)) {
        unlink($output);
    }
}
?>
--EXPECT--
Rejected 0, destination preserved: yes
Rejected 2-, destination preserved: yes
Rejected 1-2-3, destination preserved: yes
Rejected 3-2, destination preserved: yes
Rejected 2-4, destination preserved: yes
Retry saved 1 page: Page Two
