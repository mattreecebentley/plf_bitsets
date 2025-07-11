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
		std::basic_string<char> values_output1 = values.to_string();

		std::cout << "ostream output: " << values << "  Success.\n";

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
			const unsigned int num = rand() & 1;
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

		failpass("And test", and_values.count() == 0);


		plf::bitset<134> or_values = values;
		or_values |= flip_values;

		failpass("Or test", or_values.count() == 134);


 		plf::bitset<134> xor_values = and_values;
 		xor_values ^= or_values;

 		failpass("Xor test", xor_values.count() == 134);

		std::basic_string<char> values_output = values.to_string();
		std::basic_string<char> flip_output = flip_values.to_string();

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

		values_output = values.to_string('x', 'o');

		for (unsigned int index = 0; index != 134; ++index)
		{
			if ((values_output[index] == 'x') ? 0 : 1 != !(flip_output[index] - 48))
			{
				printf("Failed to_string comparison test");
				getchar();
				abort();
			}
		}

		message("Non-default output string comparison test passed");


		failpass("All test", or_values.all() && !values.all() && !flip_values.all() && !and_values.all());

		failpass("Any test", or_values.any() && values.any() && flip_values.any() && !and_values.any());

		failpass("None test", !or_values.none() && !values.none() && !flip_values.none() && and_values.none());

		and_values.set(100);
		and_values.set(131);
		or_values.reset(110);
		or_values.reset(132);

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


	printf("Press ENTER to quit");
	getchar();
	return 0;
}
