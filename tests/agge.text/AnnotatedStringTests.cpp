#include <agge.text/annotated_string.h>

#include <tests/common/helpers.h>

#include <algorithm>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	template <typename T, typename AnnotationT>
	inline bool operator ==(const pair<basic_string<T>, const AnnotationT *> &expected,
		const typename agge::annotated_string<T, AnnotationT>::range &actual)
	{
		return expected.first == basic_string<T>(actual.begin, actual.end)
			&& !!expected.second == !!actual.annotation
			&& (!expected.second || *expected.second == *actual.annotation);
	}

	namespace tests
	{
		namespace
		{
			template <typename T, typename AnnotationT>
			class ranges_container
			{
			public:
				ranges_container(const annotated_string<T, AnnotationT> &underlying)
					: _underlying(&underlying)
				{	}

				typename annotated_string<T, AnnotationT>::const_iterator begin() const
				{	return _underlying->ranges_begin();	}

				typename annotated_string<T, AnnotationT>::const_iterator end() const
				{	return _underlying->ranges_end();	}

			private:
				const annotated_string<T, AnnotationT> *_underlying;
			};

			template <typename T, typename AnnotationT>
			ranges_container<T, AnnotationT> ranges(const annotated_string<T, AnnotationT> &from)
			{	return ranges_container<T, AnnotationT>(from);	}
		}

		begin_test_suite( AnnotatedStringTests )
			test( EmptyStringContainsNoSpans )
			{
				typedef annotated_string<wchar_t, string> container_t;

				// INIT
				container_t seq;

				// ACT / ASSERT
				assert_equal(seq.ranges_end(), seq.ranges_begin());
			}


			test( AppendingSimpleSequenceDoesNotAnnotateIt )
			{
				typedef annotated_string<wchar_t, string> container_t;

				// INIT
				container_t seq;

				// ACT
				seq += L"Lorem ipsum dolor sit amet";

				// ACT / ASSERT
				pair<wstring, const string *> reference1[] = {
					make_pair(L"Lorem ipsum dolor sit amet", nullptr),
				};

				assert_equal(reference1, ranges(seq));

				// ACT
				seq += L", consectetur adipisci elit";

				// ACT / ASSERT
				pair<wstring, const string *> reference2[] = {
					make_pair(L"Lorem ipsum dolor sit amet, consectetur adipisci elit", nullptr),
				};

				assert_equal(reference2, ranges(seq));
			}


			test( ConvertingFromStringProducesTrivialAnnotatedString )
			{
				typedef annotated_string<char, char> container_t;

				// INIT / ACT
				const container_t seq1("Lorem ipsum");

				// ACT / ASSERT
				pair<string, const char *> reference1[] = {
					make_pair("Lorem ipsum", nullptr),
				};

				assert_equal(reference1, ranges(seq1));

				// INIT / ACT
				const container_t seq2("dolor sit amet");

				// ACT / ASSERT
				pair<string, const char *> reference2[] = {
					make_pair("dolor sit amet", nullptr),
				};

				assert_equal(reference2, ranges(seq2));
			}


			test( AnnotatingInTheEndDoesNotAffectExistedSequence )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT
				seq += "aZ";
				seq.annotate(123456);

				// ACT / ASSERT
				pair<string, const int *> reference[] = {
					make_pair("aZ", nullptr),
				};

				assert_equal(reference, ranges(seq));
			}


			test( AnnotatingAtTheStartModifiesFollowingSequence )
			{
				typedef annotated_string<char, string> container_t;

				// INIT
				container_t seq;

				// ACT
				seq.annotate("zamazu");
				seq += "foobar";

				// ACT / ASSERT
				string m = "zamazu";
				pair<string, const string *> reference[] = {
					make_pair("foobar", &m),
				};

				assert_equal(reference, ranges(seq));
			}


			test( SettingModifierInTheMiddleModifiesFollowingSequence )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT
				seq += "foo ";
				seq.annotate(17);
				seq += "Bar";
				seq.annotate(191);
				seq += "BAZ";

				// ACT / ASSERT
				int annotations[] = {	17, 191,	};
				pair<string, const int *> reference[] = {
					make_pair("foo ", nullptr),
					make_pair("Bar", &annotations[0]),
					make_pair("BAZ", &annotations[1]),
				};

				assert_equal(reference, ranges(seq));
			}


			test( LastSetModifierTakesPrecedence )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT
				seq += "In a ";
				seq.annotate(100);
				seq.annotate(90);
				seq.annotate(32);
				seq += "free";
				seq.annotate(0);
				seq += " socicety political opponents survive";

				// ACT / ASSERT
				int annotations[] = {	32, 0,	};
				pair<string, const int *> reference[] = {
					make_pair("In a ", nullptr),
					make_pair("free", &annotations[0]),
					make_pair(" socicety political opponents survive", &annotations[1]),
				};

				assert_equal(reference, ranges(seq));
			}


			test( IteratorProvidesNecessaryOperators )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT
				seq.annotate(171819);
				seq += "A";
				seq.annotate(123);
				seq += "BCD";
				seq.annotate(42);
				seq += "q";

				// ACT / ASSERT
				container_t::const_iterator i = seq.ranges_begin(), j = i;

				++++j;

				assert_not_null(i->annotation);
				assert_equal(171819, *i->annotation);
				assert_equal(1, distance(i->begin, i->end));
				assert_is_false(j == i);
				assert_is_false(seq.ranges_end() == i);
				assert_is_true(j != i);
				assert_is_true(seq.ranges_end() != i);
				++i;
				assert_not_null(i->annotation);
				assert_equal(123, *i->annotation);
				assert_equal(3, distance(i->begin, i->end));
				assert_is_false(j == i);
				assert_is_false(seq.ranges_end() == i);
				assert_is_true(j != i);
				assert_is_true(seq.ranges_end() != i);
				++i;
				assert_not_null(i->annotation);
				assert_equal(42, *i->annotation);
				assert_equal(1, distance(i->begin, i->end));
				assert_is_true(j == i);
				assert_is_false(seq.ranges_end() == i);
				assert_is_false(j != i);
				assert_is_true(seq.ranges_end() != i);
				++i;
				assert_is_false(j == i);
				assert_is_true(seq.ranges_end() == i);
				assert_is_true(j != i);
				assert_is_false(seq.ranges_end() != i);
			}


			test( SizeIsStringSize )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT / ASSERT
				assert_is_true(static_cast<const container_t &>(seq).empty());
				assert_equal(0u, static_cast<const container_t &>(seq).size());

				// ACT
				seq += "Astra";

				// ACT / ASSERT
				assert_is_false(seq.empty());
				assert_equal(5u, static_cast<const container_t &>(seq).size());

				// ACT
				seq.annotate(123);
				seq += "BC";
				seq.annotate(1);

				// ACT / ASSERT
				assert_equal(7u, static_cast<const container_t &>(seq).size());
			}

		end_test_suite
	}
}
