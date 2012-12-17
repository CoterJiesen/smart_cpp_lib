// database_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>

#include <iostream>

#include <database\table_serialize\table.hpp>
#include <database\table_serialize\table_iterator.hpp>

int _tmain(int argc, _TCHAR* argv[])
{
	// ������
	std::string cols_name[] = 
	{
		"test1",
		"test2",
		"test3"
	};

	// ��Ԫ������
	std::string cell_data[] = 
	{
		"1",
		"1.01",
		"test_data",
	};

	database::table t;

	// �����к���
	for(size_t i = 0; i != _countof(cols_name); ++i)
		size_t col_i = t.insert_col(cols_name[i]);

	const size_t row_num = 5;
	for(size_t i = 0; i != row_num; ++i)
		t.insert_row();

	// д�뵥Ԫ������
	for(size_t i = 0; i != _countof(cols_name); ++i)
	{
		for(size_t j = 0; j != row_num; ++j)
		{
			t.set_cell(i, j, cell_data[i].c_str(), cell_data[i].size());
		}
	}

	// ���л�������
	std::vector<char> buffer;
	buffer.resize(t.size());

	// ���л���������
	database::ostream os(&buffer[0], buffer.size());
	os << t;

	// �����л�
	database::table tt;
	database::istream in(buffer.data(), buffer.size());
	in >> tt;

	// �Ƚϴ�ӡ
	assert(t == tt);
	std::copy(database::table_iterator(tt), database::table_iterator(), std::ostream_iterator<std::string>(std::cout, " "));
	
	
	return 0;
}

