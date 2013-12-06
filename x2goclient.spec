Name:           x2goclient
Version:        4.0.1.2
Release:        0x2go1%{?dist}
Summary:        Graphical client for use with the X2Go network based computing environment

Group:          Applications/Communications
License:        GPLv2+
URL:            http://www.x2go.org
Source0:        http://code.x2go.org/releases/source/%{name}/%{name}-%{version}.tar.gz

BuildRequires:  cups-devel
BuildRequires:  desktop-file-utils
BuildRequires:  libssh-devel
BuildRequires:  libXpm-devel
%if 0%{?fedora}
BuildRequires:  man2html-core
%else
BuildRequires:  man
%endif
BuildRequires:  openldap-devel
BuildRequires:  qt-devel
%if 0%{?fedora_version} >= 18
BuildRequires:  qtbrowserplugin-static
%elif 0%{?rhel_version} >= 6
BuildRequires:  qtbrowserplugin-static
%endif
Requires:       hicolor-icon-theme
Requires:       mozilla-filesystem
Requires:       nxproxy

%description
This client will be able to connect to X2Go server(s) and start, stop, resume 
and terminate (running) desktop sessions. X2Go Client stores different server 
connections and may automatically request authentication data from LDAP 
directories. 


%prep
%setup -q
# Fix up install issues
sed -i -e 's/-o root -g root//' Makefile
sed -i -e '/^MOZPLUGDIR=/s/lib/%{_lib}/' Makefile
%if 0%{?fedora_version} >= 18
# Use system qtbrowserplugin
sed -i -e '/CFGPLUGIN/aTEMPLATE=lib' x2goclient.pro
sed -i -e '/^LIBS /s/$/ -ldl/' x2goclient.pro
sed -i -e 's/include.*qtbrowserplugin.pri)/LIBS += -lqtbrowserplugin/' x2goclient.pro
rm -r qtbrowserplugin*
%elif 0%{?rhel_version} >= 6
# Use system qtbrowserplugin
sed -i -e '/CFGPLUGIN/aTEMPLATE=lib' x2goclient.pro
sed -i -e '/^LIBS /s/$/ -ldl/' x2goclient.pro
sed -i -e 's/include.*qtbrowserplugin.pri)/LIBS += -lqtbrowserplugin/' x2goclient.pro
rm -r qtbrowserplugin*
%endif

%build
export PATH=%{_qt4_bindir}:$PATH
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot} PREFIX=%{_prefix}
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop


%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :


%files
%doc AUTHORS COPYING LICENSE 
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%{_datadir}/icons/hicolor/16x16/apps/%{name}.png
%{_datadir}/icons/hicolor/32x32/apps/%{name}.png
%{_datadir}/icons/hicolor/64x64/apps/%{name}.png
%{_datadir}/%{name}/
%{_libdir}/mozilla/plugins/libx2goplugin.so
%{_mandir}/man1/%{name}.1.gz


%changelog
* Wed Sep 11 2013 Orion Poplawski <orion@cora.nwra.com> - 4.0.1.1-1
- Update to 4.0.1.1
- Drop patches applied upstream

* Thu Sep 5 2013 Orion Poplawski <orion@cora.nwra.com> - 4.0.1.0-5
- Build against system qtbrowserplugin

* Fri Aug 30 2013 Orion Poplawski <orion@cora.nwra.com> - 4.0.1.0-4
- Add BR desktop-file-utils and validate desktop file
- Add gtk-update-icon-cache scriptlets

* Wed Apr 10 2013 Orion Poplawski <orion@cora.nwra.com> - 4.0.1.0-3
- Add patch to set dpi automatically

* Thu Mar 28 2013 Orion Poplawski <orion@cora.nwra.com> - 4.0.1.0-2
- Add patch to fix proxy connection issue

* Mon Mar 25 2013 Orion Poplawski <orion@cora.nwra.com> - 4.0.1.0-1
- Update to 4.0.1.0

* Tue Feb 12 2013 Orion Poplawski <orion@cora.nwra.com> - 4.0.0.2-1
- Update to 4.0.0.2

* Fri Jan 18 2013 Orion Poplawski <orion@cora.nwra.com> - 4.0.0.1-1
- Update to 4.0.0.1

* Wed Dec 12 2012 Orion Poplawski <orion@cora.nwra.com> - 3.99.3.1-0.1
- Update to latest git

* Tue Dec 11 2012 Orion Poplawski <orion@cora.nwra.com> - 3.99.3.0-1
- Initial Fedora package
