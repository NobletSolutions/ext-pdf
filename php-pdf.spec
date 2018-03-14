Name:       php-pdf
Version:    0.9.4
Release:    1%{?dist}
Summary:    PHP PDF Poppler/ PDFWriter Extension

Group:      Development/Languages
License:    Apache 2.0
URL:        https://github.com/NobletSolutions/ext-pdf-writer
Source0:    %{name}-%{version}.tar.bz2

BuildRequires: php-devel >= 7.1 php-cpp-devel pdf-writer-devel poppler-cpp-devel fontconfig-devel
Requires: php-cpp poppler-cpp pdf-writer php-common >= 7.1 fontconfig php-pdf-fonts

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

%files
%{_libdir}/php/modules/pdf.so
%{_sysconfdir}/php.d/60-pdf.ini

%changelog

