#!/bin/sh
set -eu

mock_dir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
mock=${MOCK:-mock}
config_dir=$(realpath -- "${MOCK_CONFIG_DIR:-/etc/mock}")

case "$1" in
    build)
        if [ ! -f "$2" ]; then
            echo "Usage: make mock-matrix SRPM=/path/to/php-pdf.src.rpm" >&2
            exit 1
        fi
        srpm=$(realpath -- "$2")
        results=${MOCK_RESULTS_DIR:-.build/mock}
        mkdir -p -- "$results"
        run_dir=$(mktemp -d "$(realpath -- "$results")/run.XXXXXX")
        printf 'Mock matrix results: %s\n' "$run_dir"
        ;;
    smoke)
        if [ ! -d "$2" ]; then
            echo "Usage: make mock-smoke MOCK_RUN_DIR=/path/to/matrix-run" >&2
            exit 1
        fi
        run_dir=$(realpath -- "$2")
        ;;
esac

smoke_dir=$(mktemp -d "$run_dir/smoke.XXXXXXXXXXXX")
smoke_id="smoke-${smoke_dir##*.}"
printf 'Installed-RPM smoke logs: %s\n' "$smoke_dir"

run_mock() {
    "$mock" --configdir "$config_dir" -r "$config" --config-opts "root=$root" "$@"
}

smoke_mock() {
    run_mock --uniqueext "$smoke_id" --resultdir "$smoke_result" "$@"
}

smoke_package() (
    set -- "$result_dir"/php-pdf-[0-9]*.x86_64.rpm
    if [ "$#" -ne 1 ] || [ ! -f "$1" ]; then
        printf 'Expected one php-pdf binary RPM in %s\n' "$result_dir" >&2
        exit 1
    fi
    rpm_file=$1
    expected=$(rpm -qp --qf '%{NEVRA}' -- "$rpm_file") || exit 1

    probe=$(cat <<'SH'
unset PHPRC PHP_INI_SCAN_DIR
ini=/etc/php.d/60-pdf.ini
if [ "$(rpm -qf --qf "%{NEVRA}" "$ini")" != "$2" ]; then
    printf 'Expected %s to be owned by %s\n' "$ini" "$2" >&2
    exit 1
fi
printf 'Installed %s\n' "$2"
/usr/bin/php -r '
    $series = PHP_MAJOR_VERSION . "." . PHP_MINOR_VERSION;
    printf("PHP %s\n", PHP_VERSION);
    if ($series !== $argv[1]) {
        fprintf(STDERR, "Expected PHP %s\n", $argv[1]);
        exit(1);
    }
    $scanned = preg_split("/,\s*/", trim(php_ini_scanned_files() ?: ""));
    if (!in_array($argv[2], $scanned, true)) {
        fprintf(STDERR, "PHP did not scan %s\n", $argv[2]);
        exit(1);
    }
    printf("Scanned %s\n", $argv[2]);
' "$1" "$ini"
if /usr/bin/php -n --ri pdf >/dev/null 2>&1; then
    echo 'pdf also loaded with INI processing disabled' >&2
    exit 1
fi
/usr/bin/php --ri pdf
SH
)
    result=0
    smoke_mock --init &&
        smoke_mock --install php-cli "$rpm_file" &&
        smoke_mock --chroot --unpriv -- /bin/sh -ec "$probe" sh "$series" "$expected" ||
        result=$?
    smoke_mock --clean || result=1
    exit "$result"
)

status=0
for series in 8.3 8.4 8.5; do
    suffix="php${series%.*}${series#*.}"
    root="alma+epel-9-x86_64-$suffix"
    config="$config_dir/$root.cfg"
    if [ "$series" = "8.4" ]; then
        config="$mock_dir/php84.cfg"
    fi
    result_dir="$run_dir/$suffix"
    if [ "$1" = build ]; then
        mkdir -- "$result_dir"
        printf 'Building PHP %s with %s\n' "$series" "$root"
        if ! run_mock --resultdir "$result_dir" --define "php_pdf_expected_php $series" --rebuild "$srpm"; then
            printf 'Mock failed for PHP %s (%s)\n' "$series" "$result_dir" >&2
            status=1
            continue
        fi
    fi

    smoke_result="$smoke_dir/$suffix"
    mkdir -- "$smoke_result"
    printf 'Testing installed RPM on PHP %s\n' "$series"
    if smoke_package > "$smoke_result/smoke.log" 2>&1; then
        printf 'Installed RPM passed on PHP %s\n' "$series"
    else
        printf 'Installed RPM failed on PHP %s (%s/smoke.log)\n' "$series" "$smoke_result" >&2
        status=1
    fi
done
exit "$status"
