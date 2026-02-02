# plf_bitsets
plf::bitset implements all the functionality of std::bitset with a few small exceptions (some constructors, some minor function differences). 

plf::bitsetb with it's first template parameter, "user_supplied_buffer" set to 'true', is a 'borrowing' bitset, which has it's buffer and size supplied by the user in the constructor, instead of allocating itself. This is useful for treating any particular block of memory you happen to have as a bitset. Most of it's functionality is the same as plf::bitset, though it also has move construction/assignment.

plf::bitsetb with it's first template parameter, "user_supplied_buffer" set to 'false', allocates it's own buffer on the heap and deallocates on destruction, while it's size is supplied by the constructor. This is useful if you have a non-templated class where you want to have differently-sized member bitsets between class instances, or the size of the bitset isn't known at runtime.


As a brief overview of plf::bitset's performance characteristics, versus std::bitset under GCC-libstdc++/MSVC-MSSTL respectively:
Under release (O2, AVX2) builds it has:
* 34652%/67612% faster setting/resetting of ranges of bits (via functions set_range and reset_range).
* 101%/35% faster left-shifting and 98%/22% right-shifting.
* 6%/204% faster set(position, value).
* 3%/0% faster operator [ ].
* 24%/20% faster overall in test suite benchmarks (testing all functionality of bitset on loop).

Under debug builds it has:
* 428127%/750726% faster setting/resetting of ranges of bits.
* 108%/85% faster left-shifting and 110%/66% right-shifting.
* 206%/31% faster set(position, value).
* 360%/132% faster operator [ ].
* 175%/40% faster overall in test suite benchmarks

The benchmarks on the project page (https://plflib.org/bitsets.htm) give more details. Most other performance characteristics are more or less the same between plf and std.

All the bitsets have additional functionality:
* Copy constructor/assignment
* The set_range/reset_range functions
* Range-based equivalents of the any/all/none/count functions
* Optimized functions for finding the first/last zero/one of the bitset
* An allocation-free noexcept swap() using the XOR method.
* Functions for index-congruent to_string and to_ulong/ullong functions.

They don't implement the from-string or from-ulong/ullong constructors. Index bounds-checking for functions is supported by the third template parameter, 'bool hardened' (false by default).
The second template parameter on each bitset, 'storage_type', allows the user to specify what type of unsigned integer to use for the internal storage. This can save space for small bitsets with less than 64 bits.

Again, see the project page for more details.

All three bitsets are under an ethical license "Computing for Good". You can find more information about it here: https://plflib.org/computing_for_good_license.htm
