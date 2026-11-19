# php-pdf

Run the PHPT suite locally with `make test`. It builds a fresh extension and uses PHP's `run-tests.php` from the development package. Set `PHP_CONFIG` and, if needed, `PHP` or `RUN_TESTS` to select a different installation.

## PHP API examples

`PdfWriter::complexCombine()` takes the destination path first and an array of `PdfCopy` objects second. Both positional and named calls use the same parameters.

```php
$documents = [new Pdf\PdfCopy('input.pdf', 1, '2-3')];
Pdf\PdfWriter::complexCombine('output.pdf', $documents);
Pdf\PdfWriter::complexCombine(destination: 'output.pdf', documents: $documents);
```

`new Pdf\PdfDocument($path, $ownerPassword, $userPassword)` takes the owner password second and the user password third. The corresponding PHP parameter names are `user` and `password`. Pass an empty string as the second argument when you only have the user password.

### PDF content hashes

`PdfDocument::hashContent()` and `PdfDocument::hashDocument($path)` return a lowercase SHA-256 digest of the file bytes. When PDFWriter resolves the trailer's standalone `/Info` dictionary, the hash omits bytes from its cross-reference offset through the first literal `endobj` in the legacy 4 KiB search window, before any NUL byte. If that window has no match, the object terminator is located with the PDF tokenizer. Bytes outside that range, including other metadata, cross-reference tables, and trailer entries, contribute to the digest. Changes to those bytes can change the digest even when only metadata was edited.

For compatibility, an Info position on a 4 KiB boundary leaves the file intact. For compressed Info entries, the legacy search starts at the byte offset equal to the object-stream number. The entire file is hashed if that search finds no match, or if PDFWriter cannot find a usable trailer or Info reference. I/O failures and invalid Info types or standalone objects raise an exception.

## AlmaLinux 9 RPM matrix

Run the same source RPM against the existing `alma+epel-9-x86_64-php{83,84,85}.cfg` Mock configs in `/etc/mock`. Use a host with working Mock access.

```sh
make mock-matrix SRPM=/path/to/php-pdf.src.rpm
```

Each PHP stream gets its own Mock root and cache. The command prints a new result directory under `.build/mock/` with `php83/`, `php84/`, and `php85/` subdirectories for RPMs and logs. Set `MOCK_CONFIG_DIR` or `MOCK_RESULTS_DIR` to use different locations. The command attempts all three builds and exits unsuccessfully if any stream fails.

The PHP 8.4 entry includes the existing system config and redirects its retired Noblet repository URL to the Prolegis PHP 8.4 repository.

RPM's `%check` loads the staged extension and runs the PHPT suite. In the matrix it also checks that the buildroot's PHP CLI matches the requested major and minor version.

After each build, the matrix installs the resulting binary RPM and `php-cli` into a fresh matching Mock chroot. It verifies that PHP scans the RPM-owned `/etc/php.d/60-pdf.ini`, that the loaded extension reports the RPM version, and that `php --ri pdf` succeeds. Smoke logs are saved under `smoke.*/php{83,84,85}/` in the matrix result directory. To repeat the smoke tests for an existing run, use the following command.

```sh
make mock-smoke MOCK_RUN_DIR=/path/to/matrix-run
```

## Building a release

Pass an explicit Git ref to build the source archive and SRPM, run the Mock matrix, and smoke-test the installed RPMs. The packaging tools are `git`, `rpmspec`, `rpmbuild`, and `bzip2`.

```sh
make release REF=v0.12.1
```

The archive and spec both come from the resolved commit. The spec supplies the version used by the RPM and the extension. Source artifacts go in `.build/release/<commit>/`, or under `RELEASE_DIR` if set. Use `make dist REF=<ref>` for just the archive or `make srpm REF=<ref>` for the archive and SRPM.
