// The code includes the most frequently used includes necessary to work with C++ REST SDK
#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace ::pplx;

using namespace web;
using namespace web::http;
using namespace web::http::client;

using namespace std;


//REST SDK
//Casablanca

task<vector<int> > getFriendsList(int id)
{
	//Строка запроса
	uri_builder builder;
	builder.append_query(U("user_id"), std::to_wstring(id));
	builder.append_query(U("count"), U("5"));
	auto query = builder.to_string();

	http_client client(U("https://api.vk.com/method/friends.get"));

	return client.request(methods::GET, query)
	.then([query](http_response response) {
		return response.extract_json();
	})
	.then([query](json::value json){
		auto field = *json.as_object().cbegin();

		if (field.first == U("error"))
		{
			std::wcerr << L"error " << query << std::endl;
			return vector<int> {};
		}

		vector<int> ret;
		for (auto i : field.second.as_array())
		{
			ret.push_back(i.as_integer());
		}

		return ret;
	});
}


#include <set>
#include <map>

typedef map<int, set<int>> graph;


graph bfs(int id)
{
	vector<int> allFriends;
	set<int> used;
	vector<int> curWave;
	vector<int> nextWave;

	curWave.push_back(id);
	used.insert(id);

	graph g;

	for (int i = 0; i < 2; ++i)
	{
		for (auto c : curWave) {
			auto next = getFriendsList(c);
			for (auto n : next.get()) {
				if (used.find(n) == used.end())
				{
					//c <-> n
					g[c].insert(n);
					g[n].insert(c);

					nextWave.push_back(n);
					used.insert(n);
				}
			}
		}

		for (auto f : nextWave)
			allFriends.push_back(f);

		curWave.swap(nextWave);
		nextWave.clear();
	}

	return g;
}

#include <fstream>
int main()
{
	auto g = bfs(1);

	ofstream fout("gr.dot");
	fout << "graph Durov {";
	for (auto v : g)
	{
		fout << v.first;
		for (auto i : v.second)
		{
			fout << " -- " << i;
		}
		fout << endl;
	}
	fout << "}";
}