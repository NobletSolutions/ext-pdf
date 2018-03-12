<?php

if (class_exists('\Pdf\PdfWriter')) {
  $writer = new \Pdf\PdfWriter('./File.pdf', './Output-range.pdf');
  $writer->setFont("Arial");
  $writer->writeTextToPage(2, [new \Pdf\PdfText(100, 600, "The text to write")]);
  $writer->save(["1","2-3","6-8"]);
}
