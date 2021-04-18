#include <agge.text/layout.h>

#include "mocks.h"
#include "helpers.h"
#include "helpers_layout.h"

#include <agge.text/limit.h>
#include <agge.text/font_factory.h>
#include <map>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			class font_factory : public agge::font_factory
			{
			public:
				struct font_descriptor_less
				{
					bool operator ()(const font_descriptor &lhs, const font_descriptor &rhs) const
					{	return lhs.family < rhs.family ? true : rhs.family < lhs.family ? false : lhs.height < rhs.height;	}
				};

				typedef map<font_descriptor, shared_ptr<font>, font_descriptor_less> fonts_map;

			public:
				virtual shared_ptr<font> create_font(const font_descriptor &descriptor)
				{
					fonts_map::const_iterator i = fonts.find(descriptor);

					assert_not_equal(fonts.end(), i);
					return i->second;
				}

			public:
				fonts_map fonts;
			};
		}

		begin_test_suite( RichTextLayoutTests )

			mocks::font_factory factory;
			shared_ptr<font> arial, helvetica, segoe;

			init( InitFonts )
			{
				font_metrics m1 = { 10.0f, 2.0f, 2.0f };
				font_metrics m2 = { 14.7f, 3.0f, 1.0f };
				font_metrics m3 = { 9.0f, 1.0f, 1.0f };
				mocks::font_accessor::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, { L'D', 4 }, };
				mocks::font_accessor::char_to_index indices2[] = { { L' ', 0 }, { L'A', 5 }, { L'B', 2 }, { L'C', 3 }, { L'D', 4 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 3, 0 } },
					{ { 5, 0 } },
					{ { 7, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 21.15, 0 } },
				};

				arial = factory.fonts[font_descriptor::create("Arial", 13)] = mocks::create_font(m1, indices, glyphs);
				helvetica = factory.fonts[font_descriptor::create("Helvetica", 17)] = mocks::create_font(m2, indices2, glyphs);
				segoe = factory.fonts[font_descriptor::create("Segoe UI", 10)] = mocks::create_font(m3, indices, glyphs);
			}


			test( FontSpecifiedByAnnotationIsAssignedToTheRange )
			{
				// INIT
				layout l;
				richtext_t text((font_style_annotation()));

				text << style::family("Arial") << style::height(13) << "ADB\n"
					<< style::family("Helvetica") << style::height(17) << "AAAAB\n"
					<< style::family("Segoe UI") << style::height(10) << "ACAA\n";

				// ACT
				l.process(text, limit::none(), factory);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(arial, 0.0f, 0.0f, plural + 1 + 4 + 2))
					+ ref_text_line(0.0f, 28.7f, 0.0f, plural + ref_glyph_run(helvetica, 0.0f, 0.0f, plural + 5 + 5 + 5 + 5 + 2))
					+ ref_text_line(0.0f, 41.7f, 0.0f, plural + ref_glyph_run(segoe, 0.0f, 0.0f, plural + 1 + 3 + 1 + 1)),
					mkvector(l.begin(), l.end()));
			}


			test( MultipleFontsShareTheSameSingleTextLine )
			{
				// INIT
				layout l;
				richtext_t text((font_style_annotation()));

				text << style::family("Arial") << style::height(13) << "ADB " // width: 28
					<< style::family("Helvetica") << style::height(17) << "AA" // width: 42.3
					<< style::family("Segoe UI") << style::height(10) << "AB"; // width: 12

				// ACT
				l.process(text, limit::none(), factory);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 14.7f, 0.0f, plural
						+ ref_glyph_run(arial, 0.0f, 0.0f, plural + 1 + 4 + 2 + 0)
						+ ref_glyph_run(helvetica, 28.0f, 0.0f, plural + 5 + 5)
						+ ref_glyph_run(segoe, 70.3f, 0.0f, plural + 1 + 2)),
					mkvector(l.begin(), l.end()));
			}


			test( RegularWordBreakMovesNextGlyphRunToStartOfString )
			{
				// INIT
				layout l;
				richtext_t text((font_style_annotation()));

				text << style::family("Arial") << style::height(13) << "ADB " // width: 28
					<< style::family("Helvetica") << style::height(17) << "AA BB" // width: 42.3 + 3 + 14
					<< style::family("Segoe UI") << style::height(10) << "AB"; // width: 12

				// ACT
				l.process(text, limit::wrap(70.4f), factory);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 14.7f, 70.3f, plural
						+ ref_glyph_run(arial, 0.0f, 0.0f, plural + 1 + 4 + 2 + 0)
						+ ref_glyph_run(helvetica, 28.0f, 0.0f, plural + 5 + 5))
					+ ref_text_line(0.0f, 33.4f, 26.0f, plural
						+ ref_glyph_run(helvetica, 0.0f, 0.0f, plural + 2 + 2)
						+ ref_glyph_run(segoe, 14.0f, 0.0f, plural + 1 + 2)),
					mkvector(l.begin(), l.end()));
			}


			test( FirstOffsetIsCalculatedAccordinglyToMaxAscentInFirstLine )
			{
				// INIT
				layout l;
				richtext_t text((font_style_annotation()));

				text << style::family("Segoe UI") << style::height(10) << "A"
					<< style::family("Arial") << style::height(13) << "A"
					<< style::family("Helvetica") << style::height(17) << "A";

				// ACT
				l.process(text, limit::none(), factory);

				// ASSERT
				assert_equal(plural + ref_text_line_offsets(0.0f, 14.7f), mkvector(l.begin(), l.end()));

				// INIT
				text.clear();
				text << style::family("Segoe UI") << style::height(10) << "A"
					<< style::family("Arial") << style::height(13) << "A";

				// ACT
				l.process(text, limit::none(), factory);

				// ASSERT
				assert_equal(plural + ref_text_line_offsets(0.0f, 10.0f), mkvector(l.begin(), l.end()));
			}


			test( WordBrokenTextUsesMaxDescentLeadingOfTheFirstLineAndAscentOfTheSecondToOffsetSecondLine )
			{
				// INIT
				layout l;
				richtext_t text((font_style_annotation()));

				text << style::family("Segoe UI") << style::height(10) << "AAA" // 15
					<< style::family("Helvetica") << style::height(17) << "AAA" // 63.45
					<< style::family("Arial") << style::height(13) << "AAA"; // 15

				// ACT
				l.process(text, limit::wrap(84.0f), factory);

				// ASSERT
				assert_equal(plural
					+ ref_text_line_offsets(0.0f, 14.7f)
					+ ref_text_line_offsets(0.0f, 28.7f),
					mkvector(l.begin(), l.end()));
			}


			test( BreakingAtTheBeginingOfNewTextRangeDoesNotStopProcessing )
			{
				// INIT
				layout l;
				richtext_t text((font_style_annotation()));

				text << style::family("Segoe UI") << style::height(10) << "AAA" // 15
					<< style::family("Arial") << style::height(13) << "A"; // 5

				// ACT
				l.process(text, limit::wrap(17.0f), factory);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 9.0f, 0.0f, plural + ref_glyph_run(segoe, 0.0f, 0.0f, plural + 1 + 1 + 1))
					+ ref_text_line(0.0f, 21.0f, 0.0f, plural + ref_glyph_run(arial, 0.0f, 0.0f, plural + 1)),
					mkvector(l.begin(), l.end()));
			}


			test( BoxIsCalculatedForMultiLineMultiFontRichText )
			{
				// INIT
				layout l;
				richtext_t text((font_style_annotation()));

				text << style::family("Arial") << style::height(13) << "AD\nB"
					<< style::family("Helvetica") << style::height(17) << "AAAAB\n"
					<< style::family("Segoe UI") << style::height(10) << "ACAA";

				// ACT
				l.process(text, limit::none(), factory);

				// ACT / ASSERT
				box_r reference1 = {	98.6f, 14.0f + 18.7f + 10.0f	};

				assert_equal(reference1, l.get_box());

				// INIT
				text << style::family("Arial") << style::height(13) << "A";

				// ACT
				l.process(text, limit::none(), factory);

				// ACT / ASSERT
				box_r reference2 = {	98.6f, 14.0f + 18.7f + 12.0f	};

				assert_equal(reference2, l.get_box());

				// INIT
				text.clear();
				text << style::family("Arial") << style::height(13) << "A";

				// ACT
				l.process(text, limit::none(), factory);

				// ACT / ASSERT
				box_r reference3 = {	5.0f, 12.0f	};

				assert_equal(reference3, l.get_box());
			}


			struct StyleDetector
			{
				StyleDetector()
					: history(new vector<real_t>)
				{	}

				template <typename T>
				void begin_style(T &/*builder*/)
				{
					// ASSERT (mutating the builder is prohibited at this step)
					assert_is_false(true);
				}

				template <typename T>
				void begin_style(const T &builder)
				{
					history->push_back(builder.current_glyph('A').second);
					history->push_back(builder.current_glyph('B').second);
				}

				void new_line()
				{	history->push_back(1000.0f);	}
				
				template <typename CharIteratorT>
				bool add_glyph(layout_builder &/*builder*/, glyph_index_t /*glyph_index*/, real_t extent_,
					CharIteratorT &i, CharIteratorT next, CharIteratorT /*end*/)
				{
					history->push_back(extent_);
					i = next;
					return true;
				}

				shared_ptr< vector<real_t> > history;
			};

			test( LimitProcessorIsNotifiedOfNewLines )
			{
				// INIT
				StyleDetector d;
				layout l;
				richtext_t text((font_style_annotation()));

				text << style::family("Arial") << style::height(13) << "AD\nB"
					<< style::family("Helvetica") << style::height(17) << "AAAAB\n"
					<< style::family("Segoe UI") << style::height(10) << "ACAA";

				// ACT
				l.process(text, d, factory);

				// ASSERT
				real_t reference1[] = {
					5.0f, 7.0f, 5.0f, 13.0f, 1000.0f,
					7.0f, 21.15f, 7.0f, 21.15f, 21.15f, 21.15f, 21.15f, 7.0f, 1000.0f,
					5.0f, 7.0f, 5.0f, 11.0f, 5.0f, 5.0f,
				};

				assert_equal(reference1, *d.history);

				// INIT
				text.clear();
				text << style::family("Helvetica") << style::height(17) << "ACA";
				d.history->clear();

				// ACT
				l.process(text, d, factory);

				// ASSERT
				real_t reference2[] = {
					21.15f, 7.0f, 21.15f, 11.0f, 21.15f,
				};

				assert_equal(reference2, *d.history);
			}

		end_test_suite
	}
}
