#include <iostream>
#include <chrono>
#include <ppltasks.h>
#include <vector>
#include <array>

using namespace std;
using namespace chrono;
using namespace concurrency;

void print(vector<int> const& v)
{
	for (auto a : v)
		cout << a << ' ';
	cout << endl;
}

vector<int> merge(vector<int> const& v1, vector<int> const& v2)
{
	vector<int> ret;

	int i = 0, j = 0;
	while (i + j < v1.size() + v2.size())
	{
		while (j == v2.size() && i < v1.size() || (i < v1.size() && v1[i] <= v2[j]))
		{
			ret.push_back(v1[i]);
			i++;
		}

		while (i == v1.size() && j < v2.size() || (j < v2.size() && v2[j] <= v1[i]))
		{
			ret.push_back(v2[j]);
			j++;
		}
	}

	return ret;
}

vector<int> sort(vector<int> const& v)
{
	if (v.size() == 1 || v.size() == 0)
		return v;

	int m = v.size() / 2;

	auto v1 = sort(vector<int>(v.begin(), v.begin() + m));
	auto v2 = sort(vector<int>(v.begin() + m, v.end()));

	return merge(v1, v2);
}

vector<int> sortAsync(vector<int> const& v)
{
	if (v.size() == 1 || v.size() == 0)
		return v;

	int m = v.size() / 2;

	if (v.size() < 10000)
		return sort(v);

	auto t1 = create_task([&v, m](){
		return sortAsync(vector<int>(v.begin(), v.begin() + m));
	});

	auto t2 = create_task([&v, m](){
		return sortAsync(vector<int>(v.begin() + m, v.end()));
	});

	return merge(t1.get(), t2.get());
}

task<vector<int>> sortAsync1(vector<int> const& v)
{
	if (v.size() == 1 || v.size() == 0)
		return create_task([v]() {return v; });

	int m = v.size() / 2;

	if (v.size() < 10000)
		return create_task([v]() { return sort(v); });

	auto t1 = create_task([m, v]() {
		return sortAsync1(vector<int>(v.begin(), v.begin() + m));
	});

	auto t2 = create_task([v, m]() {
		return sortAsync1(vector<int>(v.begin() + m, v.end()));
	});

	auto t3 = { t1, t2 };

	//когда when_all принимает task'и векторов
	//он по выходу сливает все результаты в один
	return when_all(begin(t3), end(t3))
	.then([] (vector<int> const&  u) {
		//потому здесь мы получим 1 вектор, в который были слиты результаты выполнения  задач
		return merge({u.begin(), u.begin() + u.size() / 2}, {u.begin() + u.size() / 2 , u.end()});
	});
}

int main()
{
	vector<int> v;
	const int N = 100000;
	for (int i = 0; i < N; ++i)
		v.push_back(rand());
	{
		auto begin = chrono::system_clock::now();
		auto v1 = sort(v);
		cout << v1[0] << endl;
		cout << duration_cast<milliseconds> (system_clock::now() - begin).count() << endl;
	}

	{
		auto begin = chrono::system_clock::now();
		auto v1 = sortAsync(v);
		cout << v1[0] << endl;
		cout << duration_cast<milliseconds> (system_clock::now() - begin).count() << endl;
	}

	{
		auto begin = chrono::system_clock::now();
		auto v1 = sortAsync1(v);
		cout << v1.get()[0] << endl;
		cout << duration_cast<milliseconds> (system_clock::now() - begin).count() << endl;
	}
}
