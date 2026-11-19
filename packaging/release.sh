#!/bin/sh
set -eu

usage() {
    echo 'Usage: make {dist|srpm|release} REF=<tag-or-commit>' >&2
    exit 1
}

[ "$#" -eq 3 ] && [ -n "$2" ] || usage
case "$1" in
    dist|srpm|release) ;;
    *) usage ;;
esac

scripts=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
project=$(dirname -- "$scripts")
commit=$(git -C "$project" rev-parse --verify --end-of-options "$2^{commit}")
mkdir -p -- "$3/$commit"
output=$(realpath -- "$3/$commit")
work=$(mktemp -d "$output/.release.XXXXXX")
trap 'rm -rf -- "$work"' EXIT

mkdir "$work/SOURCES" "$work/SPECS" "$work/tmp"
spec="$work/SPECS/php-pdf.spec"
git -C "$project" show "$commit:php-pdf.spec" > "$spec"
package=$(rpmspec -q --srpm --qf '%{NAME}-%{VERSION}' "$spec")
archive="$package.tar.bz2"

git -C "$project" archive --format=tar --prefix="$package/" "$commit" > "$work/source.tar"
bzip2 -c "$work/source.tar" > "$work/SOURCES/$archive"
if [ "$1" = dist ]; then
    mv -- "$work/SOURCES/$archive" "$output/$archive"
    printf 'Source archive for %s\n%s\n' "$commit" "$output/$archive"
    exit 0
fi

rpmbuild -bs --define "_topdir $work" --define "_tmppath $work/tmp" "$spec"
set -- "$1" "$work"/SRPMS/*.src.rpm
if [ "$#" -ne 2 ] || [ ! -f "$2" ]; then
    echo 'Expected exactly one source RPM' >&2
    exit 1
fi
srpm="$output/$(basename -- "$2")"
mv -- "$work/SOURCES/$archive" "$output/$archive"
mv -- "$2" "$srpm"
printf 'Source artifacts for %s\n%s\n%s\n' "$commit" "$output/$archive" "$srpm"

if [ "$1" = release ]; then
    sh "$scripts/mock/matrix.sh" build "$srpm"
fi
