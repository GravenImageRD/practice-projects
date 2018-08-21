#include <memory>
#include <vector>
#include <algorithm>

static const int THIS_IS_BULLSHIT = 0;

int asdfasdf(void)
{
	std::vector<int> whatever = { 10, 34, 234234 };
	for (int i : whatever)
	{

	}

	auto l = [&]() {
		whatever.push_back(5);
	};

	l();

	std::sort(whatever.begin(), whatever.end(), [](int a, int b) -> bool { return a > b; });


	return THIS_IS_BULLSHIT;
}