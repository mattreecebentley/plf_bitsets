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



#ifndef PLF_BITSETC_H
#define PLF_BITSETC_H



#include "plf_bitsetb.h"
#include <memory> // std::allocator


// Compiler-specific defines:

#define PLF_EXCEPTIONS_SUPPORT

#if ((defined(__clang__) || defined(__GNUC__)) && !defined(__EXCEPTIONS)) || (defined(_MSC_VER) && !defined(_CPPUNWIND))
	#undef PLF_EXCEPTIONS_SUPPORT
#endif


#define PLF_TYPE_BITWIDTH (sizeof(storage_type) * 8)
#define PLF_BITSET_SIZE_BYTES_CALC(bitset_size) ((bitset_size + 7) / (sizeof(unsigned char) * 8)) // ie. round up to nearest byte
#define PLF_ARRAY_CAPACITY_CALC(array_size) ((array_size + PLF_TYPE_BITWIDTH - 1) / PLF_TYPE_BITWIDTH) // ie. round up to nearest unit of storage




namespace plf
{


template<typename storage_type = std::size_t, class allocator_type = std::allocator<storage_type>>
class bitsetc : public bitsetb<storage_type>, private allocator_type
{
private:
	typedef bitsetb<storage_type> base;

	using base::buffer;
	using base::total_size;
	using base::set_overflow_to_one;
	using base::set_overflow_to_zero;


public:

	// Note: bitsetc is C++20-Only
	constexpr bitsetc(const std::size_t set_size):
		base(std::allocator_traits<allocator_type>::allocate(*this, PLF_ARRAY_CAPACITY_CALC(set_size), 0), set_size)
	{
		reset();
	}



	constexpr bitsetc(const bitsetc &source):
		base(std::allocator_traits<allocator_type>::allocate(*this, PLF_ARRAY_CAPACITY_CALC(source.total_size), 0), source.total_size),
		#if (defined(__cplusplus) && __cplusplus >= 201103L) || _MSC_VER >= 1700
			allocator_type(std::allocator_traits<allocator_type>::select_on_container_copy_construction(source))
		#else
			allocator_type(source)
		#endif
	{
		std::memcpy(static_cast<void *>(buffer), static_cast<const void *>(source.buffer), PLF_BITSET_SIZE_BYTES_CALC(source.total_size));
		set_overflow_to_zero(); // In case source.total_size != total_size
	}



	constexpr bitsetc(bitsetc &&source) noexcept:
		base(source.buffer, source.total_size)
	{
		source.buffer = NULL;
		source.total_size = 0;
	}


	constexpr ~bitsetc() noexcept
	{
		std::allocator_traits<allocator_type>::deallocate(*this, buffer, PLF_ARRAY_CAPACITY_CALC(total_size));
	}



	constexpr void change_size(const std::size_t new_size)
 	{
		storage_type *new_buffer = std::allocator_traits<allocator_type>::allocate(*this, PLF_ARRAY_CAPACITY_CALC(new_size), buffer);
		std::memcpy(static_cast<void *>(buffer), static_cast<const void *>(new_buffer), PLF_BITSET_SIZE_BYTES_CALC((new_size > total_size) ? total_size : new_size));

		if (new_size > total_size) reset_range(total_size, new_size);

		std::allocator_traits<allocator_type>::deallocate(*this, buffer, PLF_ARRAY_CAPACITY_CALC(total_size));
		buffer = new_buffer;
		total_size = new_size;
		set_overflow_to_zero();
 	}



	constexpr void swap (bitsetc &source) noexcept
	{
		storage_type *swap_buffer = buffer;
		const std::size_t swap_size = total_size;
		buffer = source.buffer;
		total_size = source.total_size;
		source.buffer = swap_buffer;
		source.total_size = swap_size;
	}



	using base::operator [];
	using base::test;
	using base::set;
	using base::set_range;
	using base::reset;
	using base::reset_range;
	using base::flip;
	using base::all;
	using base::none;
	using base::any;
	using base::count;
	using base::first_one;
	using base::first_zero;
	using base::last_one;
	using base::last_zero;
	using base::size;
	using base::operator >>=;
	using base::operator <<=;
	using base::to_string;
	using base::to_rstring;
	using base::to_srstring;
	using base::to_ulong;
	using base::to_rulong;
	using base::to_ullong;
	using base::to_rullong;
	using base::operator ==;
	using base::operator !=;
	using base::operator &=;
	using base::operator |=;
	using base::operator ^=;

};


} // plf namespace


namespace std
{

	template <typename storage_type>
	void swap (plf::bitsetc<storage_type> &a, plf::bitsetc<storage_type> &b)
	{
		a.swap(b);
	}



	template <typename storage_type>
	ostream& operator << (ostream &os, const plf::bitsetc<storage_type> &bs)
	{
		return os << bs.to_string();
	}

}

#undef PLF_EXCEPTIONS_SUPPORT
#undef PLF_TYPE_BITWIDTH
#undef PLF_BITSET_SIZE_BYTES_CALC
#undef PLF_ARRAY_CAPACITY_CALC

#endif // PLF_bitsetc_H
