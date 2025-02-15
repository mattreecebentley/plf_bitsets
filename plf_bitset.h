// Copyright (c) 2025, Matthew Bentley (mattreecebentley@gmail.com) www.plflib.org

// Computing For Good License v1.0 (https://plflib.org/computing_for_good_license.htm):
// This code is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this code.
//
// Permission is granted to use this code by anyone and for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
// 1.    The origin of this code must not be misrepresented; you must not claim that you wrote the original code. If you use this code in software, an acknowledgement in the product documentation would be appreciated but is not required.
// 2.    Altered code versions must be plainly marked as such, and must not be misrepresented as being the original code.
// 3.    This notice may not be removed or altered from any code distribution, including altered code versions.
// 4.    This code and altered code versions may not be used by groups, companies, individuals or in software whose primary or partial purpose is to:
//     a.    Promote addiction or intoxication.
//     b.    Cause harm to, or violate the rights of, other sentient beings.
//     c.    Distribute, obtain or utilize software, media or other materials without the consent of the owners.
//     d.    Deliberately spread misinformation or encourage dishonesty.
//     e.    Pursue personal profit at the cost of broad-scale environmental harm.




#ifndef PLF_BITSET_H
#define PLF_BITSET_H


// Compiler-specific defines:

 // defaults before potential redefinitions:
#define PLF_NOEXCEPT throw()
#define PLF_EXCEPTIONS_SUPPORT
#define PLF_CONSTEXPR
#define PLF_CONSTFUNC


#if ((defined(__clang__) || defined(__GNUC__)) && !defined(__EXCEPTIONS)) || (defined(_MSC_VER) && !defined(_CPPUNWIND))
	#undef PLF_EXCEPTIONS_SUPPORT
	#include <exception> // std::terminate
#endif


#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
	#if _MSC_VER >= 1900
		#undef PLF_NOEXCEPT
		#define PLF_NOEXCEPT noexcept
	#endif

	#if defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)
		#undef PLF_CONSTEXPR
		#define PLF_CONSTEXPR constexpr
	#endif

	#if defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L) && _MSC_VER >= 1929
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
		#define PLF_CPP20_SUPPORT
	#endif
#elif defined(__cplusplus) && __cplusplus >= 201103L // C++11 support, at least
	#if defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__clang__) // If compiler is GCC/G++
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept
		#endif
	#elif defined(__clang__)
		#if __has_feature(cxx_noexcept)
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept
		#endif
	#else // Assume noexcept support for other compilers
		#undef PLF_NOEXCEPT
		#define PLF_NOEXCEPT noexcept
	#endif

	#if __cplusplus >= 201703L && ((defined(__clang__) && ((__clang_major__ == 3 && __clang_minor__ == 9) || __clang_major__ > 3)) || (defined(__GNUC__) && __GNUC__ >= 7) || (!defined(__clang__) && !defined(__GNUC__))) // assume correct C++17 implementation for non-gcc/clang compilers
		#undef PLF_CONSTEXPR
		#define PLF_CONSTEXPR constexpr
	#endif

	// The following line is a little different from other plf:: containers because we need constexpr basic_string in order to make the to_string function constexpr:
	#if __cplusplus > 201704L && ((((defined(__clang__) && __clang_major__ >= 15) || (defined(__GNUC__) && (__GNUC__ >= 12))) && ((defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 15) || (defined(__GLIBCXX__) &&  _GLIBCXX_RELEASE >= 12))) || (!defined(__clang__) && !defined(__GNUC__)))
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
		#define PLF_CPP20_SUPPORT
	#endif
#endif


#define PLF_TYPE_BITWIDTH (sizeof(storage_type) * 8)
#define PLF_BITSET_SIZE_BYTES ((total_size + 7) / (sizeof(unsigned char) * 8)) // ie. round up to nearest byte
#define PLF_ARRAY_CAPACITY ((total_size + PLF_TYPE_BITWIDTH - 1) / PLF_TYPE_BITWIDTH) // ie. round up to nearest unit of storage
#define PLF_ARRAY_END (buffer + PLF_ARRAY_CAPACITY)
#define PLF_ARRAY_CAPACITY_BYTES (PLF_ARRAY_CAPACITY * sizeof(storage_type))
#define PLF_ARRAY_CAPACITY_BITS (PLF_ARRAY_CAPACITY_BYTES * 8)


#include <cmath> // log10
#include <cstring>	// memset, memcmp, size_t
#include <string>	// std::basic_string
#include <stdexcept> // std::out_of_range
#include <limits>  // std::numeric_limits
#include <bit>  // std::pop_count, std::countr_one, std::countr_zero
#include <ostream>


namespace plf
{


template<std::size_t total_size, typename storage_type = std::size_t, bool hardened = false>
class bitset
{
private:
	storage_type buffer[PLF_ARRAY_CAPACITY];


	// These two function calls should be optimized out by the compiler (under C++20) if total_size is a multiple of storage_type bitwidth, but if the "if" statement can't be constexpr due to lack of C++20 support, avoid the CPU penalty of the branch instruction and just perform the operation anyway. The idea is that there may be some remainder in the final storage_type which is unused in the bitset. By default we keep this at 0 for all bits, however some operations require them to be 1 in order to perform optimally. For those operations we set the remainder (overflow) to 1, then back to 0 at the end of the function:

	PLF_CONSTFUNC void set_overflow_to_one() PLF_NOEXCEPT
	{ // If total_size < array bit capacity, set all bits > size to 1
		#ifdef PLF_CPP20_SUPPORT
			if constexpr (total_size % PLF_TYPE_BITWIDTH != 0)
		#endif
		{
			buffer[PLF_ARRAY_CAPACITY - 1] |= std::numeric_limits<storage_type>::max() << (PLF_TYPE_BITWIDTH - (PLF_ARRAY_CAPACITY_BITS - total_size));
		}
	}



	PLF_CONSTFUNC void set_overflow_to_zero() PLF_NOEXCEPT
	{ // If total_size < array bit capacity, set all bits > size to 0
		#ifdef PLF_CPP20_SUPPORT
			if constexpr (total_size % PLF_TYPE_BITWIDTH != 0)
		#endif
		{
			buffer[PLF_ARRAY_CAPACITY - 1] &= std::numeric_limits<storage_type>::max() >> (PLF_ARRAY_CAPACITY_BITS - total_size);
		}
	}



public:

	PLF_CONSTFUNC bitset() PLF_NOEXCEPT
	{
		reset();
	}



	PLF_CONSTFUNC bitset(const bitset &source) PLF_NOEXCEPT
	{
		std::memcpy(static_cast<void *>(buffer), static_cast<const void *>(source.buffer), PLF_ARRAY_CAPACITY_BYTES); // Note: we want to copy any zero'd bits in the source overflow, since the array is currently uninitialized - unlike in operator ==.
	}



	PLF_CONSTFUNC bool operator [] (const std::size_t index) const
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if PLF_CONSTEXPR (hardened)
			{
				if (index >= total_size) throw std::out_of_range("Index larger than size of bitset");
			}
		#endif

		return static_cast<bool>((buffer[index / PLF_TYPE_BITWIDTH] >> (index % PLF_TYPE_BITWIDTH)) & storage_type(1));
	}



	PLF_CONSTFUNC bool test(const std::size_t index) const
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if PLF_CONSTEXPR (!hardened) // If hardened, will be checked in []
			{
				if (index >= total_size) throw std::out_of_range("Index larger than size of bitset");
			}
		#endif

		return operator [](index);
	}



	PLF_CONSTFUNC void set() PLF_NOEXCEPT
	{
		std::memset(static_cast<void *>(buffer), std::numeric_limits<unsigned char>::max(), PLF_BITSET_SIZE_BYTES);
		set_overflow_to_zero();
	}



	PLF_CONSTFUNC void set(const std::size_t index)
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if PLF_CONSTEXPR (hardened)
			{
				if (index >= total_size) throw std::out_of_range("Index larger than size of bitset");
			}
		#endif

		buffer[index / PLF_TYPE_BITWIDTH] |= storage_type(1) << (index % PLF_TYPE_BITWIDTH);
	}



	PLF_CONSTFUNC void set(const std::size_t index, const bool value)
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if PLF_CONSTEXPR (hardened)
			{
				if (index >= total_size) throw std::out_of_range("Index larger than size of bitset");
			}
		#endif

 		const std::size_t blockindex = index / PLF_TYPE_BITWIDTH, shift = index % PLF_TYPE_BITWIDTH;
		buffer[blockindex] = (buffer[blockindex] & ~(storage_type(1) << shift)) | (static_cast<storage_type>(value) << shift);
	}



	PLF_CONSTFUNC void set_range(const std::size_t begin, const std::size_t end)
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if PLF_CONSTEXPR (hardened)
			{
				if (begin >= total_size || end >= total_size) throw std::out_of_range("Index larger than size of bitset");
			}
		#endif

		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return;
		}

		const std::size_t begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);

		if (begin_type_index != end_type_index) // ie. if first and last bit to be set are not in the same storage_type unit
		{
			// Write first storage_type:
			buffer[begin_type_index] |= std::numeric_limits<storage_type>::max() << (begin % PLF_TYPE_BITWIDTH);

			// Fill all intermediate storage_type's (if any):
			std::memset(static_cast<void *>(buffer + begin_type_index + 1), std::numeric_limits<unsigned char>::max(), ((end_type_index - 1) - begin_type_index) * sizeof(storage_type));

			// Write last storage_type:
			buffer[end_type_index] |= std::numeric_limits<storage_type>::max() >> distance_to_end_storage;
		}
		else
		{
			buffer[begin_type_index] |= (std::numeric_limits<storage_type>::max() << (begin % PLF_TYPE_BITWIDTH)) & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage);
		}
	}



	PLF_CONSTFUNC void set_range(const std::size_t begin, const std::size_t end, const bool value)
	{
		if (value)
		{
			set_range(begin, end);
		}
		else
		{
			reset_range(begin, end);
		}
	}



	PLF_CONSTFUNC void reset() PLF_NOEXCEPT
	{
		std::memset(static_cast<void *>(buffer), 0, PLF_ARRAY_CAPACITY_BYTES);
	}



	PLF_CONSTFUNC void reset(const std::size_t index)
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if PLF_CONSTEXPR (hardened)
			{
				if (index >= total_size) throw std::out_of_range("Index larger than size of bitset");
			}
		#endif

		buffer[index / PLF_TYPE_BITWIDTH] &= ~(storage_type(1) << (index % PLF_TYPE_BITWIDTH));
	}



	PLF_CONSTFUNC void reset_range(const std::size_t begin, const std::size_t end)
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if PLF_CONSTEXPR (hardened)
			{
				if (begin >= total_size || end >= total_size) throw std::out_of_range("Index larger than size of bitset");
			}
		#endif

		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return;
		}

		const std::size_t begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);

		if (begin_type_index != end_type_index)
		{
			buffer[begin_type_index] &= ~(std::numeric_limits<storage_type>::max() << (begin % PLF_TYPE_BITWIDTH));
			std::memset(static_cast<void *>(buffer + begin_type_index + 1), 0, ((end_type_index - 1) - begin_type_index) * sizeof(storage_type));
			buffer[end_type_index] &= ~(std::numeric_limits<storage_type>::max() >> distance_to_end_storage);
		}
		else
		{
			buffer[begin_type_index] &= ~((std::numeric_limits<storage_type>::max() << (begin % PLF_TYPE_BITWIDTH)) & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage));
		}
	}



	PLF_CONSTFUNC void flip() PLF_NOEXCEPT
	{
		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] = ~buffer[current];
		set_overflow_to_zero();
	}



	PLF_CONSTFUNC void flip(const std::size_t index)
	{
		buffer[index / PLF_TYPE_BITWIDTH] ^= storage_type(1) << (index % PLF_TYPE_BITWIDTH);
	}



	PLF_CONSTFUNC bool all() PLF_NOEXCEPT
	{
		set_overflow_to_one();

		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
		{
			if (buffer[current] != std::numeric_limits<storage_type>::max())
			{
				set_overflow_to_zero();
				return false;
			}
		}

		set_overflow_to_zero();
		return true;
	}



	PLF_CONSTFUNC bool any() const PLF_NOEXCEPT
	{
		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) if (buffer[current] != 0) return true;
		return false;
	}



	PLF_CONSTFUNC bool none() const PLF_NOEXCEPT
	{
		return !any();
	}



	PLF_CONSTFUNC std::size_t count() const PLF_NOEXCEPT
	{
		std::size_t total = 0;

		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
		{
			#ifdef PLF_CPP20_SUPPORT
				total += std::popcount(buffer[current]); // leverage CPU intrinsics for faster performance
			#else
				for (storage_type value = buffer[current]; value; ++total) value &= value - 1; // Use kernighan's algorithm
			#endif
		}

		return total;
	}



	PLF_CONSTFUNC std::size_t first_one() const PLF_NOEXCEPT
	{
		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
		{
			if (buffer[current] != 0)
			{
				#ifdef PLF_CPP20_SUPPORT
					return (current * PLF_TYPE_BITWIDTH) + std::countr_zero(buffer[current]); // leverage CPU intrinsics for faster performance
				#else
					for (storage_type bit_index = 0, value = buffer[current]; /* will never reach an end condition */; ++bit_index)
					{
						if (value & (storage_type(1) << bit_index)) return (current * PLF_TYPE_BITWIDTH) + bit_index;
					}
				#endif
			}
		}

		return std::numeric_limits<std::size_t>::max();
	}



	PLF_CONSTFUNC std::size_t last_one() const PLF_NOEXCEPT
	{
		std::size_t current = PLF_ARRAY_CAPACITY;

		do
		{
			if (buffer[--current] != 0)
			{
				#ifdef PLF_CPP20_SUPPORT
					return (((current + 1) * PLF_TYPE_BITWIDTH) - std::countl_zero(buffer[current])) - 1;
				#else
					for (storage_type bit_index = PLF_TYPE_BITWIDTH - 1, value = buffer[current]; ; --bit_index)
					{
						if (value & (storage_type(1) << bit_index)) return (current * PLF_TYPE_BITWIDTH) + bit_index;
					}
				#endif
			}
		} while (current != 0);

		return std::numeric_limits<std::size_t>::max();
	}



	PLF_CONSTFUNC std::size_t first_zero() PLF_NOEXCEPT
	{
		set_overflow_to_one();

		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
		{
			if (buffer[current] != std::numeric_limits<storage_type>::max())
			{
				#ifdef PLF_CPP20_SUPPORT
					const std::size_t index = (current * PLF_TYPE_BITWIDTH) + std::countr_one(buffer[current]);
					set_overflow_to_zero();
					return index;
				#else
					for (storage_type bit_index = 0, value = buffer[current]; ; ++bit_index)
					{
						if (!(value & (storage_type(1) << bit_index)))
						{
							set_overflow_to_zero();
							return (current * PLF_TYPE_BITWIDTH) + bit_index;
						}
					}
				#endif
			}
		}

		set_overflow_to_zero();
		return std::numeric_limits<std::size_t>::max();
	}



	PLF_CONSTFUNC std::size_t last_zero() PLF_NOEXCEPT
	{
		set_overflow_to_one();
		std::size_t current = PLF_ARRAY_CAPACITY;

		do
		{
			if (buffer[--current] != std::numeric_limits<storage_type>::max())
			{
				#ifdef PLF_CPP20_SUPPORT
					const std::size_t index = (((current + 1) * PLF_TYPE_BITWIDTH) - std::countl_one(buffer[current])) - 1;
					set_overflow_to_zero();
					return index;
				#else
					for (storage_type bit_index = PLF_TYPE_BITWIDTH - 1, value = buffer[current]; ; --bit_index)
					{
						if (!(value & (storage_type(1) << bit_index)))
						{
							set_overflow_to_zero();
							return (current * PLF_TYPE_BITWIDTH) + bit_index;
						}
					}
				#endif
			}
		} while (current != 0);

		set_overflow_to_zero();
		return std::numeric_limits<std::size_t>::max();
	}



	PLF_CONSTFUNC void operator = (const bitset &source) PLF_NOEXCEPT
	{
		std::memcpy(static_cast<void *>(buffer), static_cast<const void *>(source.buffer), PLF_BITSET_SIZE_BYTES);
	}



 	PLF_CONSTFUNC bool operator == (const bitset &source) const PLF_NOEXCEPT
	{
		return std::memcmp(static_cast<const void *>(buffer), static_cast<const void *>(source.buffer), PLF_BITSET_SIZE_BYTES) == 0;
	}



 	PLF_CONSTFUNC bool operator != (const bitset &source) const PLF_NOEXCEPT
	{
		return !(*this == source);
	}



	PLF_CONSTFUNC std::size_t size() const PLF_NOEXCEPT
 	{
 		return total_size;
 	}



	PLF_CONSTFUNC bitset & operator &= (const bitset& source) PLF_NOEXCEPT
	{
		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] &= source.buffer[current];
		return *this;
	}



	PLF_CONSTFUNC bitset & operator |= (const bitset& source) PLF_NOEXCEPT
	{
		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] |= source.buffer[current];
		return *this;
	}



	PLF_CONSTFUNC bitset & operator ^= (const bitset& source) PLF_NOEXCEPT
	{
		for (std::size_t current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] ^= source.buffer[current];
		return *this;
	}



	PLF_CONSTFUNC bitset operator ~ () const
	{
		bitset<total_size, storage_type> temp(*this);
		temp.flip();
		return temp;
	}



	PLF_CONSTFUNC bitset & operator >>= (std::size_t shift_amount) PLF_NOEXCEPT
	{
		std::size_t end = PLF_ARRAY_CAPACITY - 1;

		if (shift_amount >= PLF_TYPE_BITWIDTH)
		{
			std::size_t current = 0;

			if (shift_amount < total_size)
			#ifdef PLF_CPP20_SUPPORT
				[[likely]]
			#endif
			{
				std::size_t current_source = shift_amount / PLF_TYPE_BITWIDTH;

				if ((shift_amount %= PLF_TYPE_BITWIDTH) != 0)
				{
					const storage_type shifter = PLF_TYPE_BITWIDTH - shift_amount;

					for (; current_source != end; ++current, ++current_source)
					{
						buffer[current] = (buffer[current_source] >> shift_amount) | (buffer[current_source + 1] << shifter);
					}

					buffer[current++] = buffer[end] >> shift_amount;
				}
				else
				{
					++end;

					for (; current_source != end; ++current, ++current_source)
					{
						buffer[current] = buffer[current_source];
					}
				}
			}

			std::memset(static_cast<void *>(buffer + current), 0, (PLF_ARRAY_CAPACITY - current) * sizeof(storage_type));
		}
		else if (shift_amount != 0)
		{
			const storage_type shifter = PLF_TYPE_BITWIDTH - shift_amount;

			for (std::size_t current = 0; current != end; ++current)
			{
				buffer[current] = (buffer[current] >> shift_amount) | (buffer[current + 1] << shifter);
			}

			buffer[end] >>= shift_amount;
		}

		return *this;
	}



	PLF_CONSTFUNC bitset operator >> (const std::size_t shift_amount) const
	{
		bitset<total_size, storage_type> temp(*this);
		return temp >>= shift_amount;
	}



	PLF_CONSTFUNC bitset & operator <<= (std::size_t shift_amount) PLF_NOEXCEPT
	{
		std::size_t current = PLF_ARRAY_CAPACITY;

		if (shift_amount < total_size)
		#ifdef PLF_CPP20_SUPPORT
			[[likely]]
		#endif
		{
			std::size_t current_source = PLF_ARRAY_CAPACITY - (shift_amount / PLF_TYPE_BITWIDTH);

			if ((shift_amount %= PLF_TYPE_BITWIDTH) != 0)
			{
				const storage_type shifter = PLF_TYPE_BITWIDTH - shift_amount;

				while (--current_source != 0)
				{
					buffer[--current] = (buffer[current_source - 1] >> shifter) | (buffer[current_source] << shift_amount);
				}

				buffer[--current] = buffer[current_source] << shift_amount;
			}
			else
			{
				do
				{
					buffer[--current] = buffer[--current_source];
				} while (current_source != 0);
			}
		}

		std::memset(static_cast<void *>(buffer), 0, current * sizeof(storage_type));

		set_overflow_to_zero();
		return *this;
	}



	PLF_CONSTFUNC bitset operator << (const std::size_t shift_amount) const
	{
		bitset<total_size, storage_type> temp(*this);
		return temp <<= shift_amount;
	}



	template <class char_type = char, class traits = std::char_traits<char_type>, class allocator_type = std::allocator<char_type> >
	PLF_CONSTFUNC std::basic_string<char_type, traits, allocator_type> to_string(const char_type zero = char_type('0'), char_type one = char_type('1')) const
	{
		one -= zero;
		std::basic_string<char_type, traits, allocator_type> temp(total_size, zero);

		for (std::size_t index = 0, end = PLF_ARRAY_CAPACITY; index != end; ++index)
		{
 			if (buffer[index] != 0)
 			{
 				const std::size_t string_index = index * PLF_TYPE_BITWIDTH;
 				const storage_type value = buffer[index];

				#ifdef PLF_CPP20_SUPPORT // Avoid the branch otherwise
					if constexpr (total_size % PLF_TYPE_BITWIDTH == 0)
					{
						for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end; ++subindex)
						{
							temp[total_size - (string_index + subindex + 1)] = zero + (((value >> subindex) & storage_type(1)) * one);
						}
					}
					else
				#endif
				{
					for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end && (string_index + subindex) != total_size; ++subindex)
					{
						temp[total_size - (string_index + subindex + 1)] = zero + (((value >> subindex) & storage_type(1)) * one);
					}
				}
			}
		}

		return temp;
	}



	template <class char_type = char, class traits = std::char_traits<char_type>, class allocator_type = std::allocator<char_type> >
	PLF_CONSTFUNC std::basic_string<char_type, traits, allocator_type> to_rstring(const char_type zero = char_type('0'), char_type one = char_type('1')) const
	{
		one -= zero;
		std::basic_string<char_type, traits, allocator_type> temp(total_size, zero);

		for (std::size_t index = 0, end = PLF_ARRAY_CAPACITY; index != end; ++index)
		{
 			if (buffer[index] != 0)
 			{
 				const std::size_t string_index = index * PLF_TYPE_BITWIDTH;
 				const storage_type value = buffer[index];

				#ifdef PLF_CPP20_SUPPORT
					if constexpr (total_size % PLF_TYPE_BITWIDTH == 0)
					{
						for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end; ++subindex)
						{
							temp[string_index + subindex] = zero + (((value >> subindex) & storage_type(1)) * one);
						}
					}
					else
				#endif
				{
					for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end && (string_index + subindex) != total_size; ++subindex)
					{
						temp[string_index + subindex] = zero + (((value >> subindex) & storage_type(1)) * one);
					}
				}
			}
		}

		return temp;
	}



	PLF_CONSTFUNC std::basic_string<char> to_srstring() const
	{
		char temp[total_size]; // Hella faster in benchmarking to write to this array on the stack then construct the basic_string on return, than to write on the heap or directly to the basic_string

		for (std::size_t index = 0, end = PLF_ARRAY_CAPACITY; index != end; ++index)
		{
 			if (buffer[index] != 0)
 			{
 				const std::size_t string_index = index * PLF_TYPE_BITWIDTH;
 				const storage_type value = buffer[index];

				#ifdef PLF_CPP20_SUPPORT
					if constexpr (total_size % PLF_TYPE_BITWIDTH == 0)
					{
						for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end; ++subindex)
						{
							temp[string_index + subindex] = ((value >> subindex) & storage_type(1)) + 48;
						}
					}
					else
				#endif
				{
					for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end && (string_index + subindex) != total_size; ++subindex)
					{
						temp[string_index + subindex] = ((value >> subindex) & storage_type(1)) + 48;
					}
				}
			}
		}

		return std::basic_string<char>(temp, total_size);
	}



private:

	template <typename number_type>
	PLF_CONSTFUNC number_type to_type() const
	{
		if (total_size > static_cast<std::size_t>(std::log10(static_cast<double>(std::numeric_limits<number_type>::max()))) + 1)
		{
			throw std::overflow_error("Bitset cannot be represented by this type due to the size of the bitset");
		}

		number_type value = 0;

		for (std::size_t index = 0, multiplier = 1; index != total_size; ++index, multiplier *= 10)
		{
			value += operator [](index) * multiplier;
		}

		return value;
	}



	template <typename number_type>
	PLF_CONSTFUNC number_type to_reverse_type() const
	{
		if (total_size > static_cast<std::size_t>(std::log10(static_cast<double>(std::numeric_limits<number_type>::max()))) + 1)
		{
			throw std::overflow_error("Bitset cannot be represented by this type due to the size of the bitset");
		}

		number_type value = 0;

		for (std::size_t reverse_index = total_size, multiplier = 1; reverse_index != 0; multiplier *= 10)
		{
			value += operator [](--reverse_index) * multiplier;
		}

		return value;
	}



public:

	PLF_CONSTFUNC unsigned long to_ulong() const
	{
		return to_type<unsigned long>();
	}



	PLF_CONSTFUNC unsigned long to_reverse_ulong() const
	{
		return to_reverse_type<unsigned long>();
	}



	#if (defined(__cplusplus) && __cplusplus >= 201103L) || _MSC_VER >= 1600
		PLF_CONSTFUNC unsigned long long to_ullong() const
		{
			return to_type<unsigned long long>();
		}



		PLF_CONSTFUNC unsigned long long to_reverse_ullong() const
		{
			return to_reverse_type<unsigned long long>();
		}
	#endif



	PLF_CONSTFUNC void swap (bitset &source) PLF_NOEXCEPT
	{
		*this ^= source;
		source ^= *this;
		*this ^= source;
	}
};


} // plf namespace


namespace std
{

	template <std::size_t total_size, typename storage_type>
	void swap (plf::bitset<total_size, storage_type> &a, plf::bitset<total_size, storage_type> &b) PLF_NOEXCEPT
	{
		a.swap(b);
	}



	template <std::size_t total_size, typename storage_type>
	ostream& operator << (ostream &os, const plf::bitset<total_size, storage_type> &bs)
	{
		return os << bs.to_string();
	}

}


#undef PLF_CPP20_SUPPORT
#undef PLF_CONSTFUNC
#undef PLF_CONSTEXPR
#undef PLF_NOEXCEPT
#undef PLF_EXCEPTIONS_SUPPORT
#undef PLF_TYPE_BITWIDTH
#undef PLF_BITSET_SIZE_BYTES
#undef PLF_ARRAY_CAPACITY
#undef PLF_ARRAY_END
#undef PLF_ARRAY_CAPACITY_BITS
#undef PLF_ARRAY_CAPACITY_BYTES

#endif // PLF_BITSET_H
