<?php

function writeThreePagePdf(string $path, string $prefix = 'Page', bool $withMetadata = false): void
{
    $objects = [
        '<< /Type /Catalog /Pages 2 0 R >>',
        '<< /Type /Pages /Kids [4 0 R 6 0 R 8 0 R] /Count 3 >>',
        '<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>',
    ];

    foreach (['One', 'Two', 'Three'] as $label) {
        $streamObject = count($objects) + 2;
        $objects[] = '<< /Type /Page /Parent 2 0 R /MediaBox [0 0 144 216] '
            . '/Resources << /Font << /F1 3 0 R >> >> /Contents ' . $streamObject . ' 0 R >>';
        $stream = "BT /F1 12 Tf 24 180 Td ($prefix $label) Tj ET\n";
        $objects[] = '<< /Length ' . strlen($stream) . " >>\nstream\n" . $stream . 'endstream';
    }

    $info = '';
    if ($withMetadata) {
        $objects[] = '<< /Creator (PHPT Fixture) /CreationDate (D:20240102030405Z) /ModDate (D:20240203040506Z) >>';
        $info = ' /Info ' . count($objects) . ' 0 R';
    }

    $pdf = "%PDF-1.4\n";
    $xref = "0000000000 65535 f \n";
    foreach ($objects as $index => $object) {
        $xref .= sprintf("%010d 00000 n \n", strlen($pdf));
        $pdf .= ($index + 1) . " 0 obj\n" . $object . "\nendobj\n";
    }

    $xrefOffset = strlen($pdf);
    $size = count($objects) + 1;
    $pdf .= "xref\n0 $size\n$xref";
    $pdf .= "trailer\n<< /Size $size /Root 1 0 R$info >>\nstartxref\n$xrefOffset\n%%EOF\n";

    if (file_put_contents($path, $pdf) !== strlen($pdf)) {
        throw new RuntimeException('Unable to write the test PDF');
    }
}
