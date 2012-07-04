
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

template<typename TermType>
void print_truth_table(const logical_expr::logical_function<TermType> &f) {
    cout << endl << "Truth Table:" << endl;
    for( char c = 'A'; c != 'A' + f.term_size(); ++c )
        cout << c;
    cout << " | f()"<< endl;
    for( int i = 0; i < f.term_size() + 6; ++i )
        cout << ((i == f.term_size() + 1) ? '|' : '-');
    cout << endl;
    logical_expr::arg_generator<> gen(0, std::pow(2, f.term_size()), f.term_size());
    for( auto it = gen.begin(); it != gen.end(); ++it )
        cout << *it << " |  " << f(*it) << endl;
}

void print_bit_table(const quine_mccluskey::minimizer::table_type &table)
{
    int true_count = 0;
    for( auto set : table ) {
        cout << "true_count = " << true_count++ << endl;
        for( auto term : set )
            cout << term << " ";
        cout << endl;
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
        typedef logical_expr::logical_term<logical_expr::term_mark> TermType;
        logical_expr::logical_function<TermType> function;
        for( string term : token.second )
            function += logical_expr::parse_logical_term<logical_expr::term_mark>(term, token.first.size());
        quine_mccluskey::minimizer qm(function);
        auto stdspf = qm.get_std_spf();
        print_truth_table(stdspf);
        cout << endl;
        auto table = qm.make_min_table();
        print_bit_table(table);
        cout << endl << "minimized: " << endl;
        do {
            cout << "Current level = " << qm.get_current_level() << endl;
        } while( qm.find_prime_implicants() );
    }
    catch( std::exception &e ) {
        cerr << "[-] Exception: " << e.what() << endl;
        exit_code = EXIT_FAILURE;
    }
    return exit_code;
}

