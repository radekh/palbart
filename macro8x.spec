%define name	macro8x
%define ver	1.0.0
%define rel	1

Summary: DEC PDP-8 series cross assembler
Name: %{name}
Version: %{ver}
Release: %{rel}
Copyright: None
Group: Development/Languages
Vendor:	Bob Supnik bob.supnik@ljo.dec.com & Gary Messenbrink
Packager: Gary Messenbrink <gam@rahul.net>
Prefix: /usr/local/bin

%description
An assembler with macro capabilities that generates code for Digital Equipment Corporation
PDP-8 series computers. Generates a formatted listing, an alphabetized symbold table and a
cross reference table. Extensive error information provided via an error file.

%define namver %{name}\-%{ver}

%prep

%files
%defattr(-,root,root)
%{prefix}/%{name}
