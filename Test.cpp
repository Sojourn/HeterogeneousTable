#include <iostream>
#include <string>
#include <cassert>

#include "HeterogeneousTable.h"

static const float test_val = -3.33f;
static const char *test_str = "test";

void test_add();
void test_get();
void test_remove();
void test_query();

int main(int argc, char **argv)
{
	test_add();
	test_get();
	test_remove();
	test_query();

	return 0;
}

void test_add()
{
	HeterogeneousTable<size_t> db;

	assert(db.AddRow<float>(0, test_val) == test_val);
	assert(db.AddRow<std::string>(0, test_str) == test_str);

	assert(db.AddRow<float>(1, test_val) == test_val);
	assert(db.AddRow<std::string>(1, test_str) == test_str);
}

void test_get()
{
	HeterogeneousTable<size_t> db;

	float *val_ptr = &db.AddRow<float>(0, test_val);
	std::string *str_ptr = &db.AddRow<std::string>(0, test_str);

	assert(db.GetRow<float>(0) == test_val);
	assert(db.GetRow<std::string>(0) == test_str);
	
	assert(&db.GetRow<float>(0) == val_ptr);
	assert(&db.GetRow<std::string>(0) == str_ptr);
}

void test_remove()
{
	HeterogeneousTable<size_t> db;

	db.AddRow<float>(0, test_val);
	db.AddRow<std::string>(0, test_str);

	assert(db.HasRow<float>(0));
	assert(db.HasRow<std::string>(0));

	db.RemoveRow<float>(0);
	db.RemoveRow<std::string>(0);

	assert(!db.HasRow<float>(0));
	assert(!db.HasRow<std::string>(0));
}

void test_query()
{
	HeterogeneousTable<size_t> db;

	size_t sum = 0;
	for(size_t i = 0; i < 10; i++)
	{
		db.AddRow<size_t>(i, i);
		sum += i;
	}

	size_t accumulator = 0;
	db.Query<size_t>([&](size_t key, size_t value) {
		assert(key == value);
		accumulator += value;
	});

	assert(sum == accumulator);
}