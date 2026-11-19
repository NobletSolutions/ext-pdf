%global source_date_epoch_from_changelog 0

Name:       php-pdf
Version:    0.12.1
Release:    1%{?dist}
Summary:    PHP PDF Poppler/ PDFWriter Extension

Group:      Development/Languages
License:    Apache 2.0
URL:        https://github.com/NobletSolutions/ext-pdf-writer
Source0:    %{name}-%{version}.tar.bz2

BuildRequires: php-devel >= 7.1 php-cpp-devel pdf-writer-devel poppler-cpp-devel fontconfig-devel openssl-devel
BuildRequires: gcc gcc-c++ automake make php-cli
Requires:      php-cpp poppler-cpp pdf-writer >= 4.0 openssl
Requires:      fontconfig php-pdf-fonts mscorefonts
Requires:      php(zend-abi) = %{php_zend_api}
Requires:      php(api) = %{php_core_api}

%description
PHP PDF Poppler / PDFWriter extension

%prep
%setup -q

%build

make %{?_smp_mflags}

%install
mkdir -p %{buildroot}`/usr/bin/php-config --extension-dir`
mkdir -p %{buildroot}/etc/php.d
%make_install

%check
%if 0%{?php_pdf_expected_php:1}
%{_bindir}/php -n -r '
    $series = PHP_MAJOR_VERSION . "." . PHP_MINOR_VERSION;
    if ($series !== $argv[1]) {
        fwrite(STDERR, "Expected PHP {$argv[1]}, found " . PHP_VERSION . "\n");
        exit(1);
    }
' '%{php_pdf_expected_php}'
%endif
%{__make} test PHP=%{_bindir}/php \
    TEST_EXTENSION="$RPM_BUILD_ROOT$(%{_bindir}/php-config --extension-dir)/pdf.so"

%files
%{_libdir}/php/modules/pdf.so
%{_sysconfdir}/php.d/60-pdf.ini

