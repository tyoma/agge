#include <agge/pod_vector.h>

#include <algorithm>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			struct A
			{
				int a;

				bool operator ==(const A &rhs) const
				{	return a == rhs.a;	}
			};

			struct B
			{
				int abc;
				char def;

				bool operator ==(const B &rhs) const
				{	return abc == rhs.abc && def == rhs.def;	}
			};
		}

		begin_test_suite( PODVectorTests )
			test( NewVectorHasExpectedState )
			{
				// INIT / ACT
				pod_vector<char> empty1;
				pod_vector<double> empty2(0);
				pod_vector<A> As(10);
				const pod_vector<B> Bs(13211);

				// ACT / ASSERT
				assert_is_true(empty1.empty());
				assert_equal(0u, empty1.size());
				assert_equal(0u, empty1.capacity());
				assert_equal(empty1.end(), empty1.begin());
				assert_is_true(empty2.empty());
				assert_equal(0u, empty2.size());
				assert_equal(0u, empty2.capacity());
				assert_equal(empty2.end(), empty2.begin());

				assert_is_false(As.empty());
				assert_equal(10u, As.size());
				assert_is_true(10u <= As.capacity());
				assert_equal(10, As.end() - As.begin());
				assert_is_false(Bs.empty());
				assert_equal(13211u, Bs.size());
				assert_is_true(13211u <= Bs.capacity());
				assert_equal(13211, Bs.end() - Bs.begin());
			}


			test( AppendingIncreasesSize )
			{
				// INIT
				A somevalue;
				pod_vector<A> v;
				const pod_vector<A> &cv = v;

				// ACT
				v.push_back(somevalue);

				// ASSERT
				assert_equal(1u, cv.size());
				assert_is_false(cv.empty());
				assert_equal(1, v.end() - v.begin());
				assert_equal(1, cv.end() - cv.begin());
				assert_equal(v.begin(), cv.begin());

				// ACT
				v.push_back(somevalue);
				v.push_back(somevalue);

				// ASSERT
				assert_equal(3u, cv.size());
				assert_is_false(cv.empty());
				assert_equal(3, v.end() - v.begin());
				assert_equal(3, cv.end() - cv.begin());
				assert_equal(v.begin(), cv.begin());
			}


			test( AppendingIncreasesSizeFromInitial )
			{
				// INIT
				A somevalue;
				pod_vector<A> v(171);
				const pod_vector<A> &cv = v;

				// ACT
				v.push_back(somevalue);

				// ASSERT
				assert_equal(172u, cv.size());
				assert_is_false(cv.empty());
				assert_equal(172, v.end() - v.begin());
				assert_equal(172, cv.end() - cv.begin());
				assert_equal(v.begin(), cv.begin());

				// ACT
				v.push_back(somevalue);

				// ASSERT
				assert_equal(173u, cv.size());
			}


			test( StateIsResetOnClear )
			{
				// INIT
				pod_vector<int> v1(7);
				const pod_vector<int> &cv1 = v1;
				pod_vector<char> v2(100000);

				v1.push_back(1);
				v1.push_back(13);

				const count_t c1 = v1.capacity();
				const count_t c2 = v2.capacity();

				// ACT
				v1.clear();
				v2.clear();

				// ASSERT
				assert_is_true(v1.empty());
				assert_equal(0u, v1.size());
				assert_equal(c1, v1.capacity());
				assert_equal(cv1.end(), cv1.begin());
				assert_equal(v1.end(), v1.begin());
				assert_equal(c2, v2.capacity());
			}


			test( BufferIsPreservedOnClear )
			{
				// INIT
				pod_vector<int> v(7);
				void * const p = &v[0];

				// ACT
				v.clear();
				v.push_back(3123);

				// ASSERT
				assert_equal(p, &v[0]);
			}


			test( AppendedValuesAreCopied )
			{
				// INIT
				A val1 = {	3	}, val2 = {	5	};
				B val3 = {	1234, 1	}, val4 = {	2345, 11	}, val5 = {	34567, 13	};
				pod_vector<A> vA;
				const pod_vector<A> &cvA = vA;
				pod_vector<B> vB(3);
				const pod_vector<B> &cvB = vB;

				// ACT
				vA.push_back(val1);
				vA.push_back(val2);
				vB.push_back(val3);
				vB.push_back(val4);
				vB.push_back(val5);

				// ACT / ASSERT
				assert_equal(val1, *(vA.begin() + 0));
				assert_equal(val2, *(vA.begin() + 1));
				assert_equal(val1, *(cvA.begin() + 0));
				assert_equal(val2, *(cvA.begin() + 1));

				assert_equal(val3, *(vB.begin() + 3));
				assert_equal(val4, *(vB.begin() + 4));
				assert_equal(val5, *(vB.begin() + 5));
				assert_equal(val3, *(cvB.begin() + 3));
				assert_equal(val4, *(cvB.begin() + 4));
				assert_equal(val5, *(cvB.begin() + 5));
			}


			test( VectorIsIndexed )
			{
				// INIT
				B val1 = {	1234, 1	}, val2 = {	2345, 11	}, val3 = {	34567, 13	};
				pod_vector<char> vA(7);
				const pod_vector<char> &cvA = vA;
				pod_vector<B> vB;
				const pod_vector<B> &cvB = vB;

				vA.push_back(17);
				vA.push_back(123);
				vA.push_back(19);
				vB.push_back(val1);
				vB.push_back(val3);
				vB.push_back(val2);
				vB.push_back(val2);
				vB.push_back(val1);

				// ACT / ASSERT
				assert_equal(17, vA[7]);
				assert_equal(123, vA[8]);
				assert_equal(19, vA[9]);
				assert_equal(17, cvA[7]);
				assert_equal(123, cvA[8]);
				assert_equal(19, cvA[9]);

				assert_equal(val1, vB[0]);
				assert_equal(val1, cvB[0]);
				assert_equal(val3, vB[1]);
				assert_equal(val2, vB[2]);
				assert_equal(val2, vB[3]);
				assert_equal(val1, vB[4]);

				// ACT
				vA[0] = 31;
				vB[4] = val3;

				// ACT / ASSERT
				assert_equal(31, vA[0]);
				assert_equal(val3, vB[4]);
			}


			test( IndexedReferencesAreThoseActuallyStored )
			{
				// INIT
				B val1 = {	1234, 1	};
				pod_vector<char> vA(4);
				const pod_vector<char> &cvA = vA;
				pod_vector<B> vB(5);
				const pod_vector<B> &cvB = vB;

				vA.push_back(17);
				vA.push_back(123);
				vB.push_back(val1);

				// ASSERT
				assert_equal(&*(cvA.begin() + 0), &vA[0]);
				assert_equal(&*(vA.begin() + 1), &cvA[1]);
				assert_equal(&*(cvB.begin() + 0), &vB[0]);
			}


			test( InitialCapacityEqualsToInitialSize )
			{
				// INIT / ACT
				pod_vector<int> v1;
				pod_vector<char> v2(0);
				pod_vector<char> v3(1);
				pod_vector<double> v4(7);
				pod_vector<float> v5(83);

				// ACT / ASSERT
				assert_equal(0u, v1.capacity());
				assert_equal(0u, v2.capacity());
				assert_equal(1u, v3.capacity());
				assert_equal(7u, v4.capacity());
				assert_equal(83u, v5.capacity());
			}


			test( AppendingOverCapacityIncresesCapacityByHalfOfCurrent )
			{
				// INIT
				A somevalue;
				pod_vector<A> v1(3), v2(4);

				v1.clear();
				v2.clear();

				v1.push_back(somevalue);
				v1.push_back(somevalue);
				v1.push_back(somevalue);
				v2.push_back(somevalue);
				v2.push_back(somevalue);
				v2.push_back(somevalue);
				v2.push_back(somevalue);

				// ASSERT
				assert_equal(3u, v1.capacity());
				assert_equal(4u, v2.capacity());

				// ACT
				v1.push_back(somevalue);
				v2.push_back(somevalue);

				// ASSERT
				assert_equal(4u, v1.capacity());
				assert_equal(6u, v2.capacity());
			}


			test( IncresingCapacityPreservesOldValuesAndPushesNew )
			{
				// INIT
				B val1 = {	1234, 1	}, val2 = {	2345, 11	}, val3 = {	34567, 13	}, val4 = {	134567, 17	};
				pod_vector<int> v1(2);
				pod_vector<B> v2(3);

				v1.clear(); // 2 -> capacity
				v1.push_back(13);
				v1.push_back(1710013);

				v2.clear(); // 3 -> capacity
				v2.push_back(val1);
				v2.push_back(val2);
				v2.push_back(val3);

				const int *previous_buffer1 = v1.data();
				const B *previous_buffer2 = v2.data();

				// ACT
				v1.push_back(19);
				v2.push_back(val4);

				// ACT / ASSERT
				assert_not_equal(previous_buffer1, v1.data());
				assert_equal(13, *(v1.data() + 0));
				assert_equal(1710013, *(v1.data() + 1));
				assert_equal(19, *(v1.data() + 2));

				assert_not_equal(previous_buffer2, v2.data());
				assert_equal(val1, *(v2.data() + 0));
				assert_equal(val2, *(v2.data() + 1));
				assert_equal(val3, *(v2.data() + 2));
				assert_equal(val4, *(v2.data() + 3));
			}


			test( CopyingVectorCopiesElements )
			{
				// INIT
				B val1 = {	1234, 1	}, val2 = {	2345, 11	}, val3 = {	34567, 13	};
				pod_vector<B> v1;
				pod_vector<int> v2(10);

				v2.clear(); // 10 -> capacity

				v1.push_back(val1);
				v1.push_back(val2);
				v1.push_back(val3);
				v2.push_back(13);

				// ACT
				pod_vector<B> copied1(v1);
				pod_vector<int> copied2(v2);

				// ACT / ASSERT
				assert_equal(3u, copied1.capacity());
				assert_equal(3u, copied1.size());
				assert_not_equal(copied1.data(), v1.data());
				assert_equal(val1, *(copied1.data() + 0));
				assert_equal(val2, *(copied1.data() + 1));
				assert_equal(val3, *(copied1.data() + 2));

				assert_equal(10u, copied2.capacity());
				assert_equal(1u, copied2.size());
				assert_not_equal(copied2.data(), v2.data());
				assert_equal(13, *(copied2.data() + 0));
			}


			test( ResizedVectorGuranteesItsRangeSize )
			{
				// INIT
				pod_vector<float> v1;
				pod_vector<short> v2;

				// ACT
				v1.resize(113);
				v2.resize(19);

				// ASSERT
				assert_equal(113u, v1.size());
				assert_equal(113, v1.end() - v1.begin());
				assert_equal(19u, v2.size());
				assert_equal(19, v2.end() - v2.begin());

				// ACT
				v1.resize(193);
				v2.resize(11);

				// ASSERT
				assert_equal(193u, v1.size());
				assert_equal(193, v1.end() - v1.begin());
				assert_equal(11u, v2.size());
				assert_equal(11, v2.end() - v2.begin());
			}


			test( ResizingToASmallerSizeDoesNotDecreaseCapacity )
			{
				// INIT
				pod_vector<float> v1(10);
				pod_vector<float> v2(100);
				count_t c1 = v1.capacity();
				count_t c2 = v2.capacity();

				// ACT
				v1.resize(7);
				v2.resize(71);

				// ASSERT
				assert_equal(c1, v1.capacity());
				assert_equal(c2, v2.capacity());
			}


			test( ResizingToALargerSizeIncreaseCapacity )
			{
				// INIT
				pod_vector<float> v1(10);
				pod_vector<float> v2(100);

				// ACT
				v1.resize(17);
				v2.resize(711);

				// ASSERT
				assert_is_true(17u <= v1.capacity());
				assert_is_true(711u <= v2.capacity());
			}


			test( ResizePreservesContent )
			{
				// INIT
				pod_vector<float> v1;
				pod_vector<char> v2;

				v1.push_back(1.7f); v1.push_back(7.7f); v1.push_back(9.1f); v1.push_back(1.9f);
				v2.push_back('a'); v2.push_back('g'); v2.push_back('c'); v2.push_back('d'); v2.push_back('f');

				// ACT
				v1.resize(3);
				v2.resize(11);

				// ASSERT
				assert_equal(1.7f, v1[0]);
				assert_equal(7.7f, v1[1]);
				assert_equal(9.1f, v1[2]);

				assert_equal('a', v2[0]);
				assert_equal('g', v2[1]);
				assert_equal('c', v2[2]);
				assert_equal('d', v2[3]);
				assert_equal('f', v2[4]);
			}


			test( AssigningAValueResizesVectorToAppropriateSizeAndFillsIt )
			{
				// INIT
				pod_vector<float> v1;
				pod_vector<char> v2;
				pod_vector<unsigned char> v3;

				// ACT
				v1.assign(179, 3.3112f);
				v2.assign(17, 'r');
				v3.assign(29, 191);

				// ASSERT
				assert_equal(179u, v1.size());
				assert_equal(v1.end(), find_if(v1.begin(), v1.end(), bind1st(not_equal_to<float>(), 3.3112f)));
				assert_equal(17u, v2.size());
				assert_equal(v2.end(), find_if(v2.begin(), v2.end(), bind1st(not_equal_to<char>(), 'r')));
				assert_equal(29u, v3.size());
				assert_equal(v3.end(), find_if(v3.begin(), v3.end(), bind1st(not_equal_to<unsigned char>(), 191)));

				// ACT
				v1.assign(5, 3.15114926f);
				v2.assign(918, 'Z');
				v3.assign(18, 23);

				// ASSERT
				assert_equal(5u, v1.size());
				assert_equal(v1.end(), find_if(v1.begin(), v1.end(), bind1st(not_equal_to<float>(), 3.15114926f)));
				assert_equal(918u, v2.size());
				assert_equal(v2.end(), find_if(v2.begin(), v2.end(), bind1st(not_equal_to<char>(), 'Z')));
				assert_equal(18u, v3.size());
				assert_equal(v3.end(), find_if(v3.begin(), v3.end(), bind1st(not_equal_to<unsigned char>(), 23)));
			}


			test( PoppingLastElementReducesSize )
			{
				// INIT
				pod_vector<char> v;

				v.push_back('r');
				v.push_back('a');
				v.push_back('w');

				// ACT
				v.pop_back();

				// ASSERT
				assert_equal(2u, v.size());

				// ACT
				v.pop_back();
				v.pop_back();

				// ASSERT
				assert_equal(0u, v.size());
			}


			test( AppendingReturnsIteratorToTheElementAppended )
			{
				// INIT
				pod_vector<char> v1;
				pod_vector<int> v2;

				// ACT / ASSERT
				assert_equal('r', *v1.push_back('r'));
				assert_equal(1801123, *v2.push_back(1801123));

				// ACT
				*v2.push_back(13) = 19;

				// ASSERT
				assert_equal(19, *(v2.end() - 1));
			}


			test( SwappingVectorsOnlySwapsTheirBaseIterators )
			{
				// INIT
				pod_vector<int> v1, v2;

				v1.assign(7, 139911);
				v2.assign(131, 11);

				const pod_vector<int>::iterator begin1 = v1.begin(), end1 = v1.end();
				const pod_vector<int>::iterator begin2 = v2.begin(), end2 = v2.end();
				const count_t capacity1 = v1.capacity();
				const count_t capacity2 = v2.capacity();

				// ACT
				v1.swap(v2);

				// ASSERT
				assert_equal(begin1, v2.begin());
				assert_equal(end1, v2.end());
				assert_equal(capacity1, v2.capacity());

				assert_equal(begin2, v1.begin());
				assert_equal(end2, v1.end());
				assert_equal(capacity2, v1.capacity());
			}
		end_test_suite
	}
}
