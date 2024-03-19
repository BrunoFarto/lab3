
//****************************************************************************80
//
//  Purpose:
//
//    MAIN is the main program for SCHEDULE_OPENMP.
//
//  Discussion:
//
//    This program demonstrates the difference between default,
//    static and dynamic scheduling for a loop parallelized in OpenMP.
//
//    The purpose of scheduling is to deal with loops in which there is
//    known or suspected imbalance in the work load.  In this example,
//    if the work is divided in the default manner between two threads,
//    the second thread has 3 times the work of the first.
//
//    Both static and dynamic scheduling, if used, even out the work
//    so that both threads have about the same load.  This could be
//    expected to decrease the run time of the loop by about 1/3.
//
//  Licensing:
//
//    This code is distributed under the MIT license.
//
//  Modified:
//
//    10 July 2010
//
//  Author:
//
//    John Burkardt
//

#include <iostream>
#include <iomanip>
#include <omp.h>
#include <cp/timer.h>

using namespace std;

int prime_seq ( int n ) {
    int total = 0;

    for (int i = 2; i <= n; i++ ) {
        int prime = 1;

        for (int j = 2; j < i; j++ ) {
            if ( i % j == 0 ) {
                prime = 0;
                break;
            }
        }
        total = total + prime;
    }

    return total;
}

/**
 * counts primes, using default scheduling.
 * @param n
 * @return
 */
int prime_omp_default ( int n ) {
    int total = 0;
    for (int i = 2; i <= n; i++ ) {
        int prime = 1;

        for (int j = 2; j < i; j++ )
        {
            if ( i % j == 0 )
            {
                prime = 0;
                break;
            }
        }
        total = total + prime;
    }

    return total;
}


/**
 * counts primes, using static scheduling.
 * @param n
 * @return
 */
int prime_omp_static ( int n ) {
    int total = 0;
    for (int i = 2; i <= n; i++ ) {
        int prime = 1;

        for (int j = 2; j < i; j++ )
        {
            if ( i % j == 0 )
            {
                prime = 0;
                break;
            }
        }
        total = total + prime;
    }

    return total;
}

/**
 * counts primes, using dynamic scheduling.
 * @param n
 * @return
 */
int prime_omp_dynamic ( int n )  {
    int total = 0;
    for (int i = 2; i <= n; i++ ) {
        int prime = 1;

        for (int j = 2; j < i; j++ ) {
            if ( i % j == 0 ) {
                prime = 0;
                break;
            }
        }
        total = total + prime;
    }

    return total;
}

int main ( int argc, char *argv[] ) {

    int n_lo = 10000;
    int n_hi = 1310720;
    int n_factor = 2;

    cout << "\n";
    cout << "                           Seq     Default        Static       Dynamic\n";
    cout << "         N     Pi(N)       Time        Time          Time          Time\n";
    cout << "\n";

    int n = n_lo;


    while ( n <= n_hi ) {

        marrow::timer t;
        t.start("seq");
        int primes = prime_seq ( n );
        t.stop("seq");

        t.start("default");
        primes = prime_omp_default( n );
        t.stop("default");

        t.start("static");
        primes = prime_omp_static ( n );
        t.stop("static");

        t.start("dynamic");
        primes = prime_omp_dynamic ( n );
        t.stop("dynamic");

        cout << "  " << setw(8) << n
            << "  " << setw(8) << primes
             << "  " << setw(12) << t.average("seq")
             << "  " << setw(12) << t.average("default")
             << "  " << setw(12) << t.average("static")
             << "  " << setw(12) << t.average("dynamic") << "\n";

        n = n * n_factor;

    }

    return 0;
}