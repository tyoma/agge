#include <iostream>
#include <ut/test.h>

#ifdef ANDROID_NDK
	extern "C" {
		extern void *__dso_handle __attribute__((__visibility__ ("hidden")));
		void *__dso_handle;
	}
#endif

int main()
{
	using namespace std;
	using namespace ut;

	int result = 0;
	registry &r = registry_instance();

	for (registry::const_iterator i = r.tests_begin(); i != r.tests_end(); ++i)
	{
		test_result r = (*i)->execute();

		if (!r.initialized)
			cerr << "Failed to initialize: " << (*i)->fixture_name() << endl;
		else if (!r.passed)
			cerr << "Test failed: " << (*i)->fixture_name() << "::" << (*i)->name() << endl;
		else if (!r.terminated)
			cerr << "Test teardown failed: " << (*i)->fixture_name() << "::" << (*i)->name() << endl;
		else
		{
			cout << ".";
			continue;
		}
		--result;
	}

	cout << endl;
	cout << "Tests run: " << r.tests_count() << endl;
	cout << "Tests failed: " << -result << endl;
	cout << endl;

	return result;
}
