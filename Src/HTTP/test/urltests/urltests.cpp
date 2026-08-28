// urltests.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include <iostream>
#include "urlUtils.h"

using namespace std;
using namespace urlUtils;

int main(int argc,char *argv[])
{

	cout << "First we escape some URLs" << endl;
	
	urlQuery uq;
	uq.addQueryEntry(urlQueryEntry("full name","Neil Mark Radisch"));
	uq.addQueryEntry(urlQueryEntry("flag",""));
	uq.addQueryEntry(urlQueryEntry("wierd_stuff","!@#$%^&&&===\"\\'*()"));

	urlBuilder ub("http",urlAuthority("127.0.0.1","80","neil%%%radisch"),"/this/is/a/test",uq);
	string ubesc = ub.escape();
	cout << ubesc << endl;
	
	urlQuery uq2(urlQuery::parse(uq.escape()));
	cout << endl;
	
	cout << "Now we 'unescape' the query portion" << endl;
	cout << uq2.escape() << endl;
	cout << endl;
	
	cout << "Do they match? " << endl;
	cout << (uq2 == uq) << endl;
	cout << endl;
	
	cout << "Let's look at the raw data" << endl;
	for (urlQuery::const_iterator i = uq2.begin(); i != uq2.end(); ++i)
		{
		cout << "key = " << (*i).getKey() << endl;
		cout << "value = " << (*i).getValue() << endl;
		cout << endl;
		}
	cout << endl;
	
	return 0;
}

