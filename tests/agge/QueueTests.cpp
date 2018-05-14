#include <agge/queue.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			struct Foo
			{
				Foo(int a_ = 0) : a(a_) {	}

				virtual short get_b() {	return 0;	}

				int a;
			};

			struct Bar : Foo
			{
				Bar(int a_ = 0, short b_ = 0) : Foo(a_), b(b_) {	}

				virtual short get_b() { return b; }

				short b;
			};

			struct LifetimeAbstract
			{
				virtual ~LifetimeAbstract() { }
			};

			template <int Unique>
			class LifetimeDerived : public LifetimeAbstract
			{
			public:
				LifetimeDerived(int &n) : _n(n) {	++_n;	}
				LifetimeDerived(const LifetimeDerived &other) : _n(other._n) {	++_n;	}
				~LifetimeDerived() { --_n;	}

			private:
				void operator =(const LifetimeDerived &rhs);

			private:
				int &_n;
			};
		}

		begin_test_suite( PolyBufferTests )
			test( NewBufferIsEmpty )
			{
				// INIT / ACT
				poly_buffer<int> a(15);
				poly_buffer<Foo> b(15);

				// ACT / ASSERT
				assert_is_empty(a);
				assert_is_empty(b);
			}


			test( ElementPushedIsAccessible )
			{
				// INIT
				poly_buffer<double> a(30);
				poly_buffer<Foo> b(200);
				poly_buffer<Foo> c(200);

				// ACT
				a.push_back(1719.1);
				b.push_back(Foo(171922));
				c.push_back(Bar(13, 19));

				// ASSERT
				assert_equal(1719.1, a.front());
				assert_equal(171922, b.front().a);
				assert_equal(13, c.front().a);
				assert_equal(19, c.front().get_b());
			}


			test( PushToASaturatedSequenceReallocatesBuffer )
			{
				// INIT
				poly_buffer<double> seq1(poly_entry::size<double>() * 2);
				poly_buffer<double> seq2(poly_entry::size<double>() * 5);

				seq1.push_back(1.1);
				seq2.push_back(11.2);

				const double *p1 = &seq1.front();
				const double *p2 = &seq2.front();

				// ACT
				seq1.push_back(2.3);
				seq2.push_back(12.3);
				seq2.push_back(13.4);
				seq2.push_back(14.5);
				seq2.push_back(15.6);

				// ASSERT
				assert_equal(p1, &seq1.front());
				assert_equal(p2, &seq2.front());

				// ACT
				seq1.push_back(3.4);
				seq2.push_back(16.7);

				// ASSERT
				assert_not_equal(p1, &seq1.front());
				assert_not_equal(p2, &seq2.front());
			}


			test( SequencePushedCanBeReadBackScalar )
			{
				// INIT
				poly_buffer<int> seq(1000);

				// ACT
				seq.push_back(1);
				seq.push_back(1);
				seq.push_back(2);
				seq.push_back(3);
				seq.push_back(5);
				seq.push_back(8);
				seq.push_back(13);

				// ACT / ASSERT
				assert_equal(1, seq.front());
				seq.pop_front();
				assert_equal(1, seq.front());
				seq.pop_front();
				assert_equal(2, seq.front());
				seq.pop_front();
				assert_equal(3, seq.front());
				seq.pop_front();
				assert_equal(5, seq.front());
				seq.pop_front();
				assert_equal(8, seq.front());
				seq.pop_front();
				assert_equal(13, seq.front());
				seq.pop_front();
			}


			test( ObjectsAreCopyConstructedOnPush )
			{
				// INIT
				int n[3] = { 0 };
				poly_buffer<LifetimeAbstract> buffer(1000);

				// ACT
				buffer.push_back(LifetimeDerived<0>(n[0]));
				buffer.push_back(LifetimeDerived<1>(n[1]));
				buffer.push_back(LifetimeDerived<2>(n[2]));

				// ASSERT
				int reference1[] = { 1, 1, 1, };

				assert_equal(reference1, n);

				// ACT
				buffer.push_back(LifetimeDerived<0>(n[0]));
				buffer.push_back(LifetimeDerived<0>(n[0]));
				buffer.push_back(LifetimeDerived<2>(n[2]));

				// ASSERT
				int reference2[] = { 3, 1, 2, };

				assert_equal(reference2, n);
			}


			test( ObjectIsStoredEvenIfNoSpaceAvailable )
			{
				// INIT
				poly_buffer<int> buffer1(1);
				poly_buffer<double> buffer2(1);

				// ACT
				buffer1.push_back(43);
				buffer2.push_back(43.25);

				// ASSERT
				assert_equal(43, buffer1.front());
				assert_equal(43.25, buffer2.front());
			}


			test( VirtualDestructionIsCalledOnPop )
			{
				// INIT
				int n[3] = { 0 };
				poly_buffer<LifetimeAbstract> buffer(1000);

				buffer.push_back(LifetimeDerived<0>(n[0]));
				buffer.push_back(LifetimeDerived<1>(n[1]));
				buffer.push_back(LifetimeDerived<2>(n[2]));
				buffer.push_back(LifetimeDerived<2>(n[2]));
				buffer.push_back(LifetimeDerived<2>(n[2]));

				// ACT
				buffer.pop_front();

				// ASSERT
				int reference1[] = { 0, 1, 3, };

				assert_equal(reference1, n);

				// ACT
				buffer.pop_front();
				buffer.pop_front();

				// ASSERT
				int reference2[] = { 0, 0, 2, };

				assert_equal(reference2, n);
			}


			test( ElementsAreDestructedOnBufferDestruction )
			{
				// INIT
				int n[3] = { 0 };
				auto_ptr< poly_buffer<LifetimeAbstract> > buffer(new poly_buffer<LifetimeAbstract>(1000));

				buffer->push_back(LifetimeDerived<0>(n[0]));
				buffer->push_back(LifetimeDerived<1>(n[1]));
				buffer->push_back(LifetimeDerived<2>(n[2]));
				buffer->push_back(LifetimeDerived<2>(n[2]));
				buffer->push_back(LifetimeDerived<2>(n[2]));

				// ACT
				buffer.reset();

				// ASSERT
				int reference[] = { 0, 0, 0, };

				assert_equal(reference, n);
			}


			test( NewElementsArePlacedAtTheBeginingOfTheBufferAfterBufferIsEmpty )
			{
				// INIT
				poly_buffer<Foo> buffer1(1000);
				poly_buffer<int> buffer2(1000);
				Foo *p1 = 0;
				int *p2 = 0;

				buffer1.push_back(Bar()), p1 = &buffer1.front(), buffer1.push_back(Bar()), buffer1.push_back(Bar());
				buffer2.push_back(13), p2 = &buffer2.front(), buffer2.push_back(200211);

				// ACT
				buffer1.pop_front(), buffer1.pop_front(), buffer1.pop_front();
				buffer2.pop_front(), buffer2.pop_front();

				// ASSERT
				buffer1.push_back(Foo(17221));
				
				assert_equal(p1, &buffer1.front());
				assert_equal(17221, buffer1.front().a);

				buffer2.push_back(1112);

				assert_equal(p2, &buffer2.front());
				assert_equal(1112, buffer2.front());
			}


			test( ElementsAreCopiedPolymorphicallyOnSequenceGrow )
			{
				// INIT
				poly_buffer<Foo> buffer1(poly_entry::size<Foo>() * 2 + poly_entry::size<Bar>());
				poly_buffer<int> buffer2(poly_entry::size<int>() * 2);

				buffer1.push_back(Foo(17)), buffer1.push_back(Bar(1811, 23)), buffer1.push_back(Foo(191));
				buffer2.push_back(13), buffer2.push_back(200211);

				const Foo *p1 = &buffer1.front();
				const int *p2 = &buffer2.front();

				// ACT
				buffer1.push_back(Bar(11, 111));
				buffer2.push_back(200);

				// ASSERT
				assert_not_equal(p1, &buffer1.front());
				assert_is_false(buffer1.empty());
				assert_equal(17, buffer1.front().a);
				buffer1.pop_front();
				assert_equal(1811, buffer1.front().a);
				assert_equal(23, buffer1.front().get_b());
				buffer1.pop_front();
				assert_equal(191, buffer1.front().a);
				buffer1.pop_front();
				assert_equal(11, buffer1.front().a);
				assert_equal(111, buffer1.front().get_b());
				buffer1.pop_front();
				assert_is_empty(buffer1);

				assert_not_equal(p2, &buffer2.front());
				assert_is_false(buffer2.empty());
				assert_equal(13, buffer2.front());
				buffer2.pop_front();
				assert_equal(200211, buffer2.front());
				buffer2.pop_front();
				assert_equal(200, buffer2.front());
				buffer2.pop_front();
				assert_is_empty(buffer2);
			}


			test( OldElementsAreDestructedOnBufferReallocation )
			{
				// INIT
				int n[2] = { 0 };
				poly_buffer<LifetimeAbstract> buffer(poly_entry::size<LifetimeAbstract>() * 3);

				buffer.push_back(LifetimeDerived<0>(n[0]));
				buffer.push_back(LifetimeDerived<1>(n[1]));
				buffer.push_back(LifetimeDerived<1>(n[1]));

				// ACT
				buffer.push_back(LifetimeDerived<1>(n[1]));

				// ASSERT
				int reference[] = { 1, 3, };

				assert_equal(reference, n);
			}

		end_test_suite


		begin_test_suite( QueueTests )
			static void dummy_consume_foo(Foo &) {	}

			static void dummy_consume_int(int &) {	}

			struct pre_consume
			{
				void operator ()(int records) const
				{	records_log.push_back(records);	}

				mutable vector<int> records_log;
			};

			test( PreConsumeCallbackIsCalledWithMinusOneForEmptySequence )
			{
				// INIT
				queue<Foo> q1;
				queue<int> q2;
				pre_consume pc;

				// ACT
				q1.consume(&dummy_consume_foo, pc);

				// ASSERT
				int reference1[] = { -1, };

				assert_equal(reference1, pc.records_log);

				// ACT
				q2.consume(&dummy_consume_int, pc);

				// ASSERT
				int reference2[] = { -1, -1, };

				assert_equal(reference2, pc.records_log);
			}
		end_test_suite
	}
}
