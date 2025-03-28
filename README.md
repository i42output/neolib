# Introduction
"neolib" is a cross-platform C++ utility library.

# Dependencies
* Boost
* OpenSSL
* zlib

# Features
* segmented_array container, see [http://i42.co.uk/stuff/segmented_array.htm](http://i42.co.uk/stuff/segmented_array.htm).
* gap_vector container, a gap buffer.
* neosigslot, see [http://i42.co.uk/stuff/neosigslot.htm](http://i42.co.uk/stuff/neosigslot.htm).
* NoFussXML, see [http://i42.co.uk/stuff/NoFussXML.htm](http://i42.co.uk/stuff/NoFussXML.htm).
* NoFussJSON, a fast JSON and Relaxed JSON parser/generator.
* packet stream network library (based on Boost.Asio).
* plugin framework; uses interfaces (vtables) similar to Microsoft's COM with support for containers/iterators and polymorphic variants and enums.
* ECS (Entity-Component-System) including power management (green/turbo modes) and a time system.
* vector/matrix math library with SIMD support
