// The heaps (chapter 7): each heap of the library sorts the same sequence
// of priorities, after some of them are decreased and increased; a binary
// heap with std::greater and a bucket heap with MIN = false give the
// largest first; a radix heap refuses a priority below the last one
// removed.

#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
#include <lemon/bin_heap.h>
#include <lemon/binomial_heap.h>
#include <lemon/bucket_heap.h>
#include <lemon/dheap.h>
#include <lemon/fib_heap.h>
#include <lemon/maps.h>
#include <lemon/pairing_heap.h>
#include <lemon/quad_heap.h>
#include <lemon/radix_heap.h>

using namespace lemon;

const int N = 12;
const int prio[ N ] = { 9 , 4 , 17 , 4 , 0 , 23 , 8 , 15 , 1 , 12 , 6 , 30 };

// pushes the items 0 .. N-1, decreases item 5 to 2 and increases item 4 to
// 20, then pops them all: true if the priorities come out sorted and the
// states are those of the concept
template< typename Heap >
bool sortTest()
{
 RangeMap< int > cross( N , Heap::PRE_HEAP );
 Heap heap( cross );
 std::vector< int > v( prio , prio + N );
 for( int i = 0 ; i < N ; ++i )
  heap.push( i , v[ i ] );
 heap.decrease( 5 , 2 );
 v[ 5 ] = 2;
 heap.increase( 4 , 20 );
 v[ 4 ] = 20;
 heap.set( 11 , 3 );
 v[ 11 ] = 3;  // set() decreases here
 std::sort( v.begin() , v.end() );
 bool ok = heap.size() == N && heap.state( 7 ) == Heap::IN_HEAP;
 for( int k = 0 ; k < N ; ++k ) {
  ok = ok && heap.prio() == v[ k ] && heap[ heap.top() ] == v[ k ];
  int i = heap.top();
  heap.pop();
  ok = ok && heap.state( i ) == Heap::POST_HEAP;
  }
 return( ok && heap.empty() );
 }

int main()
{
 typedef RangeMap< int > IM;
 bool ok =
   sortTest< BinHeap< int , IM > >() && sortTest< QuadHeap< int , IM > >() &&
   sortTest< DHeap< int , IM > >() && sortTest< DHeap< int , IM , 3 > >() &&
   sortTest< FibHeap< int , IM > >() && sortTest< PairingHeap< int ,
							       IM > >() &&
   sortTest< BinomialHeap< int , IM > >() && sortTest< RadixHeap< IM > >() &&
   sortTest< BucketHeap< IM > >();

 // the largest priority first
 IM c1( N , -1 ) , c2( N , -1 );
 BinHeap< int , IM , std::greater< int > > maxheap( c1 );
 BucketHeap< IM , false > maxbucket( c2 );
 for( int i = 0 ; i < N ; ++i ) {
  maxheap.push( i , prio[ i ] );
  maxbucket.push( i , prio[ i ] );
  }
 ok = ok && maxheap.prio() == 30 && maxbucket.prio() == 30;

 // SimpleBucketHeap: push and pop only
 IM c3( N , -1 );
 SimpleBucketHeap< IM > simple( c3 );
 for( int i = 0 ; i < N ; ++i )
  simple.push( i , prio[ i ] );
 int last = -1;
 while( ! simple.empty() ) {
  ok = ok && simple.prio() >= last;
  last = simple.prio();
  simple.pop();
  }

 // RadixHeap: no priority below the last one removed
 IM c4( N , -1 );
 RadixHeap< IM > radix( c4 );
 radix.push( 0 , 10 );
 radix.push( 1 , 12 );
 radix.pop();          // removes 10
 radix.push( 2 , 11 );  // allowed: not below 10
 bool thrown = false;
 try {
  radix.push( 3 , 9 );
  }
 catch( const RadixHeap< IM >::PriorityUnderflowError & ) {
  thrown = true;
  }
 ok = ok && thrown;

 std::cout << ( ok ? "all the heaps agree" : "a heap is wrong" ) << std::endl;
 return( ok ? 0 : 1 );
 }
