#include <agge.async/circular.h>

#include <tests/common/scoped_ptr.h>
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
				Foo(int x)
					: X(x)
				{	}

				~Foo()
				{	X = 0;	}

				int X;

				bool operator ==(const Foo &rhs) const
				{	return X == rhs.X;	}
			};

			struct PolyBase
			{
				virtual ~PolyBase() {	}
				virtual uint16_t get_size() const = 0;
			};

			template <typename T, uint16_t N>
			class PolyDerived : public PolyBase
			{
			public:
				PolyDerived(const T &value)
					: _value(value)
				{	}

				bool operator ==(const PolyDerived &rhs) const
				{	return _value == rhs._value;	}

			private:
				virtual uint16_t get_size() const {	return N;	}

			private:
				char padding[N];
				T _value;
			};

			template <typename T, uint16_t N>
			class PolyDerivedNC : public PolyDerived<T, N>
			{
			public:
				PolyDerivedNC(const T &value)
					: PolyDerived<T, N>(value)
				{	}

				PolyDerivedNC(PolyDerivedNC &other)
					: PolyDerived<T, N>(other)
				{	}
			};

			class TrackedCopies
			{
			public:
				TrackedCopies(int &n)
					: _n(n)
				{	++_n;	}

				TrackedCopies(const TrackedCopies &other)
					: _n(other._n)
				{	++_n;	}

				~TrackedCopies()
				{	--_n;	}

			private:
				int &_n;
			};

			class preconsume
			{
			public:
				preconsume(int &n, bool result)
					: _n(n), _result(result)
				{	}

				bool operator ()(int n) const
				{	return _n = n, _result;	}

			private:
				const preconsume &operator =(const preconsume &rhs);

			private:
				int &_n;
				bool _result;
			};

			class postproduce
			{
			public:
				postproduce(int &n)
					: _n(n)
				{	}

				void operator ()(int n) const
				{	_n = n;	}

			private:
				const postproduce &operator =(const postproduce &rhs);

			private:
				int &_n;
			};

			template <typename T>
			class consumer
			{
			public:
				consumer(vector<T> &log, vector<T *> *plog = 0)
					: _log(log), _plog(plog)
				{	}

				void operator ()(T &value) const
				{
					_log.push_back(value);
					if (_plog)
						_plog->push_back(&value);
				}

			private:
				const consumer &operator =(const consumer &rhs);

			private:
				vector<T> &_log;
				vector<T *> *_plog;
			};

			template <typename FinalT>
			class consuming_asserter
			{
			public:
				consuming_asserter(const FinalT &reference)
					: _reference(reference)
				{	}

				template <typename BaseT>
				void operator ()(const BaseT &value) const
				{	assert_equal(_reference, static_cast<const FinalT&>(value));	}

			private:
				FinalT _reference;
			};

			template <typename FinalT>
			class consuming_asserter_nc
			{
			public:
				consuming_asserter_nc(FinalT &reference)
					: _reference(reference)
				{	}

				consuming_asserter_nc(const consuming_asserter_nc &other)
					: _reference(other._reference)
				{	}

				template <typename BaseT>
				void operator ()(const BaseT &value) const
				{	assert_equal(_reference, static_cast<const FinalT&>(value));	}

			private:
				mutable FinalT _reference;
			};

			template <typename FinalT>
			consuming_asserter<FinalT> assert_value(const FinalT &reference)
			{	return consuming_asserter<FinalT>(reference);	}

			template <typename FinalT>
			consuming_asserter_nc<FinalT> assert_value_nc(FinalT &reference)
			{	return consuming_asserter_nc<FinalT>(reference);	}

			template <typename T>
			void failing_consume(T &/*value*/)
			{	assert_is_true(false);	}

			template <typename T>
			void dummy_consume(T &/*value*/)
			{	}
		}


		begin_test_suite( CircularBufferTests )
			test( EmptyBufferPassesZeroCountToPreconsume )
			{
				// INIT
				int n = 1110;
				circular_buffer<int> b1;
				circular_buffer<Foo> b2;

				// ACT / ASSERT
				assert_is_false(b1.consume(&failing_consume<int>, preconsume(n, false)));

				// ASSERT
				assert_equal(n, 0);

				// INIT
				n = 0;

				// ACT
				assert_is_false(b2.consume(&failing_consume<Foo>, preconsume(n, false)));

				// ASSERT
				assert_equal(n, 0);
			}


			test( RepeatingAvoidedAccessReturnsZeroElementsToPreconsume )
			{
				// INIT
				int n;
				vector<int> log;
				circular_buffer<int> b;

				b.consume(consumer<int>(log), preconsume(n, false));
				n = 1000;

				// ACT
				b.consume(consumer<int>(log), preconsume(n, false));

				// ASSERT
				assert_equal(0, n);
			}


			test( ProducedValueIsRetrievedOnConsume )
			{
				// INIT
				int n[] = { 1, 1, 1, 1, };
				vector<int> log1;
				vector<Foo> log2;
				circular_buffer<int> b1, b2;
				circular_buffer<Foo> b3, b4;

				// ACT
				b1.produce(17, postproduce(n[0]));
				b2.produce(172211, postproduce(n[1]));
				b3.produce(Foo(2), postproduce(n[2]));
				b4.produce(Foo(2119922), postproduce(n[3]));

				// ASSERT
				assert_equal(1, n[0]);
				assert_equal(1, n[1]);
				assert_equal(1, n[2]);
				assert_equal(1, n[3]);

				// ACT / ASSERT
				assert_is_true(b1.consume(consumer<int>(log1), preconsume(n[0], true)));
				assert_is_true(b3.consume(consumer<Foo>(log2), preconsume(n[2], true)));

				// ASSERT
				assert_equal(1, n[0]);
				assert_equal(1, n[2]);

				int reference1[] = { 17, };
				Foo reference3[] = { Foo(2), };

				assert_equal(reference1, log1);
				assert_equal(reference3, log2);

				// ACT
				b2.consume(consumer<int>(log1), preconsume(n[1], true));
				b4.consume(consumer<Foo>(log2), preconsume(n[3], true));

				// ASSERT
				assert_equal(1, n[1]);
				assert_equal(1, n[3]);

				int reference2[] = { 17, 172211, };
				Foo reference4[] = { Foo(2), Foo(2119922), };

				assert_equal(reference2, log1);
				assert_equal(reference4, log2);
			}


			struct preconsume_check_zero
			{
				bool operator ()(int) const
				{
					int n = 11;

					buffer.produce(123, postproduce(n));
					assert_equal(0, n);
					return true;
				}

				circular_buffer<int> &buffer;
			};

			test( ProducingAnElementWhileConsumerIsWaitingGivesZeroElementsToPostproduce )
			{
				// INIT
				circular_buffer<int> buffer;
				preconsume_check_zero pc = { buffer };
				vector<int> log;

				// ACT / ASSERT
				buffer.consume(consumer<int>(log), pc);

				// ASSERT
				int reference[] = { 123, };

				assert_equal(reference, log);
			}


			struct postproduce_check_presence
			{
				void operator ()(int) const
				{
					int n = 11;
					vector<int> log;

					buffer.consume(consumer<int>(log), preconsume(n, true));
					assert_equal(1, n);
					int reference[] = { 13222, };
					assert_equal(reference, log);
				}

				circular_buffer<int> &buffer;
			};

			test( PostProduceIsCalledWhenElementHasAlreadyBeenPut )
			{
				// INIT
				circular_buffer<int> buffer;
				postproduce_check_presence pp = { buffer };

				// ACT / ASSERT
				buffer.produce(13222, pp);
			}


			test( SeveralElementsAreWrittenAndRead )
			{
				// INIT
				int n[2];
				circular_buffer<double> buffer1;
				circular_buffer<Foo> buffer2;
				vector<double> log1;
				vector<Foo> log2;

				// ACT
				buffer1.produce(13.0, postproduce(n[0]));
				buffer1.produce(17.0, postproduce(n[0]));
				buffer1.produce(19131711.0, postproduce(n[0]));
				buffer2.produce(Foo(11), postproduce(n[1]));
				buffer2.produce(Foo(111), postproduce(n[1]));
				buffer2.produce(Foo(1111), postproduce(n[1]));
				buffer2.produce(Foo(11111), postproduce(n[1]));
				buffer2.produce(Foo(1111), postproduce(n[1]));

				// ASSERT
				assert_equal(3, n[0]);
				assert_equal(5, n[1]);

				// ACT
				buffer1.consume(consumer<double>(log1), preconsume(n[0], true));
				buffer2.consume(consumer<Foo>(log2), preconsume(n[1], true));

				// ASSERT
				assert_equal(3, n[0]);
				assert_equal(5, n[1]);

				// ACT
				buffer1.consume(consumer<double>(log1), preconsume(n[0], true));
				buffer1.consume(consumer<double>(log1), preconsume(n[0], true));
				buffer2.consume(consumer<Foo>(log2), preconsume(n[1], true));
				buffer2.consume(consumer<Foo>(log2), preconsume(n[1], true));
				buffer2.consume(consumer<Foo>(log2), preconsume(n[1], true));
				buffer2.consume(consumer<Foo>(log2), preconsume(n[1], true));

				// ASSERT
				double reference1[] = { 13.0, 17.0, 19131711.0, };
				Foo reference2[] = { Foo(11), Foo(111), Foo(1111), Foo(11111), Foo(1111), };

				assert_equal(reference1, log1);
				assert_equal(reference2, log2);
			}


			test( ElementsRollOverTheEndOfTheBuffer )
			{
				// INIT
				int n;
				circular_buffer< double, static_entry<double> > buffer1(5 * sizeof(double));
				circular_buffer< char, static_entry<char> > buffer2(3 * sizeof(char));
				vector<double> log1;
				vector<char> log2;
				vector<double *> plog1;
				vector<char *> plog2;

				buffer1.produce(1.0, postproduce(n));
				buffer1.produce(1.0, postproduce(n));
				buffer1.consume(consumer<double>(log1), preconsume(n, true));
				buffer1.consume(consumer<double>(log1), preconsume(n, true));
				buffer2.produce((char)13, postproduce(n));
				buffer2.consume(consumer<char>(log2), preconsume(n, true));
				log1.clear();
				log2.clear();

				// ACT
				buffer1.produce(13.0, postproduce(n));
				buffer1.produce(11.0, postproduce(n));
				buffer1.produce(1.1, postproduce(n));
				buffer1.produce(1.3, postproduce(n));
				buffer1.produce(1.7, postproduce(n));
				buffer2.produce((char)17, postproduce(n));
				buffer2.produce((char)113, postproduce(n));
				buffer2.produce((char)31, postproduce(n));
				buffer1.consume(consumer<double>(log1, &plog1), preconsume(n, true));
				buffer1.consume(consumer<double>(log1, &plog1), preconsume(n, true));
				buffer1.consume(consumer<double>(log1, &plog1), preconsume(n, true));
				buffer1.consume(consumer<double>(log1, &plog1), preconsume(n, true));
				buffer1.consume(consumer<double>(log1, &plog1), preconsume(n, true));
				buffer2.consume(consumer<char>(log2, &plog2), preconsume(n, true));
				buffer2.consume(consumer<char>(log2, &plog2), preconsume(n, true));
				buffer2.consume(consumer<char>(log2, &plog2), preconsume(n, true));

				// ASSERT
				double reference1[] = { 13.0, 11.0, 1.1, 1.3, 1.7, };
				char reference2[] = { 17, 113, 31, };

				assert_equal(reference1, log1);
				assert_is_true(plog1[3] < plog1[4]);
				assert_is_true(plog1[4] < plog1[0]);

				assert_equal(reference2, log2);
				assert_is_true(plog2[2] < plog2[0]);
			}


			test( ElementsRollOverTheEndOfTheUnevenlySizedBuffer )
			{
				// INIT
				int n;
				circular_buffer< double, static_entry<double> > buffer(5 * sizeof(double) - 1);
				vector<double> log;
				vector<double *> plog;

				buffer.produce(1.0, postproduce(n));
				buffer.produce(1.0, postproduce(n));
				buffer.consume(consumer<double>(log), preconsume(n, true));
				buffer.consume(consumer<double>(log), preconsume(n, true));
				log.clear();

				// ACT
				buffer.produce(13.0, postproduce(n));
				buffer.produce(1.1, postproduce(n));
				buffer.produce(1.3, postproduce(n));
				buffer.produce(1.7, postproduce(n));
				buffer.consume(consumer<double>(log, &plog), preconsume(n, true));
				buffer.consume(consumer<double>(log, &plog), preconsume(n, true));
				buffer.consume(consumer<double>(log, &plog), preconsume(n, true));
				buffer.consume(consumer<double>(log, &plog), preconsume(n, true));

				// ASSERT
				double reference[] = { 13.0, 1.1, 1.3, 1.7, };

				assert_equal(reference, log);
				assert_is_true(plog[2] < plog[3]);
				assert_is_true(plog[3] < plog[0]);
			}


			test( ElementsAreCopyConstructedOnProduce )
			{
				// INIT
				int n, copies = 0;
				circular_buffer<TrackedCopies> buffer;

				// ACT
				buffer.produce(TrackedCopies(copies), postproduce(n));
				buffer.produce(TrackedCopies(copies), postproduce(n));

				// ASSERT
				assert_equal(2, copies);

				// ACT
				buffer.produce(TrackedCopies(copies), postproduce(n));
				buffer.produce(TrackedCopies(copies), postproduce(n));
				buffer.produce(TrackedCopies(copies), postproduce(n));

				// ASSERT
				assert_equal(5, copies);
			}


			test( ElementsAreDestroyedOnConsume )
			{
				// INIT
				int n, copies = 0;
				circular_buffer<TrackedCopies> buffer;

				buffer.produce(TrackedCopies(copies), postproduce(n));
				buffer.produce(TrackedCopies(copies), postproduce(n));
				buffer.produce(TrackedCopies(copies), postproduce(n));
				buffer.produce(TrackedCopies(copies), postproduce(n));
				buffer.produce(TrackedCopies(copies), postproduce(n));

				// ACT
				buffer.consume(&dummy_consume<TrackedCopies>, preconsume(n, true));

				// ASSERT
				assert_equal(4, copies);

				// ACT
				buffer.consume(&dummy_consume<TrackedCopies>, preconsume(n, true));
				buffer.consume(&dummy_consume<TrackedCopies>, preconsume(n, true));
				buffer.consume(&dummy_consume<TrackedCopies>, preconsume(n, true));
				buffer.consume(&dummy_consume<TrackedCopies>, preconsume(n, true));

				// ASSERT
				assert_equal(0, copies);
			}


			test( ElementsAreDestroyedOnBufferDestroy )
			{
				// INIT
				int n, copies1 = 0, copies2 = 0;
				scoped_ptr< circular_buffer<TrackedCopies> > buffer1(new circular_buffer<TrackedCopies>);
				scoped_ptr< circular_buffer<TrackedCopies> > buffer2(new circular_buffer<TrackedCopies>);

				buffer1->produce(TrackedCopies(copies1), postproduce(n));
				buffer1->produce(TrackedCopies(copies1), postproduce(n));
				buffer2->produce(TrackedCopies(copies2), postproduce(n));
				buffer2->produce(TrackedCopies(copies2), postproduce(n));
				buffer2->produce(TrackedCopies(copies2), postproduce(n));

				// ACT
				buffer1.reset();

				// ASSERT
				assert_equal(0, copies1);
				assert_equal(3, copies2);

				// ACT
				buffer2.reset();

				// ASSERT
				assert_equal(0, copies1);
				assert_equal(0, copies2);
			}


			test( WrappedElementsAreDestroyedOnBufferDestroy )
			{
				// INIT
				int n, copies = 0;
				scoped_ptr< circular_buffer<TrackedCopies> > buffer(new circular_buffer<TrackedCopies>(5 * sizeof(TrackedCopies)));

				buffer->produce(TrackedCopies(copies), postproduce(n));
				buffer->produce(TrackedCopies(copies), postproduce(n));
				buffer->produce(TrackedCopies(copies), postproduce(n));
				buffer->produce(TrackedCopies(copies), postproduce(n));
				buffer->produce(TrackedCopies(copies), postproduce(n));
				buffer->consume(&dummy_consume<TrackedCopies>, preconsume(n, true));
				buffer->produce(TrackedCopies(copies), postproduce(n));

				// ACT
				buffer.reset();

				// ASSERT
				assert_equal(0, copies);
			}


			test( PolymorphicElementsCanBeStoredInQueue )
			{
				// INIT
				int n;
				circular_buffer< PolyBase, poly_entry<PolyBase> > buffer;

				// ACT
				buffer.produce(PolyDerived<string, 5>("one"), postproduce(n));
				buffer.produce(PolyDerived<int, 15>(2), postproduce(n));
				buffer.produce(PolyDerived<double, 23>(3.0), postproduce(n));
				buffer.produce(PolyDerived<char, 7>(4), postproduce(n));

				// ACT / ASSERT
				buffer.consume(assert_value(PolyDerived<string, 5>("one")), preconsume(n, true));
				buffer.consume(assert_value(PolyDerived<int, 15>(2)), preconsume(n, true));
				buffer.consume(assert_value(PolyDerived<double, 23>(3.0)), preconsume(n, true));
				buffer.consume(assert_value(PolyDerived<char, 7>(4)), preconsume(n, true));
			}


			test( NonConstReferenceCanBeUsedToProduceAnObject )
			{
				// INIT
				int n;
				circular_buffer< PolyBase, poly_entry<PolyBase> > buffer;
				PolyDerivedNC<string, 5> v1("one");
				PolyDerivedNC<int, 15> v2(2);
				PolyDerivedNC<double, 23> v3(3.0);
				PolyDerivedNC<char, 7> v4(4);

				// ACT
				buffer.produce(v1, postproduce(n));
				buffer.produce(v2, postproduce(n));
				buffer.produce(v3, postproduce(n));
				buffer.produce(v4, postproduce(n));

				// ACT / ASSERT
				buffer.consume(assert_value_nc(v1), preconsume(n, true));
				buffer.consume(assert_value_nc(v2), preconsume(n, true));
				buffer.consume(assert_value_nc(v3), preconsume(n, true));
				buffer.consume(assert_value_nc(v4), preconsume(n, true));
			}

		end_test_suite
	}
}
