<?php

    $pdfDoc = new \Pdf\PdfDocument('./File.pdf', 'user', 'password');

    $created = $pdfDoc->getCreationDate();
    $modified = $pdfDoc->getModifiedDate();
    echo "Created On: ".(($created)?$created->format('Y-m-d H:i:s'):'Not set')."\n";
    echo "CreatedBy: ".$pdfDoc->getCreator()."\n";
    echo "Modified On: ".(($modified)?$modified->format('Y-m-d H:i:s'):'Not set')."\n";
    echo "PDF Version: " . $pdfDoc->getMajorVersion() . '.' . $pdfDoc->getMinorVersion() . "\n";
    echo "Is Encrypted: " . (($pdfDoc->isEncrypted() === true) ? 'Yes' : 'No') . "\n";
    echo "Is Linear: " . (($pdfDoc->isLinear() === true) ? 'Yes' : 'No') . "\n";
    echo "NumPages: " . $pdfDoc->getNumberOfPages() . "\n";

