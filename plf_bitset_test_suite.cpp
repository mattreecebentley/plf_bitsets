#include "plf_tools.h"

#include <cstdio>
#include <iostream>
#include "plf_bitset.h"



void message(const char *message_text)
{
	printf("%s\n", message_text);
}


void failpass(const char *test_type, bool condition)
{
	printf("%s: ", test_type);

	if (condition)
	{
		printf("Pass\n");
	}
	else
	{
		printf("Fail. Press ENTER to quit.");
		getchar();
		abort();
	}
}



template <std::size_t total_size, typename storage_type>
void const_query_test(const char *test_type)
{
	plf::bitset<total_size, storage_type> values;
	const plf::bitset<total_size, storage_type> &const_values = values;
	// total_size must be at least 8. Bits 3 and total_size - 2 are reset, and also bit 'middle' if there is a middle storage_type:
	const std::size_t none = std::numeric_limits<std::size_t>::max(), storage_bits = sizeof(storage_type) * 8, middle = storage_bits + 5;
	const bool has_middle_word = (total_size - 2) / storage_bits >= 2; // ie. at least 3 storage_types, and total_size - 2 is in the 3rd or later one

	values.set();
	const bool all_set_ok = const_values.all() && const_values.all_range(0, total_size) && const_values.all_range(0, total_size - 1);
	const bool search_set_ok = const_values.first_zero() == none && const_values.last_zero() == none && const_values.next_zero(0) == none && const_values.prev_zero(total_size - 1) == none;
	const bool out_of_range_ok = const_values.next_zero(total_size) == none && const_values.prev_zero(total_size) == none;

	values.reset(3);
	values.reset(total_size - 2);
	if (has_middle_word) values.reset(middle);

	const std::size_t zero_after_3 = (has_middle_word) ? middle : total_size - 2, zero_before_last = (has_middle_word) ? middle : 3;

	const bool all_ok = !const_values.all() && const_values.count() == total_size - ((has_middle_word) ? 3 : 2);
	const bool all_range_ok = const_values.all_range(4, zero_after_3) && !const_values.all_range(4, total_size) && !const_values.all_range(0, total_size - 1) && !const_values.all_range(0, total_size) && const_values.all_range(total_size - 1, total_size);
	const bool all_range_middle_ok = !has_middle_word || (!const_values.all_range(4, total_size - 2) && !const_values.all_range(middle + 1, total_size) && const_values.all_range(middle + 1, total_size - 2)); // fails on a middle storage_type, then on the final one
	const bool first_last_zero_ok = const_values.first_zero() == 3 && const_values.last_zero() == total_size - 2;
	const bool next_zero_ok = const_values.next_zero(4) == zero_after_3 && const_values.next_zero(zero_after_3 + 1) == ((has_middle_word) ? total_size - 2 : none);
	const bool prev_zero_ok = const_values.prev_zero(total_size - 3) == zero_before_last && const_values.prev_zero(zero_before_last - 1) == ((has_middle_word) ? 3 : none) && const_values.prev_zero(2) == none;

	if (has_middle_word) values.set(middle); // so that searches have to skip a whole storage_type to find the next zero
	const bool skip_word_ok = !has_middle_word || (const_values.next_zero(4) == total_size - 2 && const_values.prev_zero(total_size - 3) == 3);

	failpass(test_type, all_set_ok && search_set_ok && out_of_range_ok && all_ok && all_range_ok && all_range_middle_ok && first_last_zero_ok && next_zero_ok && prev_zero_ok && skip_word_ok);
}





template <std::size_t total_size, typename storage_type>
void overflow_restore_test(const char *test_type)
{
	plf::bitset<total_size, storage_type> values;

	values.set();
	values.next_zero(total_size - 1);
	const bool next_zero_ok = values.count() == total_size;

	values.set();
	values.prev_zero(0);
	const bool prev_zero_ok = values.count() == total_size;

	failpass(test_type, next_zero_ok && prev_zero_ok);
}





template <std::size_t total_words, typename storage_type>
void range_end_aligned_test(const char *test_type)
{
	const std::size_t word = sizeof(storage_type) * 8, total_size = total_words * word;
	plf::bitset<total_words * sizeof(storage_type) * 8, storage_type> values;
	values.reset();

	values.set_range(0, word);
	const bool set_ok = values.count() == word && values.count_range(0, word) == word && values.all_range(0, word);

	values.set_range(word, total_size);
	const bool set_multi_ok = values.count() == total_size && values.count_range(1, total_size) == total_size - 1 && values.all_range(1, total_size);

	values.reset_range(0, word);
	values.reset_range(word + 1, total_size);
	const bool reset_ok = values.count() == 1 && !values.any_range(0, word) && !values.any_range(word + 1, total_size) && values.any_range(0, word + 1);

	failpass(test_type, set_ok && set_multi_ok && reset_ok);
}





template <typename storage_type>
void all_range_narrow_test(const char *test_type)
{
	const std::size_t word = sizeof(storage_type) * 8;
	plf::bitset<sizeof(storage_type) * 16, storage_type> values;
	values.set();

	const bool set_ok = values.all_range(0, word / 2) && values.all_range(1, word + 3);

	values.reset(word / 2 - 1);

	const bool reset_ok = !values.all_range(0, word / 2) && !values.all_range(1, word + 3) && values.all_range(word / 2, word + 3);

	failpass(test_type, set_ok && reset_ok);
}





template <typename storage_type>
void count_range_narrow_test(const char *test_type)
{
	const std::size_t word = sizeof(storage_type) * 8;
	plf::bitset<sizeof(storage_type) * 16, storage_type> values;
	values.set();

	const bool set_ok = values.count_range(1, word / 2) == word / 2 - 1 && values.count_range(1, word + 3) == word + 2;

	values.reset(word / 2 - 1);

	const bool reset_ok = values.count_range(1, word / 2) == word / 2 - 2 && values.count_range(1, word + 3) == word + 1;

	failpass(test_type, set_ok && reset_ok);
}





template <std::size_t total_size, typename storage_type>
void to_string_test(const char *test_type)
{
	plf::bitset<total_size, storage_type> values;
	for (std::size_t index = 0; index < total_size; index += 3) values.set(index);
	values.set(total_size - 1);

	const std::string forwards = values.to_string('.', '#'), reverse = values.to_rstring('.', '#');
	bool ok = forwards.size() == total_size && reverse.size() == total_size;

	for (std::size_t index = 0; ok && index != total_size; ++index)
	{
		const char expected = values[index] ? '#' : '.';
		ok = forwards[total_size - 1 - index] == expected && reverse[index] == expected;
	}

	failpass(test_type, ok);
}





template <std::size_t total_size, typename storage_type>
void exact_multiple_test(const char *test_type)
{
	plf::bitset<total_size, storage_type> values;
	values.reset();

	const bool reset_ok = !values.all() && values.first_zero() == 0 && values.last_zero() == total_size - 1 && values.count() == 0;

	values.set();

	const bool set_ok = values.all() && values.first_zero() == std::numeric_limits<std::size_t>::max() && values.count() == total_size;

	failpass(test_type, reset_ok && set_ok);
}



int main()
{
	{
		plf::bitset<134> values;

		unsigned int total = 0, total2 = 0;

		values.set();

		total = values.count();

		for (unsigned int index = 0; index != 134; ++index)
		{
			total2 += values[index];
		}

		failpass("Set and count test", total == total2);

		total = 0;
		total2 = 0;

		#ifdef PLF_CPP11_SUPPORT
			std::cout << "ostream output: " << values << "  Success.\n";
		#endif

		{
			plf::bitset<10> test;
			test.set();
			std::cout << "to_ulong output: " << test.to_ulong() << "  Success.\n";
		}

		values.reset();

		total = values.count();

		for (unsigned int index = 0; index != 134; ++index)
		{
			total2 += values[index];
		}

		failpass("Reset and count test", total == total2  && total2 == 0);

		// total_size an exact multiple of the storage_type bitwidth leaves no overflow bits, so the overflow manipulation must be a no-op:
		exact_multiple_test<sizeof(unsigned int) * 8, unsigned int>("Exact-multiple overflow test, one word/unsigned int");
		exact_multiple_test<sizeof(unsigned int) * 16, unsigned int>("Exact-multiple overflow test, two words/unsigned int");
		exact_multiple_test<sizeof(std::size_t) * 8, std::size_t>("Exact-multiple overflow test, one word/size_t");
		exact_multiple_test<sizeof(std::size_t) * 16, std::size_t>("Exact-multiple overflow test, two words/size_t");
		to_string_test<5, unsigned int>("to_string/to_rstring test, 5 bits/unsigned int");
		to_string_test<77, unsigned char>("to_string/to_rstring test, 77 bits/unsigned char");
		to_string_test<sizeof(std::size_t) * 8, std::size_t>("to_string/to_rstring test, one word/size_t");
		to_string_test<200, std::size_t>("to_string/to_rstring test, 200 bits/size_t");
		range_end_aligned_test<3, unsigned int>("Word-aligned range end test/unsigned int");
		range_end_aligned_test<3, std::size_t>("Word-aligned range end test/size_t");
		all_range_narrow_test<unsigned char>("all_range narrow storage test/unsigned char");
		all_range_narrow_test<unsigned short>("all_range narrow storage test/unsigned short");
		count_range_narrow_test<unsigned char>("count_range narrow storage test/unsigned char");
		count_range_narrow_test<unsigned short>("count_range narrow storage test/unsigned short");
		overflow_restore_test<2, unsigned int>("Overflow restore test, 2 bits/unsigned int");
		overflow_restore_test<sizeof(unsigned int) * 8 + 1, unsigned int>("Overflow restore test, one word plus one/unsigned int");
		overflow_restore_test<sizeof(std::size_t) * 16 - 1, std::size_t>("Overflow restore test, two words less one/size_t");
		const_query_test<sizeof(unsigned int) * 8 + 7, unsigned int>("Const query test, one word plus seven/unsigned int");
		const_query_test<sizeof(std::size_t) * 16 - 1, std::size_t>("Const query test, two words less one/size_t");
		const_query_test<sizeof(std::size_t) * 16, std::size_t>("Const query test, exact multiple/size_t");
		const_query_test<10, unsigned int>("Const query test, one word/unsigned int");
		const_query_test<sizeof(std::size_t) * 24 - 1, std::size_t>("Const query test, three words less one/size_t");
		const_query_test<sizeof(std::size_t) * 24, std::size_t>("Const query test, three words, exact multiple/size_t");

		{
			const unsigned int bitset_size = 584;
			plf::bitset<bitset_size> values2;

			values.set_range(24, 32);

			failpass("set_range test 1", values.count() == 8);

			for (unsigned int counter = 0; counter != 100; ++counter)
			{
				values2.reset();
				const unsigned int begin = rand() % bitset_size;
				const unsigned int end = begin + (rand() % (bitset_size - begin));
				values2.set_range(begin, end);

				if (values2.count() != end - begin || (begin != end && (values2[begin] != 1 || values2[end - 1] != 1)))
				{
					printf("Range-based set failed, counter == %u, begin == %u, end == %u, count == %u, range == %u\n%s", counter, begin, end, static_cast<unsigned int>(values2.count()), end - begin, values2.to_rstring().c_str());
					getchar();
					abort();
				}
			}

			message("set_range test 2: Pass");


			values2.set();

			values2.reset_range(24, 32);

			failpass("reset_range test 1", values2.count() == bitset_size - 8);

			for (unsigned int counter = 0; counter != 1000; ++counter)
			{
				values2.set();
				const unsigned int begin = rand() % bitset_size;
				const unsigned int end = begin + (rand() % (bitset_size - begin));
				values2.reset_range(begin, end);
				if (values2.count() != bitset_size - (end - begin) || (begin != end && (values2[begin] != 0 || values2[end - 1] != 0)))
				{
					printf("Range-based reset failed, counter == %u, begin == %u, end == %u, count == %u, range == %u\n%s", counter, begin, end, static_cast<unsigned int>(values2.count()), bitset_size - (end - begin), values2.to_rstring().c_str());
					getchar();
					abort();
				}
			}

			message("reset_range test 2: Pass");
		}


		values.reset();

		total = 0;
		total2 = 0;

		for (unsigned int index = 0; index != 134; ++index)
		{
			const bool num = static_cast<bool>(rand() & 1);
			values.set(index, num);
			total += num;
		}


		for (unsigned int index = 0; index != 134; ++index)
		{
			total2 += values[index];
		}

		failpass("Set test 1", total == total2);


		plf::bitset<134> flip_values = ~values;

		failpass("Flip test", flip_values.count() == 134 - total2);


		plf::bitset<134> and_values = values;
		and_values &= flip_values;
		plf::bitset<134> and_values2 = values & flip_values;

		failpass("And test", and_values.count() == 0 && and_values == and_values2);


		plf::bitset<134> or_values = values;
		or_values |= flip_values;
		plf::bitset<134> or_values2 = values | flip_values;

		failpass("Or test", or_values.count() == 134 && or_values == or_values2);


 		plf::bitset<134> xor_values = and_values;
 		xor_values ^= or_values;
		plf::bitset<134> xor_values2 = values ^ flip_values;

 		failpass("Xor test", xor_values.count() == 134 && xor_values == xor_values2);

		std::basic_string<char> values_output = values.to_rstring();
		std::basic_string<char> flip_output = flip_values.to_rstring();

		for (unsigned int index = 0; index != 134; ++index)
		{
			if (values_output[index] - 48 != !(flip_output[index] - 48))
			{
				printf("Failed to_string comparison test");
				getchar();
				abort();
			}
		}

		message("String comparison test passed");

		#ifdef PLF_CPP11_SUPPORT
			values_output = values.to_string('x', 'o');
			flip_output = flip_values.to_string('x', 'o');

			for (unsigned int index = 0; index != 134; ++index)
			{
				if (values_output[index] == flip_output[index])
				{
					printf("Failed to_string comparison test");
					getchar();
					abort();
				}
			}

			message("Non-default output string comparison test passed");
		#endif


		failpass("All test", or_values.all() && !values.all() && !flip_values.all() && !and_values.all());

		failpass("Any test", or_values.any() && values.any() && flip_values.any() && !and_values.any());

		failpass("None test", !or_values.none() && !values.none() && !flip_values.none() && and_values.none());

		and_values.set(100);
		and_values.set(131);
		or_values.reset(110);
		or_values.reset(132);

		failpass("count_range test", and_values.count_range(0, 99) == 0 && and_values.count_range(120, 134) == 1);
		failpass("any_range test", !and_values.any_range(0, 99) && and_values.any_range(120, 134));
		failpass("all_range test", !or_values.all_range(100, 134) && or_values.all_range(0, 99));
		failpass("none_range test", and_values.none_range(0, 99) && !and_values.none_range(120, 134));

		failpass("count_range test 2", and_values.count_range(34, 45) == 0 && and_values.count_range(130, 134) == 1);
		failpass("any_range test 2", !and_values.any_range(34, 45) && and_values.any_range(130, 134));
		failpass("all_range test 2", !or_values.all_range(90, 112) && or_values.all_range(34, 45));
		failpass("none_range test 2", and_values.none_range(90, 99) && !and_values.none_range(129, 134));

		failpass("all_range empty range test", !and_values.all_range(50, 50) && and_values.count() == 2);

		failpass("first_one test", and_values.first_one() == 100);
		failpass("next_one test", and_values.next_one(64) == 100);
		failpass("next_one test", and_values.next_one(54) == 100);
		failpass("next_one test 2", and_values.next_one(120) == 131);
		failpass("last_one test", and_values.last_one() == 131);
		failpass("prev_one test", and_values.prev_one(132) == 131);
		failpass("prev_one test 2", and_values.prev_one(128) == 100);
		failpass("first_zero test", or_values.first_zero() == 110);
		failpass("next_zero test", or_values.next_zero(64) == 110);
		failpass("next_zero test 2", or_values.next_zero(54) == 110);
		failpass("next_zero test 3", or_values.next_zero(128) == 132);
		failpass("prev_zero test", or_values.prev_zero(133) == 132);
		failpass("prev_zero test 2", or_values.prev_zero(129) == 110);
		failpass("last_zero test", or_values.last_zero() == 132);

		and_values.swap(or_values);

		failpass("Swap test", or_values.count() == 2 && and_values.count() == 132);

		std::swap(and_values, or_values);

		failpass("Swap test 2", or_values.count() == 132 && and_values.count() == 2);
	}

	{
		const unsigned int bitset_size = 584000;
		plf::bitset<bitset_size> values;

		for (unsigned int counter = 0; counter != 100000; ++counter)
		{
			const unsigned int start = (rand() % (bitset_size - 512)) + 128, end = start + (rand() % ((bitset_size - start) - 256)) + 128;
			const unsigned int test_range_start = start - (rand() % 128), test_range_end = end + (rand() % 128);
			values.set_range(start, end);
			const unsigned int counted_range = values.count_range(test_range_start, test_range_end);

			if (counted_range != end - start)
			{
				printf("Count_range bulk test failed, counter = %d, start = %d, end = %d, end - start = %d, count = %d, counted range = %d\n Press Enter to end", counter, start, end, end - start, static_cast<unsigned int>(values.count()), counted_range);
				getchar();
				abort();
			}

			if (values.none_range(test_range_start, test_range_end))
			{
				printf("None_range/any_range bulk test failed, counter = %d, start = %d, end = %d, end - start = %d, count = %d, counted range = %d\n Press Enter to end", counter, start, end, end - start, static_cast<unsigned int>(values.count()), counted_range);
				getchar();
				abort();
			}

			if (!values.all_range(start, end))
			{
				printf("All_range bulk test failed, counter = %d, start = %d, end = %d, end - start = %d, count = %d, counted range = %d\n Press Enter to end", counter, start, end, end - start, static_cast<unsigned int>(values.count()), counted_range);
				getchar();
				abort();
			}

			values.reset();
		}

		message("Bulk count_range/all_range/any_range/none_range tests passed");
	}


	{
		plf::bitset<500000> values;

		for (unsigned int counter = 0; counter != 40000; ++counter)
		{
			const unsigned int index = rand() % 500000;
			values.set(index);
			const unsigned int value = values.first_one();

			if (value != index)
			{
				std::cout << "Failed at counter " << counter << ", index " << index << ", value " << value << "\n";
				getchar();
				abort();
			}

			values.reset(index);
		}

		message("first_one series test passed");

		for (unsigned int counter = 0; counter != 40000; ++counter)
		{
			const unsigned int index = rand() % 400 + 10, index2 = index + rand() % 400, location = rand() % 810;
			values.set(index);
			values.set(index2);
			const std::size_t value = values.next_one(location);

			if (!((value == index && location <= index) || (value == index2 && location <= index2) || (value == std::numeric_limits<std::size_t>::max() && (location >= index && location >= index2))))
			{
				std::cout << "Failed at counter " << counter << ", index " << index << ", index2 " << index2 << ", value " << value << ", location " << location <<"\n";
				getchar();
				abort();
			}

			values.reset(index);
			values.reset(index2);
		}

		message("next_one series test passed");

		for (unsigned int counter = 0; counter != 40000; ++counter)
		{
			const unsigned int index = rand() % 400 + 100, index2 = index + rand() % 400 + 400, location = rand() % 1010 + index;
			values.set(index);
			values.set(index2);
			const std::size_t value = values.prev_one(location);

			if (!((value == index && location > index) || (value == index2 && location > index2) || (value == std::numeric_limits<std::size_t>::max() && (location <= index && location <= index2))))
			{
				std::cout << "Failed at counter " << counter << ", index " << index << ", index2 " << index2 << ", value " << value << ", location " << location <<"\n";
				getchar();
				abort();
			}

			values.reset(index);
			values.reset(index2);
		}

		message("prev_one series test passed");
	}

	{
		const unsigned int bitset_size = 584;
		plf::bitset<bitset_size> shift_values, shifted_values;

		shift_values.set();
		shift_values >>= 4;
		failpass(">>= test 1", shift_values.count() == bitset_size - 4);

		for (unsigned int index = 0; index != bitset_size; ++index)
		{
			shift_values.set(index, rand() & 1);
		}

		for (unsigned int shift_amount = 0; shift_amount != bitset_size + 1; ++shift_amount)
		{
			shifted_values = shift_values;
			shifted_values >>= shift_amount;

			for (unsigned int index = 0; index != bitset_size - shift_amount; ++index)
			{
				if (shift_values[index + shift_amount] != shifted_values[index])
				{
					printf("Failed >>= comparison test, shift_amount == %u\n", shift_amount);
					printf("%s\n\n%s\n\n", shift_values.to_rstring().c_str(), shifted_values.to_rstring().c_str());

					getchar();
					abort();
				}
			}

			for (unsigned int index = bitset_size - shift_amount; index != bitset_size; ++index)
			{
				if (shifted_values[index] != 0)
				{
					printf("Failed >>= comparison remainder test, shift_amount == %u\n", shift_amount);
					printf("%s\n\n%s\n\n", shift_values.to_rstring().c_str(), shifted_values.to_rstring().c_str());

					getchar();
					abort();
				}
			}

		}

		message(">>= multipass test success");


		shift_values.set();
		shift_values <<= 100;
		failpass("<<= test 1", shift_values.count() == bitset_size - 100);


		for (unsigned int shift_amount = 0; shift_amount != bitset_size + 1; ++shift_amount)
		{
			shifted_values = shift_values;
			shifted_values <<= shift_amount;

			for (unsigned int index = 0; index != shift_amount; ++index)
			{
				if (shifted_values[index] != 0)
				{
					printf("Failed <<= comparison remainder test, shift_amount == %u\n", shift_amount);
					printf("%s\n\n%s\n\n", shift_values.to_rstring().c_str(), shifted_values.to_rstring().c_str());

					getchar();
					abort();
				}
			}

			for (unsigned int index = shift_amount; index != bitset_size; ++index)
			{
				if (shift_values[index - shift_amount] != shifted_values[index])
				{
					printf("Failed <<= comparison test, shift_amount == %u\n", shift_amount);
					printf("%s\n\n%s\n\n", shift_values.to_rstring().c_str(), shifted_values.to_rstring().c_str());

					getchar();
					abort();
				}
			}

		}

		message(">>= multipass test success");
	}

	{
		const unsigned int bitset_size = 28;
		plf::bitset<bitset_size, unsigned char> shift_values;

		for (unsigned int index = 0; index != bitset_size; index += 2)
		{
			shift_values.set(index + 1);
		}

		printf("Before shift: %s\n", shift_values.to_rstring().c_str());

		shift_values.shift_left_range_one(5);

		printf("After shift: %s\n", shift_values.to_rstring().c_str());

		shift_values.shift_left_range(4, 4);

		printf("After shift: %s\n", shift_values.to_rstring().c_str());

		shift_values.shift_left_range(9, 7);

		printf("After shift: %s\n", shift_values.to_rstring().c_str());
	}


	{
		const std::size_t not_found = std::numeric_limits<std::size_t>::max();

		plf::bitset<1000, unsigned char> values;
		values.reset();
		values.set(900);
		failpass("prev_one not-found sentinel test", values.prev_one(3) == not_found);

		// The zero-searching functions reach countr_one/countl_one, which do not yet support a
		// storage_type narrower than int, so use the narrowest type they currently accept:
		plf::bitset<1000, unsigned int> wide_values;
		wide_values.set();
		wide_values.reset(900);
		failpass("prev_zero not-found sentinel test", wide_values.prev_zero(3) == not_found);
	}


	printf("Press ENTER to quit");
	getchar();
	return 0;
}
