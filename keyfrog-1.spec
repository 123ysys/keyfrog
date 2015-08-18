Summary: KeyFrog
Name: keyfrog
Version: 1.1
Release: 1
Source0: %{name}-%{version}.tar.gz
License: BSD
URL: http://keyfrog.sf.net
Group: Monitoring
BuildRoot: %{_builddir}/%{name}-root

%description
Keyfrog monitors keyboard and visualizes its usage statistics. User can obtain
detailed information about keyboard activity: the intensity of keyboard usage,
how was it distributed in time, which applications were used, etc. This may be
useful, for example, to developers to monitor their productivity.

%prep
%setup -q

%build
./configure --prefix=/usr --with-keyvis=no
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/bin/keyfrog
%dir /usr/share/keyfrog
%dir /usr/share/keyfrog/doc
%doc /usr/share/keyfrog/doc/sample-config
%doc LICENSE.BSD AUTHORS README NEWS
