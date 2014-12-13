#include <iostream>
#include <chrono>
#include <ppltasks.h>
#include <vector>
#include <array>

using namespace std;
using namespace chrono;
using namespace concurrency;

void sum(vector<int> const& v)
{
	vector<int> res(v.size() / 3);
	parallel_for(0, (int) v.size(), 3,
	[&v, &res](int b) {
		int s = 0;
		for (int i = b; i < b + 3; ++i)
			s += v[i];
		res[b / 3] = s;
	});
	for (auto e : res)
		cout << e << endl;
}

int main()
{
	vector<int> v{ 1, 2, 3, 45, 6, 7, 8, 9, 9, 67, 5, 4 };
	sum(v); //-?
}
