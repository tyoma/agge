#include <agge.text/limit.h>

#include "mocks.h"

#include <tests/common/helpers.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	inline bool operator ==(layout_builder::state lhs, layout_builder::state rhs)
	{	return tests::equal(lhs.extent, rhs.extent) && lhs.next == rhs.next;	}

	namespace tests
	{
		namespace mocks
		{
			struct layout_builder
			{
				layout_builder()
					: ellipsis_codepoint(0x2026), ellipsis_symbol(make_pair(139, 3.71f)), state(zero())
				{	}

				pair<glyph_index_t, real_t> current_glyph(codepoint_t codepoint) const
				{
					assert_equal(ellipsis_codepoint, codepoint);
					return ellipsis_symbol;
				}

				pair<glyph_index_t, real_t> current_glyph(codepoint_t /*codepoint*/)
				{
					assert_is_false(true); // prohibit calling this method from add_glyph()
					return make_pair(0u, real_t());
				}

				const agge::layout_builder::state &get_state() const
				{	return state;	}

				void trim_current_line(agge::layout_builder::state at)
				{
					trimmed_at.push_back(at);
					state = at;
				}

				void append_glyph(glyph_index_t g, real_t e)
				{
					appended.push_back(make_pair(state.next, make_pair(g, e)));
					state.next++, state.extent += e;
				}

				codepoint_t ellipsis_codepoint;
				pair<glyph_index_t, real_t> ellipsis_symbol;
				vector<agge::layout_builder::state> trimmed_at;
				vector< pair<size_t /*at*/, pair<glyph_index_t, real_t> > > appended;
				agge::layout_builder::state state;
			};
		}

		begin_test_suite( LimitEllipsisTests )
			test( SymbolsUnderLimitAreSimplyAppended )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;

				b.state.next = 119;

				// INIT / ACT
				limit::ellipsis e(100.0f);

				e.begin_style(b);

				// ACT / ASSERT
				assert_is_true(e.add_glyph(b, 12, 1.9f, i, begin + 1, end));
				assert_equal(begin + 1, i);

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference1[] = {
					make_pair(119u, make_pair(12, 1.9f)),
				};

				assert_equal(reference1, b.appended);

				// ACT / ASSERT
				assert_is_true(e.add_glyph(b, 1200, 31.2f, i, begin + 3, end));
				assert_equal(begin + 3, i);
				assert_is_true(e.add_glyph(b, 11, 11.7f, i, begin + 4, end));
				assert_equal(begin + 4, i);

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference2[] = {
					make_pair(119u, make_pair(12, 1.9f)),
					make_pair(120u, make_pair(1200, 31.2f)),
					make_pair(121u, make_pair(11, 11.7f)),
				};

				assert_equal(reference2, b.appended);
			}


			test( EllipsisSymbolIsAddedOnReachingLimit )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;
				limit::ellipsis e(100.0f);

				e.begin_style(b);
				b.state.extent = 96.1f;
				b.state.next = 10009;

				// ACT / ASSERT
				assert_is_true(e.add_glyph(b, 123, 4.0f, i, begin + 1, end));

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference1[] = {	make_pair(10009u, make_pair(139u, 3.71f)),	};
				layout_builder::state referenceTrimmed1[] = {	{	96.1f, 10009u	}	};

				assert_equal(reference1, b.appended);
				assert_equal(referenceTrimmed1, b.trimmed_at);
				assert_equal(begin + 1, i);

				// INIT
				b = mocks::layout_builder();
				e = limit::ellipsis(73.4f);

				b.ellipsis_symbol.first = 1001;
				e.begin_style(b);
				b.state.extent = 73.4f - 3.71001f;
				b.state.next = 9009;

				// ACT
				e.add_glyph(b, 123, 3.72f, i, begin + 2, end);

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference2[] = {	make_pair(9009u, make_pair(1001u, 3.71f)),	};
				layout_builder::state referenceTrimmed2[] = {	{	73.4f - 3.71001f, 9009u	}	};

				assert_equal(reference2, b.appended);
				assert_equal(referenceTrimmed2, b.trimmed_at);
				assert_equal(begin + 2, i);

				// INIT
				b = mocks::layout_builder();
				e = limit::ellipsis(73.4f, 0x12345);

				b.ellipsis_codepoint = 0x12345;
				b.ellipsis_symbol.second = 5.5f;
				e.begin_style(b);
				b.state.extent = 73.4f - 5.5001f;
				b.state.next = 100;

				// ACT
				e.add_glyph(b, 123, 5.502f, i, begin + 1, end);

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference3[] = {	make_pair(100u, make_pair(139u, 5.5f)),	};
				layout_builder::state referenceTrimmed3[] = {	{	73.4f - 5.5001f, 100u	}	};

				assert_equal(reference3, b.appended);
				assert_equal(referenceTrimmed3, b.trimmed_at);
			}


			test( SymbolsCanBeAppendedAfterEllipsisPositionIsLatched )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;
				limit::ellipsis e(100.0f);

				e.begin_style(b);
				b.state.extent = 100.0f - 3.71f;
				b.state.next = 113;

				// ACT / ASSERT
				assert_is_true(e.add_glyph(b, 23, 1.1f, i, begin + 1, end));
				assert_is_true(e.add_glyph(b, 20, 1.3f, i, begin + 3, end));

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference[] = {
					make_pair(113u, make_pair(23u, 1.1f)),
					make_pair(114u, make_pair(20u, 1.3f)),
				};

				assert_equal(reference, b.appended);
				assert_is_empty(b.trimmed_at);
				assert_equal(begin + 3, i);
			}


			test( EllipsisIsAddedToTheStoredPositionOnOverreachingLimit )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;
				limit::ellipsis e(100.0f);

				e.begin_style(b);
				b.state.extent = 100.0f - 4.71f;
				b.state.next = 113;
				e.add_glyph(b, 23, 0.91f, i, begin + 1, end);
				// <- in here the ellipsis will be appended after the trimming
				e.add_glyph(b, 23, 1.1f, i, begin + 2, end);
				e.add_glyph(b, 20, 1.3f, i, begin + 3, end);
				b.appended.clear();

				// ACT / ASSERT
				assert_is_true(e.add_glyph(b, 23, 4.71f - 0.91f - 1.1f - 1.3f + 0.01f, i, begin + 5, end));

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference[] = {
					make_pair(114u, make_pair(139u, 3.71f)),
				};
				layout_builder::state referenceTrimmed[] = {	{	100.0f - 4.71f + 0.91f, 114u	}	};

				assert_equal(reference, b.appended);
				assert_equal(referenceTrimmed, b.trimmed_at);
				assert_equal(begin + 5, i);
			}


			test( GlyphsAreIgnoredAfterAddingEllipsis )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;
				limit::ellipsis e(50.0f);

				e.begin_style(b);
				b.state.extent = 50.0f - 4.7f;
				b.state.next = 1001;
				e.add_glyph(b, 23, 0.5f, i, begin + 1, end);
				// <- in here the ellipsis will be appended after the trimming
				e.add_glyph(b, 23, 5.1f, i, begin + 2, end);
				b.appended.clear();

				// ACT / ASSERT
				assert_is_true(e.add_glyph(b, 23, 0.0001f, i, begin + 3, end));

				// ASSERT
				layout_builder::state referenceTrimmed[] = {	{	50.0f - 4.7f + 0.5f, 1002u	}	};

				assert_is_empty(b.appended);
				assert_equal(referenceTrimmed, b.trimmed_at);
				assert_equal(begin + 3, i);

				// ACT / ASSERT
				e.add_glyph(b, 23, 0.0001f, i, begin + 4, end);
				e.add_glyph(b, 23, 0.0001f, i, begin + 5, end);

				// ASSERT
				assert_is_empty(b.appended);
				assert_equal(referenceTrimmed, b.trimmed_at);
				assert_equal(begin + 5, i);
			}


			test( IgnoreIsLiftedOnNewLine )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;
				limit::ellipsis e(50.0f);

				e.begin_style(b);
				b.state.extent = 50.0f - 4.7f;
				b.state.next = 1001;
				e.add_glyph(b, 23, 0.5f, i, begin + 1, end);
				// <- in here the ellipsis will be appended after the trimming
				e.add_glyph(b, 23, 5.1f, i, begin + 2, end);
				b.appended.clear();
				b.trimmed_at.clear();

				// INIT
				b.state.extent = 50.0f - 10.0f;

				// ACT
				e.new_line();
				e.add_glyph(b, 171, 1.1f, i, begin + 1, end);
				e.add_glyph(b, 172, 0.9f, i, begin + 1, end);
				// <- in here the ellipsis will be appended after the trimming
				e.add_glyph(b, 173, 7.5f, i, begin + 1, end);
				e.add_glyph(b, 177, 2.6f, i, begin + 2, end);

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference[] = {
					make_pair(1003u, make_pair(171u, 1.1f)),
					make_pair(1004u, make_pair(172u, 0.9f)),
					make_pair(1005u, make_pair(173u, 7.5f)),
					make_pair(1005u, make_pair(139u, 3.71f)),
				};
				layout_builder::state referenceTrimmed[] = {	{	42.0f, 1005u	}	};

				assert_equal(reference, b.appended);
				assert_equal(referenceTrimmed, b.trimmed_at);
				assert_equal(begin + 2, i);
			}


			test( GlyphIndexAndExtentAreLatchedForTheStyleAtWhichCandidatePositionIsFound )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;
				limit::ellipsis e(50.0f);

				e.begin_style(b);
				b.state.extent = 50.0f - 4.71f;
				b.state.next = 10;
				e.add_glyph(b, 23, 1.0f, i, begin + 1, end);
				// <- in here the ellipsis will be appended after the trimming
				e.add_glyph(b, 24, 1.0f, i, begin + 2, end);
				b.ellipsis_symbol.first = 101;
				b.ellipsis_symbol.second = 2.9f;
				e.begin_style(b);

				// ACT
				e.add_glyph(b, 25, 2.8f, i, begin + 3, end);

				// ASSERT
				pair<size_t, pair<glyph_index_t, real_t> > reference[] = {
					make_pair(10u, make_pair(23u, 1.0f)),
					make_pair(11u, make_pair(24u, 1.0f)),
					make_pair(11u, make_pair(139u, 3.71f)), // despite the last ellipsis was 101, 2.9f, we set the previous.
				};
				layout_builder::state referenceTrimmed[] = {	{	46.29f, 11u	}	};

				assert_equal(reference, b.appended);
				assert_equal(referenceTrimmed, b.trimmed_at);
				assert_equal(begin + 3, i);
			}


			test( EllipsisLimiterHaltsProcessingIfLimitationOccursBeforeEllipsisPositionIsLatched )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;
				limit::ellipsis e(50.0f);

				e.begin_style(b);
				b.state.extent = 50.0f - 2.0f;
				b.state.next = 10;

				// ACT
				assert_is_false(e.add_glyph(b, 25, 2.8f, i, begin + 3, end));

				// ASSERT
				assert_is_empty(b.appended);
				assert_is_empty(b.trimmed_at);
				assert_equal(begin, i);
			}


			test( TrimPositionIsResetOnNewLine )
			{
				// INIT
				mocks::layout_builder b;
				const char *const begin = "foobar", *const end = begin + 6;
				const char *i = begin;
				limit::ellipsis e(50.0f);

				e.begin_style(b);
				b.state.extent = 50.0f - 4.0f;
				b.state.next = 10;

				e.add_glyph(b, 25, 5.0f, i, begin + 1, end);
				b.appended.clear();
				b.trimmed_at.clear();

				// ACT
				e.new_line();
				b.state.extent = 50.0f - 3.7f; // not enough space for the ellipsis
				assert_is_false(e.add_glyph(b, 25, 4.1f, i, begin + 3, end));

				// ASSERT
				assert_is_empty(b.appended);
				assert_is_empty(b.trimmed_at);
				assert_equal(begin + 1, i);
			}

		end_test_suite
	}
}
