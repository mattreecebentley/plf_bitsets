#include <cstdio>
#include "plf_bitsetb.h"


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
	std::size_t buffer1[134], buffer2[134], buffer3[134], buffer4[134], buffer5[134];
	plf::bitsetb<> values(buffer1, 134);

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


	values.reset();

	total = values.count();

	for (unsigned int index = 0; index != 134; ++index)
	{
		total2 += values[index];
	}

	failpass("Reset and count test", total == total2  && total2 == 0);

	values.reset();

	values.set_range(24, 32);

	failpass("set_range test 1", values.count() == 8);

	for (unsigned int counter = 0; counter != 1000; ++counter)
	{
		values.reset();
		const unsigned int begin = rand() % 134;
		const unsigned int end = begin + (rand() % (134 - begin));
		values.set_range(begin, end);
		if (values.count() != end - begin)
		{
			printf("Range-based set failed, counter == %u, begin == %u, end == %u, count == %u, range == %u\n%s", counter, begin, end, static_cast<unsigned int>(values.count()), end - begin, values.to_string().c_str());
			getchar();
			abort();
		}
	}


	message("set_range test 2: Pass");


	values.set();

	values.reset_range(24, 32);

	failpass("reset_range test 1", values.count() == 126);

	for (unsigned int counter = 0; counter != 1000; ++counter)
	{
		values.set();
		const unsigned int begin = rand() % 134;
		const unsigned int end = begin + (rand() % (134 - begin));
		values.reset_range(begin, end);
		if (values.count() != 134 - (end - begin))
		{
			printf("Range-based reset failed, counter == %u, begin == %u, end == %u, count == %u, range == %u\n%s", counter, begin, end, static_cast<unsigned int>(values.count()), 134 - (end - begin), values.to_string().c_str());
			getchar();
			abort();
		}
	}


	message("reset_range test 2: Pass");


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


	plf::bitsetb<> flip_values(buffer2, 134, values);
	flip_values.flip();

	failpass("Flip test", flip_values.count() == 134 - total2);


	plf::bitsetb<> and_values(buffer3, 134, values);
	and_values &= flip_values;

	failpass("And test", and_values.count() == 0);


	plf::bitsetb<> or_values(buffer4, 134, values);
	or_values |= flip_values;

	failpass("Or test", or_values.count() == 134);


	plf::bitsetb<> xor_values(buffer5, 134, and_values);
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

	const unsigned int bitset_size = 28;

	plf::bitsetb<unsigned char> shift_values(reinterpret_cast<unsigned char *>(buffer5), bitset_size);
	shift_values.reset();

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


	printf("Press ENTER to quit");
	getchar();
	return 0;
}
