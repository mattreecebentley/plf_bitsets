// Copyright (c) 2025, Matthew Bentley (mattreecebentley@gmail.com) www.plflib.org

// Computing For Good License v1.0 (https://plflib.org/computing_for_good_license.htm):
// This code is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this code.
//
// Permission is granted to use this code by anyone and for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
// 1. 	The origin of this code must not be misrepresented; you must not claim that you wrote the original code. If you use this code in software, an acknowledgement in the product documentation would be appreciated but is not required.
// 2. 	Altered code versions must be plainly marked as such, and must not be misrepresented as being the original code.
// 3. 	This notice may not be removed or altered from any code distribution, including altered code versions.
// 4. 	This code and altered code versions may not be used by groups, companies, individuals or in software whose primary or partial purpose is to:
// 	 a.	 Promote addiction or intoxication.
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
#define PLF_CONSTFUNC


#if ((defined(__clang__) || defined(__GNUC__)) && !defined(__EXCEPTIONS)) || (defined(_MSC_VER) && !defined(_CPPUNWIND))
	#undef PLF_EXCEPTIONS_SUPPORT
	#include <exception> // std::terminate
#endif


#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
	#if _MSC_VER >= 1600
		#define PLF_MOVE_SEMANTICS_SUPPORT
	#endif
	#if _MSC_VER >= 1900
		#undef PLF_NOEXCEPT
		#define PLF_NOEXCEPT noexcept
	#endif
	#if defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L) && _MSC_VER >= 1929
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
		#define PLF_CPP20_SUPPORT
	#endif
#elif defined(__cplusplus) && __cplusplus >= 201103L // C++11 support, at least
	#if defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__clang__) // If compiler is GCC/G++
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || __GNUC__ > 4
			#define PLF_MOVE_SEMANTICS_SUPPORT
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept
		#endif
	#elif defined(__clang__)
		#if __has_feature(cxx_noexcept)
			#undef PLF_NOEXCEPT
			#define PLF_NOEXCEPT noexcept
		#endif
		#if __has_feature(cxx_rvalue_references) && !defined(_LIBCPP_HAS_NO_RVALUE_REFERENCES)
			#define PLF_MOVE_SEMANTICS_SUPPORT
		#endif
	#else // Assume noexcept support for other compilers
		#undef PLF_NOEXCEPT
		#define PLF_NOEXCEPT noexcept
	#endif
	// The following line is a little different from other plf:: containers because we need constexpr basic_string in order to make the to_string function constexpr:
	#if __cplusplus > 201704L && ((((defined(__clang__) && __clang_major__ >= 15) || (defined(__GNUC__) && (__GNUC__ >= 12))) && ((defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 15) || (defined(__GLIBCXX__) &&	_GLIBCXX_RELEASE >= 12))) || (!defined(__clang__) && !defined(__GNUC__)))
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
		#define PLF_CPP20_SUPPORT
	#endif
#endif


#define PLF_BITSET_SIZE_BYTES_CALC(bitset_size) ((bitset_size + 7) / (sizeof(unsigned char) * 8)) // ie. round up to nearest byte
#define PLF_TYPE_BITWIDTH (sizeof(storage_type) * 8)
#define PLF_ARRAY_CAPACITY ((total_size + PLF_TYPE_BITWIDTH - 1) / PLF_TYPE_BITWIDTH) // ie. round up to nearest unit of storage
#define PLF_ARRAY_CAPACITY_BYTES (PLF_ARRAY_CAPACITY * sizeof(storage_type))
#define PLF_ARRAY_CAPACITY_BITS (PLF_ARRAY_CAPACITY_BYTES * 8)


#include <cassert>
#include <cmath> // log10
#include <cstring> // memset, memcmp, size_t
#include <string> // std::basic_string
#include <stdexcept> // std::out_of_range
#include <limits>  // std::numeric_limits
#include <bit>  // std::pop_count, std::countr_one, std::countr_zero
#include <ostream>




namespace plf
{


template<class storage_type = std::size_t>
class bitsetb
{
private:

	template<typename store, class alloc> friend class bitsetc;

	storage_type *buffer;
	std::size_t total_size;

	// See plf::bitset code for explanation of these functions and their purpose:

	PLF_CONSTFUNC void set_overflow_to_one()
	{ // set all bits > size to 1
		buffer[PLF_ARRAY_CAPACITY - 1] |= std::numeric_limits<storage_type>::max() << (PLF_TYPE_BITWIDTH - (PLF_ARRAY_CAPACITY_BITS - total_size));
	}



	PLF_CONSTFUNC void set_overflow_to_zero()
	{ // set all bits > size to 0
		buffer[PLF_ARRAY_CAPACITY - 1] &= std::numeric_limits<storage_type>::max() >> (PLF_ARRAY_CAPACITY_BITS - total_size);
	}



public:
	typedef std::size_t size_type;

	PLF_CONSTFUNC bitsetb(storage_type * const supplied_buffer, const size_type size):
		buffer(supplied_buffer),
		total_size(size)
	{
		reset();
	}



	PLF_CONSTFUNC bitsetb(storage_type * const supplied_buffer, const size_type size, const bitsetb &source):
		buffer(supplied_buffer),
		total_size(size)
	{
		std::memcpy(static_cast<void *>(buffer), static_cast<const void *>(source.buffer), PLF_BITSET_SIZE_BYTES_CALC((source.total_size < total_size) ? source.total_size : total_size));
		set_overflow_to_zero(); // In case source.total_size != total_size
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
		return static_cast<bool>((buffer[index / PLF_TYPE_BITWIDTH] >> (index % PLF_TYPE_BITWIDTH)) & storage_type(1));
	}



	PLF_CONSTFUNC bool test(const size_type index) const
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if (index >= total_size) throw std::out_of_range("Index larger than size of bitset");
		#endif
		return operator [](index);
	}



	PLF_CONSTFUNC void set()
	{
		std::memset(static_cast<void *>(buffer), std::numeric_limits<unsigned char>::max(), PLF_BITSET_SIZE_BYTES_CALC(total_size));
		set_overflow_to_zero();
	}



	PLF_CONSTFUNC void set(const size_type index)
	{
		buffer[index / PLF_TYPE_BITWIDTH] |= storage_type(1) << (index % PLF_TYPE_BITWIDTH);
	}



	PLF_CONSTFUNC void set(const size_type index, const bool value)
	{
 		const size_type blockindex = index / PLF_TYPE_BITWIDTH, shift = index % PLF_TYPE_BITWIDTH;
		buffer[blockindex] = (buffer[blockindex] & ~(storage_type(1) << shift)) | (static_cast<storage_type>(value) << shift);
	}




	PLF_CONSTFUNC void set_range(const size_type begin, const size_type end)
	{
		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return;
		}

		const size_type begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);

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



	PLF_CONSTFUNC void reset()
	{
		std::memset(static_cast<void *>(buffer), 0, PLF_ARRAY_CAPACITY_BYTES);
	}



	PLF_CONSTFUNC void reset(const size_type index)
	{
		buffer[index / PLF_TYPE_BITWIDTH] &= ~(storage_type(1) << (index % PLF_TYPE_BITWIDTH));
	}



	PLF_CONSTFUNC void reset_range(const size_type begin, const size_type end)
	{
		if (begin == end)
		#ifdef PLF_CPP20_SUPPORT
			[[unlikely]]
		#endif
		{
			return;
		}

		const size_type begin_type_index = begin / PLF_TYPE_BITWIDTH, end_type_index = (end - 1) / PLF_TYPE_BITWIDTH, distance_to_end_storage = PLF_TYPE_BITWIDTH - (end % PLF_TYPE_BITWIDTH);

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



	PLF_CONSTFUNC void flip()
	{
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] = ~buffer[current];
		set_overflow_to_zero();
	}



	PLF_CONSTFUNC void flip(const size_type index)
	{
		buffer[index / PLF_TYPE_BITWIDTH] ^= storage_type(1) << (index % PLF_TYPE_BITWIDTH);
	}



	PLF_CONSTFUNC bool all()
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



	PLF_CONSTFUNC bool any() const
	{
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) if (buffer[current] != 0) return true;
		return false;
	}



	PLF_CONSTFUNC bool none() const
	{
		return !any();
	}



	PLF_CONSTFUNC size_type count() const PLF_NOEXCEPT
	{
		size_type total = 0;

		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
		{
			#ifdef PLF_CPP20_SUPPORT
				total += std::popcount(buffer[current]); // leverage CPU intrinsics for faster performance
			#else
				for (storage_type value = buffer[current]; value; ++total) value &= value - 1; // Use kernighan's algorithm
			#endif
		}

		return total;
	}



	PLF_CONSTFUNC size_type first_one() const
	{
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
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

		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type next_one(size_type index) const
	{
		if (index >= total_size - 1) return std::numeric_limits<size_type>::max();

		size_type current = index / PLF_TYPE_BITWIDTH;
		index = (index % PLF_TYPE_BITWIDTH) + 1; // convert to sub-index within word

		const storage_type current_word = buffer[current] >> index;

		if (index != PLF_TYPE_BITWIDTH && current_word != 0)
		{
			#ifdef PLF_CPP20_SUPPORT
				return (current * PLF_TYPE_BITWIDTH) + std::countr_zero(current_word) + index;
			#else
				for (storage_type bit_index = 0; ; ++bit_index)
				{
					if (current_word & (storage_type(1) << bit_index)) return (current * PLF_TYPE_BITWIDTH) + bit_index + index;
				}
			#endif
		}
		else
		{
			for (const size_type end = PLF_ARRAY_CAPACITY; ++current != end;)
			{
				if (buffer[current] != 0)
				{
					#ifdef PLF_CPP20_SUPPORT
						return (current * PLF_TYPE_BITWIDTH) + std::countr_zero(buffer[current]);
					#else
						for (storage_type bit_index = 0, value = buffer[current]; ; ++bit_index)
						{
							if (value & (storage_type(1) << bit_index)) return (current * PLF_TYPE_BITWIDTH) + bit_index;
						}
					#endif
				}
			}
		}

		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type last_one() const
	{
		size_type current = PLF_ARRAY_CAPACITY;

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

		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type prev_one(size_type index) const
	{
		if (index == 0 || index >= total_size) return std::numeric_limits<size_type>::max();

		size_type current = index / PLF_TYPE_BITWIDTH;
		index %= PLF_TYPE_BITWIDTH;

		const storage_type current_word = buffer[current] << (PLF_TYPE_BITWIDTH - index);

		if (index != 0 && current_word != 0)
		{
			#ifdef PLF_CPP20_SUPPORT
				return ((current * PLF_TYPE_BITWIDTH) + index - 1) - std::countl_zero(current_word);
			#else
				for (storage_type bit_index = PLF_TYPE_BITWIDTH - 1; ; --bit_index)
				{
					if (current_word & (storage_type(1) << bit_index)) return (current * PLF_TYPE_BITWIDTH) + bit_index - (PLF_TYPE_BITWIDTH - index);
				}
			#endif
		}
		else
		{
			while (current != 0)
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
			}
		 }

		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type first_zero()
	{
		set_overflow_to_one();

		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
		{
			if (buffer[current] != std::numeric_limits<storage_type>::max())
			{
				#ifdef PLF_CPP20_SUPPORT
					const size_type index = (current * PLF_TYPE_BITWIDTH) + std::countr_one(buffer[current]);
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
		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type next_zero(size_type index)
	{
		if (index >= total_size - 1) return std::numeric_limits<size_type>::max();

		set_overflow_to_one(); // even current word might be back word of the bitset

		size_type current = index / PLF_TYPE_BITWIDTH;
		index = (index % PLF_TYPE_BITWIDTH) + 1; // convert to sub-index within word, add one

		const storage_type current_word = buffer[current] | (std::numeric_limits<storage_type>::max() >> (PLF_TYPE_BITWIDTH - index)); // Set leading bits up-to-and-including the supplied index to 1

		if (index != PLF_TYPE_BITWIDTH && current_word != std::numeric_limits<storage_type>::max())
		{
			#ifdef PLF_CPP20_SUPPORT
				index = (current * PLF_TYPE_BITWIDTH) + std::countr_one(current_word);
				set_overflow_to_zero();
				return index;
			#else
				for (;; ++index)
				{
					if (!(current_word & (storage_type(1) << index)))
					{
						set_overflow_to_zero();
						return (current * PLF_TYPE_BITWIDTH) + index;
					}
				}
			#endif
		}
		else
		{
			for (const size_type end = PLF_ARRAY_CAPACITY; ++current != end;)
			{
				if (buffer[current] != std::numeric_limits<storage_type>::max())
				{
					#ifdef PLF_CPP20_SUPPORT
						index = (current * PLF_TYPE_BITWIDTH) + std::countr_one(buffer[current]);
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
		}

		set_overflow_to_zero();
		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type last_zero()
	{
		set_overflow_to_one();
		size_type current = PLF_ARRAY_CAPACITY;

		do
		{
			if (buffer[--current] != std::numeric_limits<storage_type>::max())
			{
				#ifdef PLF_CPP20_SUPPORT
					const size_type index = (((current + 1) * PLF_TYPE_BITWIDTH) - std::countl_one(buffer[current])) - 1;
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
		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC size_type prev_zero(size_type index)
	{
		if (index == 0 || index >= total_size) return std::numeric_limits<size_type>::max();

		set_overflow_to_one();

		size_type current = index / PLF_TYPE_BITWIDTH;
		index %= PLF_TYPE_BITWIDTH;

		const storage_type current_word = buffer[current] | (std::numeric_limits<storage_type>::max() << index);

		if (index != 0 && current_word != std::numeric_limits<storage_type>::max())
		{
			#ifdef PLF_CPP20_SUPPORT
				index = (((current + 1) * PLF_TYPE_BITWIDTH) - std::countl_one(buffer[current])) - 1;
				set_overflow_to_zero();
				return index;
			#else
				while (true)
				{
					if (!(current_word & (storage_type(1) << --index)))
					{
						set_overflow_to_zero();
						return (current * PLF_TYPE_BITWIDTH) + index;
					}
				}
			#endif
		}
		else
		{
			while (current != 0)
			{
				if (buffer[--current] != std::numeric_limits<storage_type>::max())
				{
					#ifdef PLF_CPP20_SUPPORT
						index = (((current + 1) * PLF_TYPE_BITWIDTH) - std::countl_one(buffer[current])) - 1;
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
			}
		}

		set_overflow_to_zero();
		return std::numeric_limits<size_type>::max();
	}



	PLF_CONSTFUNC void operator = (const bitsetb &source)
	{
		std::memcpy(static_cast<void *>(buffer), static_cast<const void *>(source.buffer), PLF_BITSET_SIZE_BYTES_CALC((source.total_size < total_size) ? source.total_size : total_size));
	}



	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		PLF_CONSTFUNC void operator = (bitsetb &&source)
		{
			buffer = source.buffer;
			total_size = source.total_size;
			source.buffer = NULL;
			source.total_size = 0;
		}
	#endif



 	PLF_CONSTFUNC bool operator == (const bitsetb &source) const
	{
		if (source.total_size != total_size) return false;
		return std::memcmp(static_cast<const void *>(buffer), static_cast<const void *>(source.buffer), PLF_BITSET_SIZE_BYTES_CALC(total_size)) == 0;
	}



 	PLF_CONSTFUNC bool operator != (const bitsetb &source) const
	{
		return !(*this == source);
	}



	PLF_CONSTFUNC size_type size() const PLF_NOEXCEPT
 	{
 		return total_size;
 	}



	PLF_CONSTFUNC void change_size(const size_type new_size)
 	{
		if (new_size > total_size) reset_range(total_size, new_size);
		total_size = new_size;
 	}



	PLF_CONSTFUNC bitsetb & operator &= (const bitsetb& source)
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if (source.total_size < total_size) throw std::length_error("Source smaller than *this, cannot interprocess.");
		#endif
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] &= source.buffer[current];
		return *this;
	}



	PLF_CONSTFUNC bitsetb & operator |= (const bitsetb& source)
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if (source.total_size < total_size) throw std::length_error("Source smaller than *this, cannot interprocess.");
		#endif
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] |= source.buffer[current];
		return *this;
	}



	PLF_CONSTFUNC bitsetb & operator ^= (const bitsetb& source)
	{
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if (source.total_size < total_size) throw std::length_error("Source smaller than *this, cannot interprocess.");
		#endif
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) buffer[current] ^= source.buffer[current];
		return *this;
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

			std::memset(static_cast<void *>(buffer + current), 0, (PLF_ARRAY_CAPACITY - current) * sizeof(storage_type));
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
	PLF_CONSTFUNC void shift_left_range (size_type shift_amount, const size_type first) PLF_NOEXCEPT
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

			for (size_type current = first_word_index; current != end; ++current)
			{
				buffer[current] = (buffer[current] >> shift_amount) | (buffer[current + 1] << shifter);
			}

			buffer[end] >>= shift_amount;
		}

		// Restore X bits to first word
		const storage_type remainder = first - (first_word_index * PLF_TYPE_BITWIDTH);
  		buffer[first_word_index] = (buffer[first_word_index] & (std::numeric_limits<storage_type>::max() << remainder)) | (first_word & (std::numeric_limits<storage_type>::max() >> (PLF_TYPE_BITWIDTH - remainder)));
	}



	// An optimization of the above for shifting by 1:
	PLF_CONSTFUNC void shift_left_range_one (const size_type first) PLF_NOEXCEPT
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
		const storage_type remainder = first - (first_word_index * PLF_TYPE_BITWIDTH);
  		buffer[first_word_index] = (buffer[first_word_index] & (std::numeric_limits<storage_type>::max() << remainder)) | (first_word & (std::numeric_limits<storage_type>::max() >> (PLF_TYPE_BITWIDTH - remainder)));
	}



	PLF_CONSTFUNC bitsetb & operator <<= (size_type shift_amount) PLF_NOEXCEPT
	{
		if (shift_amount >= PLF_TYPE_BITWIDTH)
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

			std::memset(static_cast<void *>(buffer), 0, current * sizeof(storage_type));
		}
		else if (shift_amount != 0)
		{
			const storage_type shifter = PLF_TYPE_BITWIDTH - shift_amount;
			storage_type shift_remainder = 0;

			for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current)
			{
				const storage_type temp = buffer[current];
				buffer[current] = (temp << shift_amount) | shift_remainder;
				shift_remainder = temp >> shifter;
			}
		}

		set_overflow_to_zero();
		return *this;
	}



	template <class char_type = char, class traits = std::char_traits<char_type>, class allocator_type = std::allocator<char_type> >
	PLF_CONSTFUNC std::basic_string<char_type, traits, allocator_type> to_string(const char_type zero = char_type('0'), char_type one = char_type('1')) const
	{
		one -= zero;
		std::basic_string<char_type, traits, allocator_type> temp(total_size, zero);

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



	template <class char_type = char, class traits = std::char_traits<char_type>, class allocator_type = std::allocator<char_type> >
	PLF_CONSTFUNC std::basic_string<char_type, traits, allocator_type> to_rstring(const char_type zero = char_type('0'), char_type one = char_type('1')) const
	{
		one -= zero;
		std::basic_string<char_type, traits, allocator_type> temp(total_size, zero);

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



	PLF_CONSTFUNC std::basic_string<char> to_srstring() const
	{
		char temp = new char[total_size];

		for (size_type index = 0, end = PLF_ARRAY_CAPACITY; index != end; ++index)
		{
 			if (buffer[index] != 0)
 			{
 				const size_type string_index = index * PLF_TYPE_BITWIDTH;
 				const storage_type value = buffer[index];

				for (storage_type subindex = 0, sub_end = PLF_TYPE_BITWIDTH; subindex != sub_end && (string_index + subindex) != total_size; ++subindex)
				{
					temp[string_index + subindex] = ((value >> subindex) & storage_type(1)) + 48;
				}
			}
		}

		std::basic_string<char> returner(temp, total_size);
		delete [] temp;
		return returner;
	}



private:

	template <typename number_type>
	PLF_CONSTFUNC number_type to_type() const
	{
		if (total_size > static_cast<size_type>(std::log10(static_cast<double>(std::numeric_limits<number_type>::max()))) + 1)
		{
			throw std::overflow_error("Bitset cannot be represented by this type due to the size of the bitset");
		}

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
		if (total_size > static_cast<size_type>(std::log10(static_cast<double>(std::numeric_limits<number_type>::max()))) + 1)
		{
			throw std::overflow_error("Bitset cannot be represented by this type due to the size of the bitset");
		}

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



	#if (defined(__cplusplus) && __cplusplus >= 201103L) || _MSC_VER >= 1600

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
		#ifdef PLF_EXCEPTIONS_SUPPORT
			if (source.total_size != total_size) throw std::length_error("Bitsetb's do not have the same size, cannot swap.");
		#endif
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) std::swap(buffer[current], source.buffer[current]);
	}
};


} // plf namespace


namespace std
{

	template <typename storage_type>
	void swap (plf::bitsetb<storage_type> &a, plf::bitsetb<storage_type> &b)
	{
		a.swap(b);
	}



	template <typename storage_type>
	ostream& operator << (ostream &os, const plf::bitsetb<storage_type> &bs)
	{
		return os << bs.to_string();
	}

}

#undef PLF_MOVE_SEMANTICS_SUPPORT
#undef PLF_CPP20_SUPPORT
#undef PLF_CONSTFUNC
#undef PLF_NOEXCEPT
#undef PLF_EXCEPTIONS_SUPPORT
#undef PLF_TYPE_BITWIDTH
#undef PLF_BITSET_SIZE_BYTES_CALC
#undef PLF_ARRAY_CAPACITY
#undef PLF_ARRAY_CAPACITY_BYTES
#undef PLF_ARRAY_CAPACITY_BITS

#endif // PLF_BITSETB_H
