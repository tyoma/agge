#include <agge.text/annotated_string.h>

#include "helpers.h"

#include <algorithm>
#include <tests/common/helpers.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	template <typename T, typename AnnotationT>
	inline bool operator ==(const pair<basic_string<T>, AnnotationT> &expected,
		const typename annotated_string<T, AnnotationT>::range &actual)
	{
		return expected.first == basic_string<T>(actual.begin(), actual.end())
			&& expected.second == actual.get_annotation();
	}

	namespace tests
	{
		namespace
		{
			class non_default_constructible
			{
			public:
				non_default_constructible(int)
				{	}
			};

			template <typename T, typename AnnotationT>
			vector<typename annotated_string<T, AnnotationT>::range> ranges_vector(
				const annotated_string<T, AnnotationT> &from)
			{	return mkvector(from.ranges_begin(), from.ranges_end());	}
		}

		begin_test_suite( AnnotatedStringTests )
			test( EmptyStringContainsNoRanges )
			{
				typedef annotated_string<wchar_t, string> container1_t;
				typedef annotated_string<int, non_default_constructible> container2_t;

				// INIT
				container1_t seq1;

				// ACT / ASSERT
				assert_equal(seq1.ranges_end(), seq1.ranges_begin());

				// INIT / ACT
				container2_t seq2(non_default_constructible(123));

				// ASSERT
				assert_equal(seq2.ranges_end(), seq2.ranges_begin());
			}


			test( AppendingSimpleSequenceDoesNotAnnotateIt )
			{
				typedef annotated_string<wchar_t, string> container_t;

				// INIT
				container_t seq;

				// ACT
				seq.append(L"Lorem ipsum dolor sit amet");

				// ACT / ASSERT
				pair<wstring, string> reference1[] = {
					make_pair(L"Lorem ipsum dolor sit amet", string()),
				};

				assert_equal(reference1, ranges_vector(seq));

				// ACT
				seq.append(L", consectetur adipisci elit");

				// ACT / ASSERT
				pair<wstring, string> reference2[] = {
					make_pair(L"Lorem ipsum dolor sit amet, consectetur adipisci elit", string()),
				};

				assert_equal(reference2, ranges_vector(seq));

				// INIT
				wstring appendix = L"lorem-lorem";

				// ACT
				seq.append(appendix.begin(), appendix.begin() + 3);

				// ASSERT
				pair<wstring, string> reference3[] = {
					make_pair(L"Lorem ipsum dolor sit amet, consectetur adipisci elitlor", string()),
				};

				assert_equal(reference3, ranges_vector(seq));

				// ACT
				seq.append(appendix.begin() + 3, appendix.end());

				// ASSERT
				pair<wstring, string> reference4[] = {
					make_pair(L"Lorem ipsum dolor sit amet, consectetur adipisci elitlorem-lorem", string()),
				};

				assert_equal(reference4, ranges_vector(seq));
			}


			test( ConvertingFromStringProducesTrivialAnnotatedString )
			{
				typedef annotated_string<char, char> container_t;

				// INIT / ACT
				const container_t seq1("Lorem ipsum");

				// ACT / ASSERT
				pair<string, char> reference1[] = {
					make_pair("Lorem ipsum", '\0'),
				};

				assert_equal(reference1, ranges_vector(seq1));

				// INIT / ACT
				const container_t seq2("dolor sit amet");

				// ACT / ASSERT
				pair<string, char> reference2[] = {
					make_pair("dolor sit amet", '\0'),
				};

				assert_equal(reference2, ranges_vector(seq2));
			}


			test( AnnotatingInTheEndDoesNotAffectExistedSequence )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT
				seq.append("aZ");
				seq.annotate(123456);

				// ACT / ASSERT
				pair<string, int> reference[] = {
					make_pair("aZ", 0),
				};

				assert_equal(reference, ranges_vector(seq));
			}


			test( AnnotatingAtTheStartModifiesFollowingSequence )
			{
				typedef annotated_string<char, string> container_t;

				// INIT
				container_t seq;

				// ACT
				seq.annotate("zamazu");
				seq.append("foobar");

				// ACT / ASSERT
				pair<string, string> reference[] = {
					make_pair("foobar", "zamazu"),
				};

				assert_equal(reference, ranges_vector(seq));
			}


			test( SettingModifierInTheMiddleModifiesFollowingSequence )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT
				seq.append("foo ");
				seq.annotate(17);
				seq.append("Bar");
				seq.annotate(191);
				seq.append("BAZ");

				// ACT / ASSERT
				pair<string, int> reference[] = {
					make_pair("foo ", 0),
					make_pair("Bar", 17),
					make_pair("BAZ", 191),
				};

				assert_equal(reference, ranges_vector(seq));
			}


			test( LastSetModifierTakesPrecedence )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT
				seq.append("In a ");
				seq.annotate(100);
				seq.annotate(90);
				seq.annotate(32);
				seq.append("free");
				seq.annotate(0);
				seq.append(" socicety political opponents survive");

				// ACT / ASSERT
				pair<string, int> reference[] = {
					make_pair("In a ", 0),
					make_pair("free", 32),
					make_pair(" socicety political opponents survive", 0),
				};

				assert_equal(reference, ranges_vector(seq));
			}


			test( IteratorProvidesNecessaryOperators )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				// ACT
				seq.annotate(171819);
				seq.append("A");
				seq.annotate(123);
				seq.append("BCD");
				seq.annotate(42);
				seq.append("q");

				// ACT / ASSERT
				container_t::const_iterator i = seq.ranges_begin(), j = i;

				++++j;

				assert_equal(171819, i->get_annotation());
				assert_equal(1, distance(i->begin(), i->end()));
				assert_is_false(j == i);
				assert_is_false(seq.ranges_end() == i);
				assert_is_true(j != i);
				assert_is_true(seq.ranges_end() != i);
				++i;
				assert_equal(123, i->get_annotation());
				assert_equal(3, distance(i->begin(), i->end()));
				assert_is_false(j == i);
				assert_is_false(seq.ranges_end() == i);
				assert_is_true(j != i);
				assert_is_true(seq.ranges_end() != i);
				++i;
				assert_equal(42, i->get_annotation());
				assert_equal(1, distance(i->begin(), i->end()));
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
				seq.append("Astra");

				// ACT / ASSERT
				assert_is_false(seq.empty());
				assert_equal(5u, static_cast<const container_t &>(seq).size());

				// ACT
				seq.annotate(123);
				seq.append("BC");
				seq.annotate(1);

				// ACT / ASSERT
				assert_equal(7u, static_cast<const container_t &>(seq).size());
			}


			test( ClearingProducesStringWithNoRanges )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				seq.annotate(17);
				seq.append("some text");
				seq.annotate(19);
				seq.append("!");

				// ACT
				seq.clear();

				// ACT / ASSERT
				assert_is_empty(seq);
				assert_equal(seq.ranges_end(), seq.ranges_begin());
			}


			test( PreviousRangesAreResetOnClearing )
			{
				typedef annotated_string<char, int> container_t;

				// INIT
				container_t seq;

				seq.append("some text");
				seq.annotate(19);
				seq.append("!");

				// ACT
				seq.clear();
				seq.annotate(1317);
				seq.append("new text");

				// ASSERT
				assert_equal((plural + pair<string, int>("new text", 1317)), ranges_vector(seq));
			}


			test( BaseAnnotationsAreReturnedIfNoAnnotationsWereSpecified )
			{
				typedef annotated_string<char, int> container1_t;
				typedef annotated_string<char, wstring> container2_t;

				// INIT / ACT
				container1_t seq1(17);
				container1_t seq2("text #2", 193);
				container2_t seq3(L"foo");
				container2_t seq4("text #4", L"bar");

				seq1.append("text #1");
				seq3.append("text #3");

				// ACT / ASSERT
				assert_equal((plural + pair<string, int>("text #1", 17)), ranges_vector(seq1));
				assert_equal((plural + pair<string, int>("text #2", 193)), ranges_vector(seq2));
				assert_equal((plural + pair<string, wstring>("text #3", L"foo")), ranges_vector(seq3));
				assert_equal((plural + pair<string, wstring>("text #4", L"bar")), ranges_vector(seq4));
			}


			test( StartAnnotationIsResetToBaseOnClear )
			{
				typedef annotated_string<char, int> container_t;

				// INIT / ACT
				container_t seq1(17);
				container_t seq2("", 193);

				seq1.annotate(100);
				seq1.append("text #1");
				seq2.annotate(100);
				seq2.append("text #3");

				// ACT
				seq1.clear();
				seq2.clear();

				seq1.append("new");
				seq2.append("new");

				// ACT / ASSERT
				assert_equal((plural + pair<string, int>("new", 17)), ranges_vector(seq1));
				assert_equal((plural + pair<string, int>("new", 193)), ranges_vector(seq2));
			}


			test( SettingBaseAnnotationClearsStringAndAppliesIt )
			{
				typedef annotated_string<char, int> container_t;

				// INIT / ACT
				container_t seq;

				seq.annotate(123);
				seq.append("one");
				seq.annotate(12);
				seq.append("two");

				// ACT
				seq.set_base_annotation(1984);

				// ASSERT
				assert_is_empty(seq);
				assert_equal(seq.ranges_end(), seq.ranges_begin());

				// ACT
				seq.append("test");

				// ASSERT
				assert_equal((plural + pair<string, int>("test", 1984)), ranges_vector(seq));
			}


			test( LastAnnotationIsReturnAsCurrent )
			{
				typedef annotated_string<wchar_t, string> container1_t;
				typedef annotated_string<wchar_t, int> container2_t;

				// INIT / ACT
				container1_t seq1("foe");
				container2_t seq2(1);

				// ACT / ASSERT
				assert_equal("foe", seq1.current_annotation());
				assert_equal(1, seq2.current_annotation());

				// ACT
				seq1.annotate("jazz");
				seq2.append(L"zzz");
				seq2.annotate(1312);

				// ACT / ASSERT
				assert_equal("jazz", seq1.current_annotation());
				assert_equal(1312, seq2.current_annotation());
			}

		end_test_suite
	}
}
