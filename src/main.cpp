
#include <iostream>
#include <string>
#include <sstream>
#include <set>
#include <utility>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include "logical_expr.hpp"
#include "quine_mccluskey.hpp"

using namespace std;

void print_truth_table(const logical_expr::logical_function &logfunc) {
    cout << endl << "Truth Table:" << endl;
    for( char c = 'A'; c != 'A' + logfunc.term_size(); ++c )
        cout << c;
    cout << " | f()"<< endl;
    for( int i = 0; i < logfunc.term_size() + 6; ++i )
        cout << ( (i == logfunc.term_size() + 1) ? '|' : '-');
    cout << endl;
    for( int i = 0; i < std::pow(2, logfunc.term_size()); ++i ) {
        logical_expr::logical_term::arg_type arg(logfunc.term_size(), i);
        cout << arg << " |  " << logfunc(arg) << endl;
        logical_expr::logical_term tmp(arg);
    }
}

int main(int argc, char **argv)
{
    int exit_code = EXIT_SUCCESS;
    try {
        cout << "Quine-McCluskey" << endl
             << "[*] Enter the formula to solve" << endl
             << "Input: " << flush;
        string line;
        getline(cin, line);
        auto token = logical_expr::function_parser<'^', true>(line).parse();
        logical_expr::logical_function function;
        for( string term : token.second )
            function += logical_expr::logical_term(term, token.first.size());
        quine_mccluskey::minimizer qm(function);
        logical_expr::logical_function stdspf = qm.get_std_spf();
        print_truth_table(stdspf);
        auto table = qm.make_min_table();
        cout << endl;
        int true_count = 0;
        for( auto set : table ) {
            cout << "true_count = " << true_count++ << endl;
            for( const logical_expr::logical_term term : set )
                cout << term << " ";
            cout << endl;
        }
    }
    catch( std::exception &e ) {
        cerr << "[-] Exception: " << e.what() << endl;
        exit_code = EXIT_FAILURE;
    }
    return exit_code;
}

