Summary: Non-interactive multi-connection network downloader with image filtering and Metalink support.
Name: mulk
Version: 0.5.2
Release: 1
License: GPL
Group: Utilities/Console
Source0: %{name}-%{version}.tar.gz
URL: http://sourceforge.net/projects/mulk/
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
Multi-connection command line tool for downloading Internet sites with image
filtering and Metalink support. Similar to wget and cURL, but it manages up to 50
simultaneous and parallel links. Main features are: HTML code parsing, recursive 
fetching, Metalink retrieving, segmented download and image filtering by width and height.
It is based on libcurl, liburiparser, libtidy, libmetalink and libcrypto.

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
%attr(0644,root,root) %{_mandir}/man1/mulk.1.bz2
%doc COPYING COPYING.LESSER LICENSE.OpenSSL AUTHORS README NEWS THANKS ChangeLog

%changelog
* Fri Mar 26 2010 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- License has been changed to LGPL for the library.

* Fri Mar 26 2010 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- Internationalization handling added.

* Mon Apr 27 2009 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- Description changed.

* Fri Mar 27 2009 Emanuele Bovisio <pocoyo@users.sourceforge.net>
- Create.
