# SPDX-License-Identifier: GPL-2.0-only
Name:           rf-2a4m1
Version:        1.0.0
Release:        1%{?dist}
Summary:        GenBasic RF-2A4M1 USB Wi-Fi DKMS driver (GPL, from source)

License:        GPLv2
URL:            https://github.com/GenBasic/genbasic-rf-2a4m1
BuildArch:      noarch

Requires:       dkms >= 2.1.0.0
Requires(post): dkms
Requires(preun):dkms
Recommends:     rf-2a4m1-firmware

%description
Fully-GPL cfg80211 kernel module for the GenBasic RF-2A4M1 single-band 2.4 GHz
(1T1R) 802.11b/g/n USB Wi-Fi adapter. Compiled from source per-kernel by DKMS;
rebuilds automatically on kernel upgrade. Architecture-independent (noarch).

%prep
# Sources are staged from the checkout by the build wrapper.

%install
mkdir -p %{buildroot}%{_usrsrc}/%{name}-%{version}
cp -a src Kbuild Makefile dkms.conf VERSION %{buildroot}%{_usrsrc}/%{name}-%{version}/

%post
%dkms_add %{name}
%dkms_build %{name}
%dkms_install %{name}

%preun
%dkms_remove %{name} %{version}

%files
%license LICENSE
%{_usrsrc}/%{name}-%{version}/

%changelog
* Mon Jul 13 2026 GenBasic <engineering@genbasic.com> - 1.0.0-1
- Initial packaging skeleton.
