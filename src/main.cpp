
#include <iostream>
#include <string>
#include <utility>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include "logical_expr.hpp"
#include "quine_mccluskey.hpp"

using namespace std;

template<typename Property>
void print_term_expr(const logical_expr::logical_term<Property> &term)
{
    for( int i = 0; i < term.size(); ++i ) {
        if( term[i] == false )  cout << "^";
        if( term[i] != logical_expr::dont_care )
            cout << static_cast<char>('A' + i);
    }
}

template<typename TermType>
void print_func_expr(const logical_expr::logical_function<TermType> &func)
{
    cout << "f = ";
    for( auto it = func.begin(); it != func.end(); ++it ) {
        print_term_expr(*it);
        if( it + 1 != func.end() )
            cout << " + ";
    }
    cout << endl;
}

template<typename TermType>
void print_truth_table(const logical_expr::logical_function<TermType> &f)
{
    cout << "Truth Table: ";
    print_func_expr(f);
    for( char c = 'A'; c != 'A' + f.term_size(); ++c )
        cout << c;
    cout << " | f()"<< endl;
    for( int i = 0; i < f.term_size() + 6; ++i )
        cout << ((i == f.term_size() + 1) ? '|' : '-');
    cout << endl;
    logical_expr::arg_generator<> generator(0, std::pow(2, f.term_size()), f.term_size());
    for( auto arg : generator )
        cout << arg << " |  " << f(arg) << endl;
}

int main(int argc, char **argv)
{
    int exit_code = EXIT_SUCCESS;
    try {
        cout << "Logical Function Simplifier (Quine-McCluskey)"   << endl
             << "[*] Enter a logical expression to be simplified" << endl
             << "    (ex. \"f(A, B, C) = A + BC + ^A^B + ABC\" )" << endl
             << "[*] Input: " << flush;
        string line;
        getline(cin, line);
        // Parse input logical expression and return tokenized
        constexpr char inverter = '^';
        auto token = logical_expr::function_parser<inverter, true>(line).parse();

        // Create a logical function using logical_term<term_make>
        typedef logical_expr::logical_term<logical_expr::term_mark> TermType;
        logical_expr::logical_function<TermType> function;
        for( string term : token.second )
            function += logical_expr::parse_logical_term<logical_expr::term_mark, inverter>(term, token.first.size());

        // Create a simplifier using Quine-McCluskey algorithm
        quine_mccluskey::simplifier qm(function);
        cout << endl << "Sum of products form:" << endl;
        print_truth_table(qm.get_std_spf());    // Print the function in sum of products form
        cout << endl << "Compressing ..." << endl;
        qm.compress_table();                    // Compress the compression table
        cout << endl << "Prime implicants: " << endl;
        for( auto term : qm.get_prime_implicants() ) {
            print_term_expr(term);
            cout << "  ";
        }   
        cout << endl << endl << "Result of simplifying:" << endl;
        for( auto func : qm.simplify() )        // Simplify and print its results
            print_func_expr(func);
    }
    catch( std::exception &e ) {
        cerr << endl << "[-] Exception: " << e.what() << endl;
        exit_code = EXIT_FAILURE;
    }
    return exit_code;
}

