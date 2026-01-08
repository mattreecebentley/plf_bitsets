// Copyright (c) 2025, Matthew Bentley (mattreecebentley@gmail.com) www.plflib.org

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



#ifndef PLF_BITSETC_H
#define PLF_BITSETC_H



#include "plf_bitsetb.h"
#include <memory> // std::allocator



#define PLF_TYPE_BITWIDTH (sizeof(storage_type) * 8)
#define PLF_ARRAY_CAPACITY ((total_size + PLF_TYPE_BITWIDTH - 1) / PLF_TYPE_BITWIDTH) // ie. round up to nearest unit of storage
#define PLF_ARRAY_CAPACITY_CALC(bitset_size) ((bitset_size + PLF_TYPE_BITWIDTH - 1) / PLF_TYPE_BITWIDTH)




namespace plf
{


template<typename storage_type = std::size_t, class allocator_type = std::allocator<storage_type>, bool hardened = false>
class bitsetc : public bitsetb<storage_type, hardened>, private allocator_type
{
private:
	typedef bitsetb<storage_type> base;

	// Just to save on typing:
	using base::buffer;
	using base::total_size;
	using base::check_source_size;


public:
	typedef std::size_t size_type;



	// Avoid compiler getting an ambiguous match with the std library operators:
	using base::operator ==;
	using base::operator !=;



	// Note: bitsetc is C++20-Only
	constexpr bitsetc(const size_type set_size):
		base(std::allocator_traits<allocator_type>::allocate(*this, PLF_ARRAY_CAPACITY_CALC(set_size), 0), set_size) // sets total_size in base as well
	{
		std::uninitialized_fill_n(buffer, PLF_ARRAY_CAPACITY, 0);
		base::reset();
	}



	constexpr bitsetc(const bitsetc &source):
		base(nullptr, source.total_size),
		allocator_type(std::allocator_traits<allocator_type>::select_on_container_copy_construction(source))
	{
		buffer = std::allocator_traits<allocator_type>::allocate(*this, PLF_ARRAY_CAPACITY, 0);
		std::uninitialized_copy_n(source.buffer, PLF_ARRAY_CAPACITY, buffer);
	}



	constexpr bitsetc(bitsetc &&source) noexcept:
		base(source.buffer, source.total_size),
		allocator_type(static_cast<allocator_type &>(source))
	{
		source.buffer = nullptr;
		source.total_size = 0;
	}



	constexpr bitsetc(bitsetc &&source, const std::type_identity_t<allocator_type> &alloc) noexcept:
		base(source.buffer, source.total_size),
		allocator_type(alloc)
	{
		if constexpr (!std::allocator_traits<allocator_type>::is_always_equal::value)
		{
			if (alloc != static_cast<allocator_type &>(source))
			{
				buffer = nullptr;
				*this = source;
				source.~bitsetc();
			}
		}

		source.buffer = nullptr;
		source.total_size = 0;
	}



	constexpr ~bitsetc() noexcept
	{
		std::allocator_traits<allocator_type>::deallocate(*this, buffer, PLF_ARRAY_CAPACITY);
	}



	constexpr void operator = (const bitsetc &source)
	{
		if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value)
		{
			if constexpr (!std::allocator_traits<allocator_type>::is_always_equal::value)
			{
				if (static_cast<allocator_type &>(*this) != static_cast<const allocator_type &>(source))
				{ // Deallocate existing block as source allocator is not necessarily able to do so
					std::allocator_traits<allocator_type>::deallocate(*this, buffer, PLF_ARRAY_CAPACITY);
					buffer = nullptr;
				}
			}

			static_cast<allocator_type &>(*this) = static_cast<const allocator_type &>(source);
		}

		if (PLF_ARRAY_CAPACITY != PLF_ARRAY_CAPACITY_CALC(source.total_size) || buffer == nullptr)
		{
			if (buffer != nullptr) std::allocator_traits<allocator_type>::deallocate(*this, buffer, PLF_ARRAY_CAPACITY);
			buffer = std::allocator_traits<allocator_type>::allocate(*this, PLF_ARRAY_CAPACITY_CALC(source.total_size), 0);
		}

		total_size = source.total_size;
		std::uninitialized_copy_n(source.buffer, PLF_ARRAY_CAPACITY, buffer); // If buffer hasn't been replaced, this is still fine as it is a trivial type
	}



	constexpr void operator = (bitsetc &&source) noexcept
	{
		std::allocator_traits<allocator_type>::deallocate(*this, buffer, PLF_ARRAY_CAPACITY);
		buffer = source.buffer;
		total_size = source.total_size;
		source.buffer = nullptr;
		source.total_size = 0;
	}



	constexpr void change_size(const size_type new_size)
 	{
		storage_type *new_buffer = std::allocator_traits<allocator_type>::allocate(*this, PLF_ARRAY_CAPACITY_CALC(new_size), buffer);
		std::uninitialized_copy_n(new_buffer, PLF_ARRAY_CAPACITY_CALC((new_size > total_size) ? total_size : new_size), buffer);
		base::set_overflow_to_zero();

		if (new_size > total_size) reset_range(total_size, new_size);

		std::allocator_traits<allocator_type>::deallocate(*this, buffer, PLF_ARRAY_CAPACITY);
		buffer = new_buffer;
		total_size = new_size;
		base::set_overflow_to_zero();
 	}



	constexpr void swap (bitsetc &source) noexcept
	{
		storage_type *swap_buffer = buffer;
		const size_type swap_size = total_size;
		buffer = source.buffer;
		total_size = source.total_size;
		source.buffer = swap_buffer;
		source.total_size = swap_size;
	}



	constexpr bitsetc operator & (const bitsetc& source) const noexcept
	{
		check_source_size(source.total_size);
		bitsetc result(total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) result.buffer[current] = buffer[current] & source.buffer[current];
		return result;
	}



	constexpr bitsetc operator | (const bitsetc& source) const noexcept
	{
		check_source_size(source.total_size);
		bitsetc result(total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) result.buffer[current] = buffer[current] | source.buffer[current];
		return result;
	}



	constexpr bitsetc operator ^ (const bitsetc& source) const noexcept
	{
		check_source_size(source.total_size);
		bitsetc result(total_size);
		for (size_type current = 0, end = PLF_ARRAY_CAPACITY; current != end; ++current) result.buffer[current] = buffer[current] ^ source.buffer[current];
		return result;
	}



};


} // plf namespace


namespace std
{

	template <typename storage_type, class allocator_type, bool hardened>
	void swap (plf::bitsetc<storage_type, allocator_type, hardened> &a, plf::bitsetc<storage_type, allocator_type, hardened> &b)
	{
		a.swap(b);
	}



	template <typename storage_type, class allocator_type, bool hardened>
	ostream& operator << (ostream &os, const plf::bitsetc<storage_type, allocator_type, hardened> &bs)
	{
		return os << bs.to_string();
	}

}

#undef PLF_TYPE_BITWIDTH
#undef PLF_ARRAY_CAPACITY
#undef PLF_ARRAY_CAPACITY_CALC

#endif // PLF_BITSETC_H
