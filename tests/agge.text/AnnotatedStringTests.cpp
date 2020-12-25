#include <agge.text/annotated_string.h>

#include <tests/common/helpers.h>

#include <algorithm>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace ut
{
	template <typename T, typename AnnotationT, size_t n>
	inline void are_equal(pair<size_t, AnnotationT> (&expected)[n], const agge::annotated_string<T, AnnotationT> &actual,
		const LocationInfo &location)
	{
		size_t index = 0u;
		const pair<size_t, AnnotationT> *m = expected;

		for (typename agge::annotated_string<T, AnnotationT>::const_iterator i = actual.begin(); i != actual.end();
			++i, ++index)
		{
			if (i->annotation)
			{
				are_not_equal(end(expected), m, location);
				are_equal(m->first, index, location);
				are_equal(m->second, *i->annotation, location);
				++m;
			}
		}
		are_equal(end(expected), m, location);
	}
}

namespace agge
{
	template <typename T, typename AnnotationT>
	inline bool operator ==(const basic_string<T> &expected, const agge::annotated_string<T, AnnotationT> &actual)
	{
		typename agge::annotated_string<T, AnnotationT>::const_iterator i = actual.begin();
		typename basic_string<T>::const_iterator j = expected.begin();

		for (; i != actual.end() && j != expected.end(); ++i, ++j)
		{
			if (*j != *i)
				return false;
		}
		return actual.end() == i && expected.end() == j;
	}

	namespace tests
	{
		namespace
		{
			template <typename T, typename AnnotationT>
			bool is_trivial(const annotated_string<T, AnnotationT> &seq)
			{
				for (typename annotated_string<T, AnnotationT>::const_iterator i = seq.begin(); i != seq.end(); ++i)
				{
					if (i->annotation)
						return false;
				}
				return true;
			}
		}

		begin_test_suite( AnnotatedStringTests )
			test( AppendingSimpleSequenceDoesNotModifyIt )
			{
				typedef annotated_string<wchar_t, string> container_t;

				// INIT
				container_t seq;

				// ACT
				seq += L"Lorem ipsum dolor sit amet";

				// ACT / ASSERT
				assert_equal(wstring(L"Lorem ipsum dolor sit amet"), seq);
				assert_is_true(is_trivial(seq));

				// ACT
				seq += L", consectetur adipisci elit";

				// ACT / ASSERT
				assert_is_true(is_trivial(seq));
				assert_equal(wstring(L"Lorem ipsum dolor sit amet, consectetur adipisci elit"), seq);
			}


			test( ConvertingFromStringProducesTrivialAnnotatedString )
			{
				typedef annotated_string<char, char> container_t;

				// INIT / ACT
				const container_t seq1("Lorem ipsum");

				// ACT / ASSERT
				assert_equal(string("Lorem ipsum"), seq1);
				assert_is_true(is_trivial(seq1));

				// INIT / ACT
				const container_t seq2("dolor sit amet");

				// ACT / ASSERT
				assert_equal(string("dolor sit amet"), seq2);
				assert_is_true(is_trivial(seq2));
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
				assert_equal(string("aZ"), seq);
				assert_is_true(is_trivial(seq));
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
				assert_equal(string("foobar"), seq);

				pair<size_t, string> reference_modifiers[] = {
					make_pair(0u, "zamazu"),
				};

				assert_equal(reference_modifiers, seq);
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
				assert_equal(string("foo BarBAZ"), seq);

				pair<size_t, int> reference_modifiers[] = {
					make_pair(4, 17), make_pair(7, 191),
				};

				assert_equal(reference_modifiers, seq);
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
				assert_equal(string("In a free socicety political opponents survive"), seq);

				pair<size_t, int> reference_modifiers[] = {
					make_pair(5, 32), make_pair(9, 0),
				};

				assert_equal(reference_modifiers, seq);
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
				seq += "BC";

				// ACT / ASSERT
				container_t::const_iterator i = seq.begin();

				assert_equal('A', *i);
				assert_not_null(i->annotation);
				assert_equal(171819, *i->annotation);
				++i;
				assert_equal('B', *i);
				assert_not_null(i->annotation);
				assert_equal(123, *i->annotation);
				++i;
				assert_equal('C', *i);
				assert_null(i->annotation);
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
