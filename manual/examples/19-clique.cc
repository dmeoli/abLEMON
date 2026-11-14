// The two maximum clique algorithms of chapter 19: on small random graphs
// TomitaSekiMc proves its clique maximum (checked against the enumeration
// of all the subsets), and on a larger graph with a planted clique the
// heuristic GrossoLocatelliPullanMc finds a clique no larger than the
// maximum one, while a limit on the search tree stops the exact search.

#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/random.h>
#include <lemon/tomita_seki_mc.h>
#include <lemon/grosso_locatelli_pullan_mc.h>

using namespace lemon;

typedef ListGraph Graph;

void randomGraph( Graph & g , std::vector< Graph::Node > & v , int n ,
		  double p ,
		  Random & r )
{
 for( int i = 0 ; i < n ; ++i )
  v.push_back( g.addNode() );
 for( int i = 0 ; i < n ; ++i )
  for( int j = i + 1 ; j < n ; ++j )
   if( r() < p )
    g.addEdge( v[ i ] , v[ j ] );
 }

bool adjacent( const Graph & g , Graph::Node a , Graph::Node b )
{
 return( findEdge( g , a , b ) != INVALID );
 }

// whether the nodes marked in the map are pairwise adjacent and k
template< typename Map >
bool isClique( const Graph & g , const Map & m , int k )
{
 std::vector< Graph::Node > c;
 for( Graph::NodeIt u( g ) ; u != INVALID ; ++u )
  if( m[ u ] )
   c.push_back( u );
 for( std::size_t i = 0 ; i < c.size() ; ++i )
  for( std::size_t j = i + 1 ; j < c.size() ; ++j )
   if( ! adjacent( g , c[ i ] , c[ j ] ) )
    return( false );
 return( int( c.size() ) == k );
 }

// the largest clique by the enumeration of the 2^n subsets
int bruteForce( const Graph & g , const std::vector< Graph::Node > & v )
{
 int n = int( v.size() ) , best = 0;
 for( long s = 1 ; s < ( 1L << n ) ; ++s ) {
  int k = 0;
  bool clique = true;
  for( int i = 0 ; i < n && clique ; ++i ) {
   if( ! ( s >> i & 1 ) )
    continue;
   ++k;
   for( int j = i + 1 ; j < n && clique ; ++j )
    if( ( s >> j & 1 ) && ! adjacent( g , v[ i ] , v[ j ] ) )
     clique = false;
   }
  if( clique && k > best )
   best = k;
  }
 return( best );
 }

int main()
{
 Random r( 2026 );
 bool ok = true;

 for( int t = 0 ; t < 5 && ok ; ++t ) {  // small graphs
  Graph g;
  std::vector< Graph::Node > v;
  randomGraph( g , v , 14 , 0.5 , r );
  TomitaSekiMc< Graph > mc( g );
  Graph::NodeMap< bool > in( g );
  ok = mc.run() && mc.cliqueSize() == bruteForce( g , v );
  mc.cliqueMap( in );
  ok = ok && isClique( g , in , mc.cliqueSize() );
  }

 Graph g;  // a planted clique
 std::vector< Graph::Node > v;
 randomGraph( g , v , 60 , 0.2 , r );
 for( int i = 0 ; i < 9 ; ++i )
  for( int j = i + 1 ; j < 9 ; ++j )
   if( ! adjacent( g , v[ 3 * i ] , v[ 3 * j ] ) )
    g.addEdge( v[ 3 * i ] , v[ 3 * j ] );

 TomitaSekiMc< Graph > exact( g );
 bool proved = exact.run();
 Graph::NodeMap< bool > in( g );
 exact.cliqueMap( in );
 ok = ok && proved && exact.cliqueSize() >= 9 &&
      isClique( g , in , exact.cliqueSize() );

 GrossoLocatelliPullanMc< Graph > heur( g , 7 );  // seeded generator
 GrossoLocatelliPullanMc< Graph >::TerminationCause cause =
   heur.iterationLimit( 200 ).sizeLimit( exact.cliqueSize() ).run();
 Graph::NodeMap< bool > hin( g );
 heur.cliqueMap( hin );
 ok = ok && heur.cliqueSize() <= exact.cliqueSize() &&
      isClique( g , hin , heur.cliqueSize() ) &&
      ( cause != heur.SIZE_LIMIT || heur.cliqueSize() == exact.cliqueSize() );

 // with a penalty delay of 1 each increase of a penalty is undone at once,
 // and PENALTY_BASED picks the same nodes as RANDOM with the same seed
 GrossoLocatelliPullanMc< Graph > rnd_rule( g , 11 ) , pen_rule( g , 11 );
 rnd_rule.iterationLimit( 50 ).run( rnd_rule.RANDOM );
 pen_rule.iterationLimit( 50 ).penaltyDelay( 1 ).run( pen_rule.PENALTY_BASED );
 Graph::NodeMap< bool > rin( g ) , pin( g );
 rnd_rule.cliqueMap( rin );
 pen_rule.cliqueMap( pin );
 for( Graph::NodeIt u( g ) ; u != INVALID ; ++u )
  ok = ok && rin[ u ] == pin[ u ];

 TomitaSekiMc< Graph > stopped( g );  // a stopped search
 ok = ok && ! stopped.treeLimit( 5 ).run() && stopped.treeSize() == 5;

 std::cout << "maximum clique " << exact.cliqueSize() << " (search tree "
	   << exact.treeSize() << " nodes), heuristic clique "
	   << heur.cliqueSize() << ", cause " << int( cause ) << std::endl;
 return( ok ? 0 : 1 );
 }
