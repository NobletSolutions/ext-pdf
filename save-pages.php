<?php

if (class_exists('\Pdf\PdfWriter')) {
  $writer = new \Pdf\PdfWriter('./File.pdf', './Output-range.pdf');

  $writer->save(["1","2-3","6-8"]);
}
