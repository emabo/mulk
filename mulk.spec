%define name mulk
%define ver 0.7.0
%define ver2 0.1.0
%define rel 1

Summary: Non-interactive multi-connection network downloader with image filtering and Metalink support.
Name: %{name}
Version: %{ver}
Release: %{rel}
License: GPL
Group: Utilities/Console
Source: %{name}-%{ver}.tar.gz
URL: http://sourceforge.net/projects/mulk/
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-%{ver}-%{rel}-buildroot

%description
Multi-connection command line tool for downloading Internet sites with image
filtering and Metalink support. Similar to wget and cURL, but it manages up to 50
simultaneous and parallel links. Main features are: HTML code parsing, recursive 
fetching, Metalink retrieving, segmented download and image filtering by width and height.
It is based on libcurl, liburiparser, libtidy, libmetalink and libcrypto.

%package devel
Summary: The includes and libraries to develop with libmulk
Group: Development/Libraries

%description devel
libmulk is the library containing all mulk features.

%prep
%setup -q

%build
%configure --prefix=%{prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
%find_lang %{name}

%clean
rm -rf ${RPM_BUILD_ROOT}

%files -f %{name}.lang
%defattr(-,root,root)
%attr(0755,root,root) %{_bindir}/mulk
%attr(0644,root,root) %{_mandir}/man1/mulk.1*
%{_libdir}/libmulk.so*
%doc COPYING COPYING.LESSER LICENSE.OpenSSL AUTHORS README NEWS THANKS ChangeLog

%files devel
%defattr(-,root,root)
%attr(0644,root,root) %{_includedir}/mulk/* 
%{_libdir}/libmulk.a
%{_libdir}/libmulk.la

%changelog
* Sat Jun 16 2012 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- library added to RPM.

* Fri Mar 26 2010 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- License has been changed to LGPL for the library.

* Fri Mar 26 2010 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- Internationalization handling added.

* Mon Apr 27 2009 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- Description changed.

* Fri Mar 27 2009 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- Create.
