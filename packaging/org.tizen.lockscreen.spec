Name:       org.tizen.lockscreen
Summary:    Lockscreen application
Version:    0.1.16
Release:    0
Group:      Applications/Other
License:    Flora
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(capi-ui-efl-util)
BuildRequires: pkgconfig(ail)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(heynoti)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(libtzplatform-config)

BuildRequires: cmake
BuildRequires: gettext
BuildRequires: edje-tools

%description
Lockscreen application.

%prep
%setup -q

%build
LDFLAGS+="-Wl,--rpath=%{TZ_SYS_RO_APP}/%{name}/lib -Wl,--as-needed";export LDFLAGS
%cmake . -DCMAKE_INSTALL_PREFIX=%{TZ_SYS_RO_APP}/%{name} -DTZ_SYS_ETC=%{TZ_SYS_ETC}
make %{?jobs:-j%jobs}

%install
%make_install
mkdir -p %{buildroot}%{TZ_SYS_RO_APP}/%{name}/data
mkdir -p %{buildroot}%{_datarootdir}/license
install -m 0755 LICENSE.Flora %{buildroot}%{_datarootdir}/license/%{name}

%post
/sbin/ldconfig

GOPTION="-g 6514"

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%attr(-,inhouse,inhouse) %dir %{TZ_SYS_RO_APP}/%{name}/data
%{TZ_SYS_RO_APP}/%{name}/*
%{_datarootdir}/packages/%{name}.xml
%{_datarootdir}/license/%{name}
%{TZ_SYS_RO_APP}/%{name}/res/locale/*/LC_MESSAGES/lockscreen.mo
#%{TZ_SYS_ETC}/smack/accesses.d/%{name}.rule
