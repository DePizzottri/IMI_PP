#include <iostream>
#include <chrono>
#include <ppltasks.h>

using namespace std;
using namespace chrono;
using namespace concurrency;

int fib(int n)
{
	if (n == 1 || n  == 2)
		return 1;
	return fib(n - 1) + fib(n - 2);
}

int fibAsync1(int n)
{
	if (n == 1 || n == 2)
		return 1;

	if (n < 30)
		return fib(n);

	auto t1 = create_task([n]() {return fibAsync1(n - 1); });
	auto t2 = create_task([n]() {return fibAsync1(n - 2); });
	
	return t1.get() + t2.get();
}

int fibAsync2(int n)
{
	if (n == 1 || n == 2)
		return 1;

	if (n < 30)
		return fib(n);

	auto t1 = create_task([n]() {return fibAsync1(n - 1); });
	auto t2 = create_task([n]() {return fibAsync1(n - 2); });

	auto ts = { t1, t2 };
	auto r = when_all(ts.begin(), ts.end());
	auto rr = r.then([](vector<int> const& v) {return v[0] + v[1]; });
	return rr.get();
}

task<int> fibAsync3(int n)
{
	if (n == 1 || n == 2)
		create_task([]() {return 1; });

	if (n < 30)
		return create_task([n]() {return fib(n);});

	auto t1 = create_task([n]() {return fibAsync3(n - 1); });
	auto t2 = create_task([n]() {return fibAsync3(n - 2); });

	auto ts = { t1, t2 };
	return when_all(ts.begin(), ts.end()).
		then([](vector<int> const& v) 
	{
		return v[0] + v[1]; 
	});
}

int fibAsync4(int n)
{
	if (n == 1 || n == 2)
		return 1;

	if (n < 30)
		return fib(n);

	int res = 0;
	parallel_invoke(
		[&]() {res += fibAsync4(n - 1); },
		[&]() {res += fibAsync4(n - 2); }
	);

	return res;
}

int main()
{
	const int N = 45;

	{
		auto begin = chrono::system_clock::now();
		int f = fib(N);
		cout << "Fib = \t\t" << f << " ";
		cout << duration_cast<milliseconds> (system_clock::now() - begin).count() << endl;
	}

	{
		auto begin = chrono::system_clock::now();
		int f = fibAsync1(N);
		cout << "FibAsync1 = \t" << f << " ";
		cout << duration_cast<milliseconds> (system_clock::now() - begin).count() << endl;
	}

	{
		auto begin = chrono::system_clock::now();
		int f = fibAsync2(N);
		cout << "FibAsync2 = \t" << f << " ";
		cout << duration_cast<milliseconds> (system_clock::now() - begin).count() << endl;
	}

	{
		auto begin = chrono::system_clock::now();
		int f = fibAsync3(N).get();
		cout << "FibAsync3 = \t" << f << " ";
		cout << duration_cast<milliseconds> (system_clock::now() - begin).count() << endl;
	}

	{
		auto begin = chrono::system_clock::now();
		int f = fibAsync4(N);
		cout << "FibAsync4 = \t" << f << " ";
		cout << duration_cast<milliseconds> (system_clock::now() - begin).count() << endl;
	}
}
