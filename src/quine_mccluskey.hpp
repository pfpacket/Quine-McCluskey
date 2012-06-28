#ifndef QUINE_MCCLUSKEY_HPP
#define QUINE_MCCLUSKEY_HPP


#include <iostream>
#include <set>
#include <algorithm>
#include <stdexcept>
#include "logical_expr.hpp"


namespace quine_mccluskey {

using namespace std;
using namespace logical_expr;


class minimizer {
public:
    typedef vector<logical_term> set_type;
    typedef vector<set_type> table_type;

    minimizer() {}
    explicit minimizer(const logical_function &function) : func_(function) {}
    ~minimizer() {}

    // Return standard sum of products form
    const logical_function& get_std_spf() { return get_std_spf(func_); }
    const logical_function& get_std_spf(const logical_function &func) {
        stdspf_.clear();
        for( int i = 0; i < std::pow(2, func.term_size()); ++i ) {
            logical_term::arg_type arg(func.term_size(), i);
            if( func(arg) )
                stdspf_ += logical_term(arg);
        }
        return stdspf_;
    }

    const table_type& make_min_table() {
        table_.resize(func_.term_size() + 1, set_type());
        for( auto term : stdspf_ )
            table_[term.num_of_value(true)].push_back(term);
        return table_;
    }

    void minimize() {
        for( int i = 0; i+1 < table_.size(); ++i ) {
            for( int j = 0; j < table_[i].size(); ++j ) {
                for( int k = 0; k < table_[i+1].size(); ++k ) {
                    try {
                        auto term = onebit_minimize(table_[i][j], table_[i+1][k]);
                        cout << "MINIMIZE(" << table_[i][j] << ", " << table_[i+1][k] << ") = " << term << endl;
                    } catch( std::exception &e ) {}
                }
            }
        }
    }

private:
    logical_function func_, stdspf_;
    table_type table_;
};



}   // namespace quine_mccluskey


#endif  // QUINE_MCCLUSKEY_HPP
