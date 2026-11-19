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

## AlmaLinux 9 RPM matrix

Run the same source RPM against the existing `alma+epel-9-x86_64-php{83,84,85}.cfg` Mock configs in `/etc/mock`. Use a host with working Mock access.

```sh
make mock-matrix SRPM=/path/to/php-pdf.src.rpm
```

Each PHP stream gets its own Mock root and cache. The command prints a new result directory under `.build/mock/` with `php83/`, `php84/`, and `php85/` subdirectories for RPMs and logs. Set `MOCK_CONFIG_DIR` or `MOCK_RESULTS_DIR` to use different locations. The command attempts all three builds and exits unsuccessfully if any stream fails.

The PHP 8.4 entry includes the existing system config and redirects its retired Noblet repository URL to the Prolegis PHP 8.4 repository.

RPM's `%check` loads the staged extension and runs the PHPT suite. In the matrix it also checks that the buildroot's PHP CLI matches the requested major and minor version.

After each build, the matrix installs the resulting binary RPM and `php-cli` into a fresh matching Mock chroot. It verifies that PHP scans the RPM-owned `/etc/php.d/60-pdf.ini` and that `php --ri pdf` succeeds. Smoke logs are saved under `smoke.*/php{83,84,85}/` in the matrix result directory. To repeat the smoke tests for an existing run, use the following command.

```sh
make mock-smoke MOCK_RUN_DIR=/path/to/matrix-run
```

To create the SRPM from a Git revision that includes the RPM test configuration, run these commands from the checkout on a host with `rpmbuild`, `rpmspec`, and `bzip2`.

```sh
ref=HEAD
topdir=$(mktemp -d)
mkdir -p "$topdir/SOURCES" "$topdir/SPECS"
spec="$topdir/SPECS/php-pdf.spec"
git show "$ref:php-pdf.spec" > "$spec"
package=$(rpmspec -q --srpm --qf '%{NAME}-%{VERSION}' "$spec")
git archive --prefix="$package/" "$ref" | bzip2 > "$topdir/SOURCES/$package.tar.bz2"
rpmbuild -bs --define "_topdir $topdir" "$spec"
make mock-matrix SRPM="$(find "$topdir/SRPMS" -name '*.src.rpm' -print)"
```
