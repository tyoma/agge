#include <aggx/parallel.h>

using namespace std;

int main()
{
	aggx::parallel p(4);

	for (size_t n = 1000000; n; --n)
	{
		p.call([] (size_t threadid) {
		});
	}
}
