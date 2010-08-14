%define name	palbart
%define ver	2.4.0
%define rel	1

Summary: DEC PDP-8 series cross assembler
Name: %{name}
Version: %{ver}
Release: %{rel}
Copyright: None
Group: Development/Languages
Vendor:	Gary Messenbrink
Packager: gam@rahul.net
Prefix: /usr/local/bin

%description
An assembler that generates code for Digital Equipment Corporation PDP-8 series computers.
Generates a formatted listing, an alphabetized symbold table and a cross reference table.
Extensive error information provided via an error file.

%define namver %{name}\-%{ver}

%prep

%files
%defattr(-,root,root)
%{prefix}/%{name}
