# test-cpp

Here's a small unit test framework I made for C++. It's meant to be pretty simple, so it's not too
feature-rich. It allows for defining basic unit tests and data-driven row tests.


## Examples
The Samples directory contains a series of examples for how to use this framework.
- [SimpleTest.cpp](Samples/SimpleTest.cpp) demonstrates how to define a basic unit test.
- [RowTest.cpp](Samples/RowTest.cpp) demonstrates how to define a row test.
- [Driver.cpp](Samples/Driver.cpp) demonstrates how to run the tests.


## Building

Building requires a Windows 7 or newer environment with Visual Studio 2017 (Community edition works).
Visual Studio needs to have been installed with the C++ toolset.

Building can be done using the `build.cmd` script. This builds all platforms and configurations
that are defined; x86/x64 and Debug/Release.

This also builds a nuget package which allows for easy installation in projects.

The corresponding `clean.cmd` script is used to delete all build artifacts.
