void printJSON(json::value json, wstring t = L"")
{
	if (json.is_null())
	{
		wcout <<t<< L"NULL" << endl;
	}
	else if (json.is_array())
	{
		wcout << t <<"[" << endl;
		for (auto e : json.as_array())
		{
			printJSON(e, t + L"\t");
		}
		wcout << "]" << endl;
	}
	else if(json.is_object())
	{
		for (auto o: json.as_object())
		{
			wcout << t << o.first << endl;
			printJSON(o.second, t + L"\t");
		}
	}
	else if (json.is_integer())
	{
		wcout << t << json.as_integer() << endl;
	} 
	else if (json.is_string())
	{
		wcout << t << json.as_string() << endl;;
	}
}
