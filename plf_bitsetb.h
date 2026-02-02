// Copyright (c) 2026, Matthew Bentley (mattreecebentley@gmail.com) www.plflib.org

// Computing For Good License v1.01 (https://plflib.org/computing_for_good_license.htm):
// This code is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this code.
//
// Permission is granted to use this code by anyone and for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
// 1. 	The origin of this code must not be misrepresented; you must not claim that you wrote the original code. If you use this code in software, an acknowledgement in the product documentation would be appreciated but is not required.
// 2. 	Altered code versions must be plainly marked as such, and must not be misrepresented as being the original code.
// 3. 	This notice may not be removed or altered from any code distribution, including altered code versions.
// 4. 	This code and altered code versions may not be used by groups, companies, individuals or in software whose primary or partial purpose is to:
// 	 a.	 Promote addiction or substance-based intoxication.
// 	 b.	 Cause harm to, or violate the rights of, other sentient beings.
// 	 c.	 Distribute, obtain or utilize software, media or other materials without the consent of the owners.
// 	 d.	 Deliberately spread misinformation or encourage dishonesty.
// 	 e.	 Pursue personal profit at the cost of broad-scale environmental harm.



#ifndef PLF_BITSETB_H
#define PLF_BITSETB_H


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
	#if _MSC_VER >= 1600
		#define PLF_MOVE_SEMANTICS_SUPPORT
	#endif

	#if _MSC_VER >= 1700
		#define PLF_TYPE_TRAITS_SUPPORT
		#define PLF_ALLOCATOR_TRAITS_SUPPORT
	#endif

	#if _MSC_VER >= 1900
		#undef PLF_NOEXCEPT
		#define PLF_NOEXCEPT noexcept(!user_supplied_buffer)
		#define PLF_CPP11_SUPPORT
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

	#if defined(_MSVC_LANG) && (_MSVC_LANG >= 202302L) && _MSC_VER >= 1944
		#define PLF_CONSTEVAL_SUPPORT
	#endif

#elif defined(__cplusplus) && __cplusplus >= 201103L // C++11 support, at least
	#if defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__clang__) // If compiler is GCC/G++
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || __GNUC__ > 4
			#define PLF_MOVE_SEMANTICS_SUPPORT
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept(!user_supplied_buffer)
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || __GNUC__ > 4
			#define PLF_CPP11_SUPPORT
			#define PLF_ALLOCATOR_TRAITS_SUPPORT
		#endif
	#elif defined(__clang__)
		#if !defined(__GLIBCXX__) && !defined(_LIBCPP_CXX03_LANG) && __clang_major__ >= 3
			#define PLF_ALLOCATOR_TRAITS_SUPPORT
			#define PLF_CPP11_SUPPORT
		#endif
		#if __has_feature(cxx_noexcept)
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept(!user_supplied_buffer)
		#endif
		#if __has_feature(cxx_rvalue_references) && !defined(_LIBCPP_HAS_NO_RVALUE_REFERENCES)
			#define PLF_MOVE_SEMANTICS_SUPPORT
		#endif
	#else // Assume support for other compilers
		#define PLF_ALLOCATOR_TRAITS_SUPPORT
		#undef PLF_NOEXCEPT
		#define PLF_NOEXCEPT noexcept(!user_supplied_buffer)
		#define PLF_CPP11_SUPPORT
	#endif

	#if __cplusplus >= 201703L && ((defined(__clang__) && ((__clang_major__ == 3 && __clang_minor__ == 9) || __clang_major__ > 3)) || (defined(__GNUC__) && __GNUC__ >= 7) || (!defined(__clang__) && !defined(__GNUC__))) // assume correct C++17 implementation for non-gcc/clang compilers
		#undef PLF_CONSTEXPR
		#define PLF_CONSTEXPR constexpr
	#endif

	// The following line is a little different from other plf:: containers because we need constexpr basic_string in order to make the to_string function constexpr:
	#if __cplusplus >= 202001L && ((((defined(__clang__) && __clang_major__ >= 15) || (defined(__GNUC__) && (__GNUC__ >= 12))) && ((defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 15) || (defined(__GLIBCXX__) &&	_GLIBCXX_RELEASE >= 12))) || (!defined(__clang__) && !defined(__GNUC__)))
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
		#define PLF_CPP20_SUPPORT
	#endif

	#if __cplusplus >= 202302L && ((defined(__clang__) && __clang_major__ >= 14) || (defined(__GNUC__) && (__GNUC__ >= 12)))
		#define PLF_CONSTEVAL_SUPPORT
	#endif
#endif



#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
	#define PLF_ALLOCATE(the_allocator, allocator_instance, size, hint)			std::allocator_traits<the_allocator>::allocate(allocator_instance, size, hint)
	#define PLF_DEALLOCATE(the_allocator, allocator_instance, location, size)	std::allocator_traits<the_allocator>::deallocate(allocator_instance, location, size)
#else
	#define PLF_ALLOCATE(the_allocator, allocator_instance, size, hint)			(allocator_instance).allocate(size, hint)
	#define PLF_DEALLOCATE(the_allocator, allocator_instance, location, size)	(allocator_instance).deallocate(location, size)
#endif



#define PLF_TYPE_BITWIDTH (sizeof(storage_type) * 8)
#define PLF_ARRAY_CAPACITY_CALC(bitset_size) ((bitset_size + PLF_TYPE_BITWIDTH - 1) / PLF_TYPE_BITWIDTH) // ie. round up to nearest unit of storage
#define PLF_ARRAY_CAPACITY ((total_size + PLF_TYPE_BITWIDTH - 1) / PLF_TYPE_BITWIDTH) // ie. round up to nearest unit of storage
#define PLF_ARRAY_CAPACITY_BITS (PLF_ARRAY_CAPACITY * PLF_TYPE_BITWIDTH)
#define PLF_ARRAY_CAPACITY_BYTES (PLF_ARRAY_CAPACITY * sizeof(storage_type))


#include <cmath> // log10
#include <cassert>
#include <memory> // std::uninitialized_copy, allocator
#include <string>	// std::basic_string
#include <stdexcept> // std::out_of_range
#include <limits>  // std::numeric_limits
#include <ostream>
#include <cstring>	// memset, size_t

#ifdef PLF_CPP20_SUPPORT
	#include <bit>  // std::pop_count, std::countr_one, std::countr_zero
	#include <algorithm> // std::equal
#endif


namespace plf
{


template<bool user_supplied_buffer = false, typename storage_type = std::size_t, class allocator_type = std::allocator<storage_type>, bool hardened = false>
class bitsetb : private allocator_type // Empty base class optimisation - inheriting allocator functions
{
private:
	typedef std::size_t size_type;

	storage_type *buffer;
	size_type total_size;

	// See plf::bitset code for explanation of these functions and their purpose:

	PLF_CONSTFUNC void set_overflow_to_one() PLF_NOEXCEPT
	{ // set all bits > size to 1
		buffer[PLF_ARRAY_CAPACITY - 1] |= std::numeric_limits<storage_type>::max() << (PLF_TYPE_BITWIDTH - (PLF_ARRAY_CAPACITY_BITS - total_size));
	}



	PLF_CONSTFUNC void set_overflow_to_zero() PLF_NOEXCEPT
	{ // set all bits > size to 0
		buffer[PLF_ARRAY_CAPACITY - 1] &= std::numeric_limits<storage_type>::max() >> (PLF_ARRAY_CAPACITY_BITS - total_size);
	}



	PLF_CONSTFUNC void check_index_is_within_size(const size_type index) const
	{
		if PLF_CONSTFUNC (hardened)
		{
			if (index >= total_size)
			{
				#ifdef PLF_EXCEPTIONS_SUPPORT
					throw std::out_of_range("Index larger than size of bitset");
				#else
					std::terminate();
				#endif
			}
		}
	}


public:

	PLF_CONSTFUNC bitsetb(const size_type size, storage_type * const supplied_buffer = NULL):
		buffer((supplied_buffer != NULL) ? supplied_buffer : PLF_ALLOCATE(allocator_type, *this, PLF_ARRAY_CAPACITY_CALC(size), 0)),
		total_size(size)
	{
		reset();
	}



	PLF_CONSTFUNC bitsetb(const bitsetb &source, storage_type * const supplied_buffer = NULL):
		#ifdef PLF_CPP11_SUPPORT
			allocator_type(std::allocator_traits<allocator_type>::select_on_container_copy_construction(source)),
		#else
			allocator_type(source),
		#endif
		buffer((supplied_buffer != NULL) ? supplied_buffer : PLF_ALLOCATE(allocator_type, *this, PLF_ARRAY_CAPACITY_CALC(source.total_size), 0)),
		total_size(source.total_size)
	{
		std::uninitialized_copy(source.buffer, source.buffer + PLF_ARRAY_CAPACITY_CALC(source.total_size), buffer);
		set_overflow_to_zero();
	}



	PLF_CONSTFUNC ~bitsetb() PLF_NOEXCEPT
	{
		if PLF_CONSTEXPR (!user_supplied_buffer)
		{
			PLF_DEALLOCATE(allocator_type, *this, buffer, PLF_ARRAY_CAPACITY_CALC(total_size));
		}
	}



	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		PLF_CONSTFUNC bitsetb(bitsetb &&source) PLF_NOEXCEPT:
			buffer(source.buffer),
			total_size(source.total_size)
		{
			source.buffer = NULL;
			source.total_size = 0;
		}
	#endif



	PLF_CONSTFUNC bool operator [] (const size_type index) const
	{
		if PLF_CONSTEXPR (hardened) check_index_is_within_size(index);
		return static_cast<bool>((buffer[index / PLF_TYPE_BITWIDTH] >> (index % PLF_TYPE_BITWIDTH)) & storage_type(1));
	}



	PLF_CONSTFUNC bool test(const size_type index) const
	{
		if PLF_CONSTEXPR (!hardened) check_index_is_within_size(index); // If hardened, will be checked in []
		return operator [](index);
	}



	PLF_CONSTFUNC void set() PLF_NOEXCEPT
	{
		#ifdef PLF_CONSTEVAL_SUPPORT
			if consteval
			{
				std::fill_n(buffer, PLF_ARRAY_CAPACITY, std::numeric_limits<storage_type>::max()); // fill_n is very slow compared to memset under gcc, particularly in debug mode, but memset isn't constexpr
			}
			else
		#endif
		{
			std::memset(static_cast<void *>(buffer), std::numeric_limits<unsigned char>::max(), PLF_ARRAY_CAPACITY_BYTES);
		}

		set_overflow_to_zero();
	}



	PLF_CONSTFUNC void set(const size_type index)
	{
		if PLF_CONSTFUNC (hardened) check_index_is_within_size(index);
		buffer[index / PLF_TYPE_BITWIDTH] |= storage_type(1) << (index % PLF_TYPE_BITWIDTH);
	}



	PLF_CONSTFUNC void set(const size_type index, const bool value)
	{
		if PLF_CONSTFUNC (hardened) check_index_is_within_size(index);

 		const size_type blockindex = index / PLF_TYPE_BITWIDTH, shift = index % PLF_TYPE_BITWIDTH;
		buffer[blockindex] = (buffer[blockindex] & ~(storage_type(1) << shift)) | (static_cast<storage_type>(value) << shift);
	}



	PLF_CONSTFUNC void set_range(const size_type begin, const size_type end)
	{
		if PLF_CONSTEXPR (hardened)
		{
			check_index_is_within_size(begin);
			check_index_is_within_size(end);
		}

		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return;
		}

		const size_type begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, begin_subindex = begin % PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);

		if (begin_type_index != end_type_index) // ie. if first and last bit to be set are not in the same storage_type unit
		{
			// Write first storage_type:
			buffer[begin_type_index] |= std::numeric_limits<storage_type>::max() << begin_subindex;

			// Fill all intermediate storage_type's (if any):
			#ifdef PLF_CONSTEVAL_SUPPORT
				if consteval
				{
					std::fill_n(buffer + begin_type_index + 1, (end_type_index - 1) - begin_type_index, std::numeric_limits<storage_type>::max());
				}
				else
			#endif
			{
				std::memset(static_cast<void *>(buffer + begin_type_index + 1), std::numeric_limits<unsigned char>::max(), ((end_type_index - 1) - begin_type_index) * sizeof(storage_type));
			}

			// Write last storage_type:
			buffer[end_type_index] |= std::numeric_limits<storage_type>::max() >> distance_to_end_storage;
		}
		else
		{
			buffer[begin_type_index] |= (std::numeric_limits<storage_type>::max() << begin_subindex) & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage);
		}
	}



	PLF_CONSTFUNC void set_range(const size_type begin, const size_type end, const bool value)
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
		#ifdef PLF_CONSTEVAL_SUPPORT
			if consteval
			{
				std::fill_n(buffer, PLF_ARRAY_CAPACITY, 0);
			}
			else
		#endif
		{
			std::memset(static_cast<void *>(buffer), 0, PLF_ARRAY_CAPACITY_BYTES);
		}
	}



	PLF_CONSTFUNC void reset(const size_type index)
	{
		if PLF_CONSTFUNC (hardened) check_index_is_within_size(index);
		buffer[index / PLF_TYPE_BITWIDTH] &= ~(storage_type(1) << (index % PLF_TYPE_BITWIDTH));
	}



	PLF_CONSTFUNC void reset_range(const size_type begin, const size_type end)
	{
		if PLF_CONSTEXPR (hardened)
		{
			check_index_is_within_size(begin);
			check_index_is_within_size(end);
		}

		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return;
		}

		const size_type begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, begin_subindex = begin % PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);

		if (begin_type_index != end_type_index)
		{
			buffer[begin_type_index] &= ~(std::numeric_limits<storage_type>::max() << begin_subindex);

			#ifdef PLF_CONSTEVAL_SUPPORT
				if consteval
				{
					std::fill_n(buffer + begin_type_index + 1, (end_type_index - 1) - begin_type_index, 0);
				}
				else
			#endif
			{
				std::memset(static_cast<void *>(buffer + begin_type_index + 1), 0, ((end_type_index - 1) - begin_type_index) * sizeof(storage_type));
			}

			buffer[end_type_index] &= ~(std::numeric_limits<storage_type>::max() >> distance_to_end_storage);
		}
		else
		{
			buffer[begin_type_index] &= ~((std::numeric_limits<storage_type>::max() << begin_subindex) & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage));
		}
	}



	PLF_CONSTFUNC void flip() PLF_NOEXCEPT
	{
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] = ~buffer[current];
		set_overflow_to_zero();
	}



	PLF_CONSTFUNC void flip(const size_type index) PLF_NOEXCEPT
	{
		buffer[index / PLF_TYPE_BITWIDTH] ^= storage_type(1) << (index % PLF_TYPE_BITWIDTH);
	}



	PLF_CONSTFUNC bool all() PLF_NOEXCEPT
	{
		set_overflow_to_one();

		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
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



	PLF_CONSTFUNC bool all_range(const size_type begin, const size_type end)
	{
		set_overflow_to_one();

		if PLF_CONSTEXPR (hardened)
		{
			check_index_is_within_size(begin);
			check_index_is_within_size(end);
		}

		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return false;
		}

		const size_type begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, begin_subindex = begin % PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);

		if (begin_type_index != end_type_index) // ie. if first and last bit to be set are not in the same storage_type unit
		{
			// Check first storage_type:
			if ((buffer[begin_type_index] | ~(std::numeric_limits<storage_type>::max() << begin_subindex)) != std::numeric_limits<storage_type>::max())
			{
				set_overflow_to_zero();
				return false;
			}

			// Check all intermediate storage_type's (if any):
			for (size_type current = begin_type_index + 1; current != end_type_index; ++current)
			{
				if (buffer[current] != std::numeric_limits<storage_type>::max())
				{
					set_overflow_to_zero();
					return false;
				}
			}

			// Write last storage_type:
			if ((buffer[end_type_index] | ~(std::numeric_limits<storage_type>::max() >> distance_to_end_storage)) != std::numeric_limits<storage_type>::max())
			{
				set_overflow_to_zero();
				return false;
			}
		}
		else
		{
			if ((buffer[begin_type_index] | ~((std::numeric_limits<storage_type>::max() << begin_subindex) & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage))) != std::numeric_limits<storage_type>::max())
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
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) if (buffer[current] != 0) return true;
		return false;
	}



	PLF_CONSTFUNC bool any_range(const size_type begin, const size_type end) const
	{
		if PLF_CONSTEXPR (hardened)
		{
			check_index_is_within_size(begin);
			check_index_is_within_size(end);
		}

		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return false;
		}

		const size_type begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, begin_subindex = begin % PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);

		if (begin_type_index != end_type_index)
		{
			if ((buffer[begin_type_index] & (std::numeric_limits<storage_type>::max() << begin_subindex)) != 0) return true;

			for (size_type current = begin_type_index + 1; current != end_type_index; ++current)
			{
				if (buffer[current] != 0) return true;
			}

			if ((buffer[end_type_index] & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage)) != 0) return true;
		}
		else
		{
			if ((buffer[begin_type_index] & ((std::numeric_limits<storage_type>::max() << begin_subindex) & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage))) != 0) return true;
		}

		return false;
	}



	PLF_CONSTFUNC bool none() const PLF_NOEXCEPT
	{
		return !any();
	}



	PLF_CONSTFUNC bool none_range(const size_type begin, const size_type end) const
	{
		return !any_range(begin, end);
	}



private:

	static PLF_CONSTFUNC size_type count_word(storage_type value) PLF_NOEXCEPT
	{
		#ifdef PLF_CPP20_SUPPORT
			return std::popcount(value); // leverage CPU intrinsics for faster performance
		#else
			size_type total = 0;
			for (; value; ++total) value &= value - 1; // Use kernighan's algorithm
			return total;
		#endif
	}


public:

	PLF_CONSTFUNC size_type count() const PLF_NOEXCEPT
	{
		size_type total = 0;

		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
		{
			total += count_word(buffer[current]);
		}

		return total;
	}



	PLF_CONSTFUNC size_type count_range(const size_type begin, const size_type end) const
	{
		if PLF_CONSTEXPR (hardened)
		{
			check_index_is_within_size(begin);
			check_index_is_within_size(end);
		}

		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return 0;
		}

		const size_type begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, begin_subindex = begin % PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);
		size_type total = 0;

		if (begin_type_index != end_type_index) // ie. if first and last bit to be set are not in the same storage_type unit
		{
			// Count first storage_type:
			total = count_word(buffer[begin_type_index] & (std::numeric_limits<storage_type>::max() << begin_subindex));

			// Count all intermediate storage_type's (if any):
			for (size_type current = begin_type_index + 1; current != end_type_index; ++current)
			{
				total += count_word(buffer[current]);
			}

			// Count last storage_type:
			total += count_word(buffer[end_type_index] & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage));
			return total;
		}
		else
		{
			return count_word(buffer[begin_type_index] & ((std::numeric_limits<storage_type>::max() << begin_subindex) & (std::numeric_limits<storage_type>::max() >> distance_to_end_storage)));
		}
	}



private:

	PLF_CONSTFUNC size_type search_one_forwards(size_type word_index) const PLF_NOEXCEPT
	{
		for (const size_type end = PLF_ARRAY_CAPACITY; word_index != end; ++word_index)
		{
			if (buffer[word_index] != 0)
			{
				#ifdef PLF_CPP20_SUPPORT
					return (word_index * PLF_TYPE_BITWIDTH) + std::countr_zero(buffer[word_index]);
				#else
					for (storage_type bit_index = 0, value = buffer[word_index]; ; ++bit_index)
					{
						if (value & (storage_type(1) << bit_index)) return (word_index * PLF_TYPE_BITWIDTH) + bit_index;
					}
				#endif
			}
		}

		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type search_one_backwards(size_type word_index) const PLF_NOEXCEPT
	{
		while (word_index != 0)
		{
			if (buffer[--word_index] != 0)
			{
				#ifdef PLF_CPP20_SUPPORT
					return (((word_index + 1) * PLF_TYPE_BITWIDTH) - std::countl_zero(buffer[word_index])) - 1;
				#else
					for (storage_type bit_index = PLF_TYPE_BITWIDTH - 1, value = buffer[word_index]; ; --bit_index)
					{
						if (value & (storage_type(1) << bit_index)) return (word_index * PLF_TYPE_BITWIDTH) + bit_index;
					}
				#endif
			}
		}

		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type search_zero_forwards(size_type word_index) PLF_NOEXCEPT
	{
		for (const size_type end = PLF_ARRAY_CAPACITY; word_index != end; ++word_index)
		{
			if (buffer[word_index] != std::numeric_limits<storage_type>::max())
			{
				#ifdef PLF_CPP20_SUPPORT
					const size_type index = (word_index * PLF_TYPE_BITWIDTH) + std::countr_one(buffer[word_index]);
					set_overflow_to_zero();
					return index;
				#else
					for (storage_type bit_index = 0, value = buffer[word_index]; ; ++bit_index)
					{
						if (!(value & (storage_type(1) << bit_index)))
						{
							set_overflow_to_zero();
							return (word_index * PLF_TYPE_BITWIDTH) + bit_index;
						}
					}
				#endif
			}
		}

		set_overflow_to_zero();
		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type search_zero_backwards(size_type word_index) PLF_NOEXCEPT
	{
		while (word_index != 0)
		{
			if (buffer[--word_index] != std::numeric_limits<storage_type>::max())
			{
				#ifdef PLF_CPP20_SUPPORT
					const size_type index = (((word_index + 1) * PLF_TYPE_BITWIDTH) - std::countl_one(buffer[word_index])) - 1;
					set_overflow_to_zero();
					return index;
				#else
					for (storage_type bit_index = PLF_TYPE_BITWIDTH - 1, value = buffer[word_index]; ; --bit_index)
					{
						if (!(value & (storage_type(1) << bit_index)))
						{
							set_overflow_to_zero();
							return (word_index * PLF_TYPE_BITWIDTH) + bit_index;
						}
					}
				#endif
			}
		}

		set_overflow_to_zero();
		return std::numeric_limits<size_type>::max();
	}



public:

	PLF_CONSTFUNC size_type first_one() const PLF_NOEXCEPT
	{
		return search_one_forwards(0);
	}



	PLF_CONSTFUNC size_type next_one(size_type index) const
	{
		if (index >= total_size - 1) return std::numeric_limits<size_type>::max();

		size_type word_index = index / PLF_TYPE_BITWIDTH;
		index = (index % PLF_TYPE_BITWIDTH) + 1; // convert to sub-index within word + 1 for the shift
		const storage_type current_word = buffer[word_index] >> index;

		if (index != PLF_TYPE_BITWIDTH && current_word != 0) // Note: shifting by full bitwidth of type is undefined behaviour, so can't rely on word << 64 being zero
		{
			#ifdef PLF_CPP20_SUPPORT
				return (word_index * PLF_TYPE_BITWIDTH) + std::countr_zero(current_word) + index;
			#else
				for (storage_type bit_index = 0; ; ++bit_index)
				{
					if (current_word & (storage_type(1) << bit_index)) return (word_index * PLF_TYPE_BITWIDTH) + bit_index + index;
				}
			#endif
		}

		return search_one_forwards(++word_index);
	}



	PLF_CONSTFUNC size_type last_one() const PLF_NOEXCEPT
	{
		return search_one_backwards(PLF_ARRAY_CAPACITY);
	}



	PLF_CONSTFUNC size_type prev_one(size_type index) const
	{
		if (index == 0 || index >= total_size) return std::numeric_limits<size_type>::max();

		const size_type word_index = index / PLF_TYPE_BITWIDTH;
		index %= PLF_TYPE_BITWIDTH;

		const storage_type current_word = buffer[word_index] << (PLF_TYPE_BITWIDTH - index);

		if (index != 0 && current_word != 0)
		{
			#ifdef PLF_CPP20_SUPPORT
				return ((word_index * PLF_TYPE_BITWIDTH) + index - 1) - std::countl_zero(current_word);
			#else
				for (storage_type bit_index = PLF_TYPE_BITWIDTH - 1; ; --bit_index)
				{
					if (current_word & (storage_type(1) << bit_index)) return (word_index * PLF_TYPE_BITWIDTH) + bit_index - (PLF_TYPE_BITWIDTH - index);
				}
			#endif
		}

		return search_one_backwards(word_index);
	}



	PLF_CONSTFUNC size_type first_zero() PLF_NOEXCEPT
	{
		set_overflow_to_one();
		return search_zero_forwards(0);
	}



	PLF_CONSTFUNC size_type next_zero(size_type index)
	{
		if (index >= total_size - 1) return std::numeric_limits<size_type>::max();

		set_overflow_to_one(); // even current word might be back word of the bitset

		size_type word_index = index / PLF_TYPE_BITWIDTH;
		index = (index % PLF_TYPE_BITWIDTH) + 1; // convert to sub-index within word

		const storage_type current_word = buffer[word_index] | (std::numeric_limits<storage_type>::max() >> (PLF_TYPE_BITWIDTH - index)); // Set leading bits up-to-and-including the supplied index to 1

		if (index != PLF_TYPE_BITWIDTH && current_word != std::numeric_limits<storage_type>::max())
		{
			#ifdef PLF_CPP20_SUPPORT
				index = (word_index * PLF_TYPE_BITWIDTH) + std::countr_one(current_word);
				set_overflow_to_zero();
				return index;
			#else
				for (;; ++index)
				{
					if (!(current_word & (storage_type(1) << index)))
					{
						set_overflow_to_zero();
						return (word_index * PLF_TYPE_BITWIDTH) + index;
					}
				}
			#endif
		}

		return search_zero_forwards(++word_index);
	}



	PLF_CONSTFUNC size_type last_zero() PLF_NOEXCEPT
	{
		set_overflow_to_one();
		return search_zero_backwards(PLF_ARRAY_CAPACITY);
	}



	PLF_CONSTFUNC size_type prev_zero(size_type index)
	{
		if (index == 0 || index >= total_size) return std::numeric_limits<size_type>::max();

		set_overflow_to_one();

		size_type word_index = index / PLF_TYPE_BITWIDTH;
		index %= PLF_TYPE_BITWIDTH;

		const storage_type current_word = buffer[word_index] | (std::numeric_limits<storage_type>::max() << index);

		if (index != 0 && current_word != std::numeric_limits<storage_type>::max())
		{
			#ifdef PLF_CPP20_SUPPORT
				index = (((word_index + 1) * PLF_TYPE_BITWIDTH) - std::countl_one(buffer[word_index])) - 1;
				set_overflow_to_zero();
				return index;
			#else
				while (true)
				{
					if (!(current_word & (storage_type(1) << --index)))
					{
						set_overflow_to_zero();
						return (word_index * PLF_TYPE_BITWIDTH) + index;
					}
				}
			#endif
		}

		return search_zero_backwards(word_index);
	}



	PLF_CONSTFUNC void operator = (const bitsetb &source)
	{
		check_source_size(source.total_size);
		std::copy(source.buffer, source.buffer + PLF_ARRAY_CAPACITY_CALC((source.total_size < total_size) ? source.total_size : total_size), buffer);
	}



	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		PLF_CONSTFUNC void operator = (bitsetb &&source) PLF_NOEXCEPT
		{
			assert(source.buffer != NULL);
			assert(&source != this);

			if PLF_CONSTEXPR (!user_supplied_buffer)
			{
				PLF_DEALLOCATE(allocator_type, *this, buffer, PLF_ARRAY_CAPACITY);
			}

			buffer = source.buffer;
			total_size = source.total_size;
			source.buffer = NULL;
			source.total_size = 0;
		}
	#endif



 	PLF_CONSTFUNC bool operator == (const bitsetb &source) const PLF_NOEXCEPT
	{
 		return (source.total_size == total_size) && std::equal(source.buffer, source.buffer + PLF_ARRAY_CAPACITY, buffer);
	}



 	PLF_CONSTFUNC bool operator != (const bitsetb &source) const PLF_NOEXCEPT
	{
		return !(*this == source);
	}



	PLF_CONSTFUNC size_type size() const PLF_NOEXCEPT
 	{
 		return total_size;
 	}



	PLF_CONSTFUNC void change_size(const size_type new_size)
 	{
		if PLF_CONSTEXPR (!user_supplied_buffer)
		{
			storage_type *new_buffer = PLF_ALLOCATE(allocator_type, *this, PLF_ARRAY_CAPACITY_CALC(new_size), buffer);
			std::uninitialized_copy(buffer, buffer + PLF_ARRAY_CAPACITY_CALC((new_size > total_size) ? total_size : new_size), new_buffer);
			PLF_DEALLOCATE(allocator_type, *this, buffer, PLF_ARRAY_CAPACITY);
			buffer = new_buffer;
			set_overflow_to_zero();
		}

		if (new_size > total_size) reset_range(total_size, new_size);
		total_size = new_size;
 	}


private:

	PLF_CONSTFUNC void check_source_size(const size_type source_size) const
	{
		if (source_size < total_size)
		{
			#ifdef PLF_EXCEPTIONS_SUPPORT
				throw std::length_error("Source smaller than *this, cannot interprocess.");
			#else
				std::terminate();
			#endif
		}
	}



public:

	PLF_CONSTFUNC bitsetb & operator &= (const bitsetb& source)
	{
		check_source_size(source.total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] &= source.buffer[current];
		return *this;
	}



	PLF_CONSTFUNC bitsetb<false, storage_type, allocator_type, hardened> operator & (const bitsetb& source) const
	{
		check_source_size(source.total_size);
		plf::bitsetb<false, storage_type, allocator_type, hardened> result(total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) result.buffer[current] = buffer[current] & source.buffer[current];
		return result;
	}



	PLF_CONSTFUNC bitsetb & operator |= (const bitsetb& source)
	{
		check_source_size(source.total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] |= source.buffer[current];
		return *this;
	}



	PLF_CONSTFUNC bitsetb<false, storage_type, allocator_type, hardened> operator | (const bitsetb& source) const
	{
		check_source_size(source.total_size);
		bitsetb<false, storage_type, allocator_type, hardened> result(total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) result.buffer[current] = buffer[current] | source.buffer[current];
		return result;
	}



	PLF_CONSTFUNC bitsetb & operator ^= (const bitsetb& source)
	{
		check_source_size(source.total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] ^= source.buffer[current];
		return *this;
	}



	PLF_CONSTFUNC bitsetb<false, storage_type, allocator_type, hardened> operator ^ (const bitsetb& source) const
	{
		check_source_size(source.total_size);
		bitsetb<false, storage_type, allocator_type, hardened> result(total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) result.buffer[current] = buffer[current] ^ source.buffer[current];
		return result;
	}



	PLF_CONSTFUNC bitsetb operator ~ () const
	{
		bitsetb<false, storage_type, allocator_type, hardened> result(*this);
		result.flip();
		return result;
	}



	PLF_CONSTFUNC bitsetb & operator >>= (size_type shift_amount) PLF_NOEXCEPT
	{
		size_type end = PLF_ARRAY_CAPACITY - 1;

		if (shift_amount >= PLF_TYPE_BITWIDTH)
		{
			size_type current = 0;

			if (shift_amount < total_size)
			#ifdef PLF_CPP20_SUPPORT
				[[likely]]
			#endif
			{
				size_type current_source = shift_amount / PLF_TYPE_BITWIDTH;

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

			#ifdef PLF_CONSTEVAL_SUPPORT
				if consteval
				{
					std::fill_n(buffer + current, PLF_ARRAY_CAPACITY - current, 0);
				}
				else
			#endif
			{
				std::memset(static_cast<void *>(buffer + current), 0, (PLF_ARRAY_CAPACITY - current) * sizeof(storage_type));
			}
		}
		else if (shift_amount != 0)
		{
			const storage_type shifter = PLF_TYPE_BITWIDTH - shift_amount;

			for (size_type current = 0; current != end; ++current)
			{
				buffer[current] = (buffer[current] >> shift_amount) | (buffer[current + 1] << shifter);
			}

			buffer[end] >>= shift_amount;
		}

		return *this;
	}



	// >>= but from a given index onwards only
	PLF_CONSTFUNC void shift_left_range (size_type shift_amount, const size_type first)
	{
		assert(first < total_size);

		size_type end = PLF_ARRAY_CAPACITY - 1;
		const size_type first_word_index = first / PLF_TYPE_BITWIDTH;
		const storage_type first_word = buffer[first_word_index];

		if (shift_amount >= PLF_TYPE_BITWIDTH)
		{
			size_type current = first_word_index;

			if (shift_amount < total_size - first)
			#ifdef PLF_CPP20_SUPPORT
				[[likely]]
			#endif
			{
				size_type current_source = first_word_index + (shift_amount / PLF_TYPE_BITWIDTH);

				if ((shift_amount %= PLF_TYPE_BITWIDTH) != 0)
				{
					const storage_type shifter = static_cast<storage_type>(PLF_TYPE_BITWIDTH - shift_amount);

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

			#ifdef PLF_CONSTEVAL_SUPPORT
				if consteval
				{
					std::fill_n(buffer + current, PLF_ARRAY_CAPACITY - current, 0);
				}
				else
			#endif
			{
				std::memset(static_cast<void *>(buffer + current), 0, (PLF_ARRAY_CAPACITY - current) * sizeof(storage_type));
			}
		}
		else if (shift_amount != 0)
		{
			const storage_type shifter = static_cast<storage_type>(PLF_TYPE_BITWIDTH - shift_amount);

			for (size_type current = first_word_index; current != end; ++current)
			{
				buffer[current] = (buffer[current] >> shift_amount) | (buffer[current + 1] << shifter);
			}

			buffer[end] >>= shift_amount;
		}

		// Restore X bits to first word
		const storage_type remainder = static_cast<storage_type>(first - (first_word_index * PLF_TYPE_BITWIDTH));
  		buffer[first_word_index] = (buffer[first_word_index] & (std::numeric_limits<storage_type>::max() << remainder)) | (first_word & (std::numeric_limits<storage_type>::max() >> (PLF_TYPE_BITWIDTH - remainder)));
	}



	// An optimization of the above for shifting by 1:
	PLF_CONSTFUNC void shift_left_range_one (const size_type first)
	{
		assert(first < total_size);

		const size_type end = PLF_ARRAY_CAPACITY - 1, first_word_index = first / PLF_TYPE_BITWIDTH;
		const storage_type first_word = buffer[first_word_index], shifter = PLF_TYPE_BITWIDTH - 1;

		for (size_type current = first_word_index; current != end; ++current)
		{
			buffer[current] = (buffer[current] >> 1) | (buffer[current + 1] << shifter);
		}

		buffer[end] >>= 1;

		// Restore X bits to first word
		const storage_type remainder = static_cast<storage_type>(first - (first_word_index * PLF_TYPE_BITWIDTH));
  		buffer[first_word_index] = (buffer[first_word_index] & (std::numeric_limits<storage_type>::max() << remainder)) | (first_word & (std::numeric_limits<storage_type>::max() >> (PLF_TYPE_BITWIDTH - remainder)));
	}



	PLF_CONSTFUNC bitsetb & operator <<= (size_type shift_amount) PLF_NOEXCEPT
	{
		size_type current = PLF_ARRAY_CAPACITY;

		if (shift_amount < total_size)
		#ifdef PLF_CPP20_SUPPORT
			[[likely]]
		#endif
		{
			size_type current_source = PLF_ARRAY_CAPACITY - (shift_amount / PLF_TYPE_BITWIDTH);

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

		#ifdef PLF_CONSTEVAL_SUPPORT
			if consteval
			{
				std::fill_n(buffer, current, 0);
			}
			else
		#endif
		{
			std::memset(static_cast<void *>(buffer), 0, current * sizeof(storage_type));
		}

		set_overflow_to_zero();
		return *this;
	}



	#ifdef PLF_CPP11_SUPPORT
		template <class char_type = char, class traits = std::char_traits<char_type>, class string_allocator_type = std::allocator<char_type> >
		PLF_CONSTFUNC std::basic_string<char_type, traits, string_allocator_type> to_string(const char_type zero = char_type('0'), char_type one = char_type('1')) const
		{
			std::basic_string<char_type, traits, string_allocator_type> temp(total_size, zero);
	#else
		PLF_CONSTFUNC std::basic_string<char> to_string(const char zero = char('0'), char one = char('1')) const
		{
			std::basic_string<char> temp(total_size, zero);
	#endif
		one -= zero;

		for (size_type index = 0, end = PLF_ARRAY_CAPACITY; index != end; ++index)
		{
 			if (buffer[index] != 0)
 			{
 				const size_type string_index = index * PLF_TYPE_BITWIDTH;
 				const storage_type value = buffer[index];

				for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end && (string_index + subindex) != total_size; ++subindex)
				{
					temp[total_size - (string_index + subindex + 1)] = zero + (((value >> subindex) & storage_type(1)) * one);
				}
			}
		}

		return temp;
	}




	#ifdef PLF_CPP11_SUPPORT
		template <class char_type = char, class traits = std::char_traits<char_type>, class string_allocator_type = std::allocator<char_type> >
		PLF_CONSTFUNC std::basic_string<char_type, traits, string_allocator_type> to_rstring(const char_type zero = char_type('0'), char_type one = char_type('1')) const
		{
			std::basic_string<char_type, traits, string_allocator_type> temp(total_size, zero);
	#else
		PLF_CONSTFUNC std::basic_string<char> to_rstring(const char zero = char('0'), char one = char('1')) const
		{
			std::basic_string<char> temp(total_size, zero);
	#endif
		one -= zero;

		for (size_type index = 0, end = PLF_ARRAY_CAPACITY; index != end; ++index)
		{
 			if (buffer[index] != 0)
 			{
 				const size_type string_index = index * PLF_TYPE_BITWIDTH;
 				const storage_type value = buffer[index];

				for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end && (string_index + subindex) != total_size; ++subindex)
				{
					temp[string_index + subindex] = zero + (((value >> subindex) & storage_type(1)) * one);
				}
			}
		}

		return temp;
	}



private:

	template <typename number_type>
	PLF_CONSTFUNC void check_bitset_representable() const
	{
		if (total_size > static_cast<size_type>(std::log10(static_cast<double>(std::numeric_limits<number_type>::max()))) + 1)
		{
			#ifdef PLF_EXCEPTIONS_SUPPORT
				throw std::overflow_error("Bitset cannot be represented by this type due to the size of the bitset");
			#else
				std::terminate();
			#endif
		}
	}


	template <typename number_type>
	PLF_CONSTFUNC number_type to_type() const
	{
		check_bitset_representable<number_type>();
		number_type value = 0;

		for (size_type index = 0, multiplier = 1; index != total_size; ++index, multiplier *= 10)
		{
			value += operator [](index) * multiplier;
		}

		return value;
	}



	template <typename number_type>
	PLF_CONSTFUNC number_type to_reverse_type() const
	{
		check_bitset_representable<number_type>();
		number_type value = 0;

		for (size_type reverse_index = total_size, multiplier = 1; reverse_index != 0; multiplier *= 10)
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



	PLF_CONSTFUNC unsigned long to_rulong() const
	{
		return to_reverse_type<unsigned long>();
	}



	#ifdef PLF_CPP11_SUPPORT
		PLF_CONSTFUNC unsigned long long to_ullong() const
		{
			return to_type<unsigned long long>();
		}



		PLF_CONSTFUNC unsigned long long to_rullong() const
		{
			return to_reverse_type<unsigned long long>();
		}
	#endif



	PLF_CONSTFUNC void swap(bitsetb &source)
	{
		if (source.total_size != total_size)
		{
			#ifdef PLF_EXCEPTIONS_SUPPORT
				throw std::length_error("Bitsetb's do not have the same size, cannot swap.");
			#else
				std::terminate();
			#endif
		}

		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) std::swap(buffer[current], source.buffer[current]);
	}

};


typedef bitsetb<false> bitsetc;


} // plf namespace


namespace std
{

	template <bool user_supplied, typename storage_type, class alloc, bool hardened>
	void swap (plf::bitsetb<user_supplied, storage_type, alloc, hardened> &a, plf::bitsetb<user_supplied, storage_type, alloc, hardened> &b)
	{
		a.swap(b);
	}



	template <bool user_supplied, typename storage_type, class alloc, bool hardened>
	ostream& operator << (ostream &os, const plf::bitsetb<user_supplied, storage_type, alloc, hardened> &bs)
	{
		return os << bs.to_string();
	}

}


#undef PLF_MOVE_SEMANTICS_SUPPORT
#undef PLF_CONSTEVAL_SUPPORT
#undef PLF_CPP11_SUPPORT
#undef PLF_CPP20_SUPPORT
#undef PLF_CONSTEXPR
#undef PLF_CONSTFUNC
#undef PLF_NOEXCEPT
#undef PLF_EXCEPTIONS_SUPPORT

#undef PLF_TYPE_BITWIDTH
#undef PLF_ARRAY_CAPACITY_CALC
#undef PLF_ARRAY_CAPACITY
#undef PLF_ARRAY_CAPACITY_BITS
#undef PLF_ARRAY_CAPACITY_BYTES

#endif // PLF_BITSETB_H
