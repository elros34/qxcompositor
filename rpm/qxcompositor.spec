Name:       qxcompositor

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}
Summary:    Qml compositor for running Xwayland on Sailfish
Version:    0.0.6
Release:    0
Group:      Qt/Qt
License:    BSD-3-Clause
URL:        https://github.com/elros34/qxcompositor
Source0:    %{name}-%{version}.tar.bz2

# Segregate the SFOS releases range covered by each release branch (branch qtwayland-5.6 for SFOS >= 4.2.0):
Requires: sailfish-version >= 4.2.0
# Requires: sailfish-version < X.Y.Z # Not yet known

Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Compositor)
BuildRequires:  pkgconfig(mlite5)


%description
Qml compositor for running Xwayland on Sailfish
# This description section includes metadata for SailfishOS:Chum, see
# https://github.com/sailfishos-chum/main/blob/main/Metadata.md
%if "%{?vendor}" == "chum"
PackageName: qxcompositor
Type: desktop-application
DeveloperName: elros34
Categories:
 - Development
 - Utilities
 - Other
Custom:
  Repo: https://github.com/elros34/qxcompositor
Url:
  Homepage: https://github.com/elros34/qxcompositor
%endif


%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}
