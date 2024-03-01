#include<iostream>
#include<random>
#include<iomanip>
#include"permutation.h"
#include<string>
#include<cmath>
#include<tuple>
#include<mutex>
#include<thread>
#include<cstdio>
#include<set>
#include<algorithm>

using namespace std;

typedef unsigned long long u64;

u64 IV = 0x80400c0600000000UL;
//u64 IV = 0x80400c06;
//u64 IV = 0x8040;

std::random_device rd;
std::mt19937_64 gen(rd());
std::uniform_int_distribution<uint64_t> dis( 0, u64(0xffffffffffffffffUL) );
std::uniform_int_distribution<int> disint( 0, 63 );

const int WIDTH = 64;
const int BIT = 31;
const int THREAD = 128;

u64 encrypt( const u64 key0, const u64 key1, const u64 N0, const u64 N1 ) 
{
    state s { IV, key0, key1, N0, N1 };
    P6();
    return s.x0;
}

void sum( const vector<int> & Index, const u64 xx, const u64 key0, const u64 key1, u64 start, u64 end, u64 & s, const int XX  )
{
    //u64 s = 0UL;
    //u64 xx = dis( gen );
    for ( long long i = start; i < end; i++ )
    {
        u64 x = xx;

        for ( int index = 0; index < BIT; index++ )
           if ( i >> ( BIT - 1 - index ) & 0x1 )
               x ^= 1UL << ( WIDTH - 1 - Index[ index ] );

        //x0 = ( x0 & ( 0xffffffffffffffff ^ mask ) ) ^  ( x & mask );

        //u64 x0 = x &  0x7fffffff; 
        //u64 x0 = x ^ 0x8000000000000000UL; 

        u64 x0 = x ^ (1UL << (63 - XX) );
        s ^= encrypt ( key0, key1, x0, x );
    }

    //return s;
}

int main()
{
    int SUC = 0;

    int off[8] = { 3, 25, 1, 4, 26, 6, 9, 31 };

    for ( int test = 0; test < 100; test++ )
    {
        cout << "Test " << test << endl;
        u64 key0 = dis( gen );
        u64 key1 = dis( gen );

        u64 keyguess[8] = { 0 }; 

        for ( int of = 0; of < 8; of++ )
        {
            for ( int XX = 0; XX < 64; XX++ )
            {
                vector<int> MIX { ( XX + 0 ) % 64, ( XX + 3 ) % 64, ( XX + 25 ) % 64, 
                                  ( XX + 1 ) % 64, ( XX + 4 ) % 64, ( XX + 26 ) % 64, ( XX + 6 ) % 64, ( XX + 9 ) % 64, ( XX + 31 ) % 64 };
                vector<int> MI;

                // find all indexes which is not in MIX
                for ( int i = 0; i < 64; i++ )
                    if ( find( MIX.begin(), MIX.end(), i ) == MIX.end() )
                        MI.push_back( i );

                for ( int check = 0; check < 8; check++ )
                {
                    u64 xx = dis( gen );
                    vector<int> Index{ XX, ( XX + off[of] )% 64 };

                    // randomly select 29 indexes
                    shuffle( MI.begin(), MI.end(), gen );
                    for ( int j = 0; j < 29; j++ )
                        Index.push_back( MI[j] );

                    // to speed up
                    vector<thread> Threads;

                    u64 res = 0;
                    vector<u64> S(THREAD, 0);
                    for ( int step = 0; step < THREAD; step ++ )
                    {
                        u64 start = step * ( 1UL << (BIT - int(log2(THREAD)) ) );
                        u64 end = ( step + 1 ) * ( 1UL << (BIT - int( log2(THREAD)  ) ) );
                        Threads.push_back( thread( sum, ref( Index ), xx, key0, key1, start, end, ref( S[step] ), XX ) );
                    }

                    for ( auto & th : Threads )
                        th.join();

                    for ( int i = 0; i < THREAD; i++ )
                        res ^= S[i];

                    if ( res != 0 )
                    {
                        keyguess[of] ^= 1UL << ( ( 63 - XX - off[of] + 64 ) % 64 );
                        break;
                    }
                }
            }

            cout << hex << (key0 ^ key1) << " ";

            for ( int i = 0; i < 8; i++ )
                keyguess[0] |= keyguess[i];

            cout << hex << keyguess[0] << " " << ( ( key0 ^ key1 ) == keyguess[0] ) << endl;

            if ( keyguess[0] == (key0 ^ key1 ) )
            {
                SUC += 1;
                break;
            }
        }

        cout << endl;
    }

    cout << SUC << endl;
}





