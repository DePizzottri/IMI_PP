#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <functional>
#include <clocale>
#include <algorithm>
#include <Windows.h>
#include <chrono>

#include <ppl.h>

using namespace std;

vector<wstring> words;

void print(unordered_map<wstring, int> const& dict)
{
	map<int, vector<wstring>, greater<int> > freq;
	for (auto wk : dict)
	{
		freq[wk.second].push_back(wk.first);
	}

	int cnt = 0;
	for (auto e : freq)
	{
		if (cnt++ == 10)
			break;
		wcout << e.first << ": ";
		for (auto w : e.second)
		{
			wcout << w << ", ";
		}
		wcout << endl;
	}
}

void freq_dict()
{
	unordered_map<wstring, int> dict;

	for (auto w : words)
	{
		dict[w]++;
	}

	print(dict);
}

using namespace concurrency;

struct ReduceFunc : binary_function<unordered_map<wstring, size_t>,
	unordered_map<wstring, size_t>, unordered_map<wstring, size_t >>
{
	unordered_map<wstring, int> operator() (
		unordered_map<wstring, int> const& l, unordered_map<wstring, int> const& r) const {
		unordered_map<wstring, int> ret(l);		
		
		for (auto e : r)
			ret[e.first] += e.second;

		return ret;
	}
};

struct MapFunc
{
	unordered_map<wstring, int> operator()(vector<wstring> const& ws) const {
		unordered_map<wstring, int> map;

		for (auto w : ws)
		{
			map[w]++;
		}

		return map;
	}
};

const int factor = 5000000;


void freq_p()
{
	vector<vector<wstring>> grp;
	for (int i = 0; i < words.size() / factor + 1; ++i)
	{
		if (std::distance(words.begin() + i*factor, words.end()) >= factor)
			grp.push_back({ words.begin() + i*factor, words.begin() + (i + 1)*factor});
		else
			grp.push_back({ words.begin() + i*factor, words.end()});
	}

	//cout << grp[grp.size()-1].size() << endl;
	auto beg = chrono::high_resolution_clock::now();

	vector<unordered_map<wstring, int> > dict1(grp.size());

	parallel_transform(begin(grp), end(grp), begin(dict1), MapFunc());

	unordered_map<wstring, int> dict_res = 
		parallel_reduce(begin(dict1), end(dict1), unordered_map<wstring, int>(), ReduceFunc());
	auto en = chrono::high_resolution_clock::now();
	
	cout << chrono::duration_cast<chrono::milliseconds> (en - beg).count() << endl;

	print(dict_res);
}

int main()
{
	::SetConsoleCP(1251);
	::SetConsoleOutputCP(1251);
	wifstream fin("book.txt");

	while (fin)
	{
		wstring word;
		fin >> word;
		words.push_back(word);
	}

	cout << words.size() << endl;

	{
		auto begin = chrono::high_resolution_clock::now();
		freq_dict();
		auto end = chrono::high_resolution_clock::now();

		cout << chrono::duration_cast<chrono::milliseconds> (end - begin).count() << endl;
	}
	{
		auto begin = chrono::high_resolution_clock::now();
		freq_p();
		auto end = chrono::high_resolution_clock::now();
		cout << chrono::duration_cast<chrono::milliseconds> (end - begin).count() << endl;
	}
}