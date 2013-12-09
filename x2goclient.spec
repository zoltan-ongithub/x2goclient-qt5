Name:           x2goclient
Version:        4.0.1.2
Release:        0.0x2go1%{?dist}
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
%if 0%{?el5} || 0%{?el6}
BuildRequires:  qt4-devel
%else
BuildRequires:  qt-devel
%endif
%if 0%{?fedora} >= 19 || 0%{?rhel} >= 6
BuildRequires:  qtbrowserplugin-static
%endif
Requires:       hicolor-icon-theme
Requires:       mozilla-filesystem
Requires:       nxproxy

%if 0%{?el5}
# For compatibility with EPEL5
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
%endif

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
%if 0%{?el5}
sed -i -e '/^QMAKE_BINARY=/s@qmake-qt4@%{_libdir}/qt4/bin/qmake@' Makefile
sed -i -e '/^LRELEASE_BINARY=/s@lrelease@%{_libdir}/qt4/bin/lrelease@' Makefile
%endif
%if 0%{?fedora} >= 19 || 0%{?rhel} >= 6
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
