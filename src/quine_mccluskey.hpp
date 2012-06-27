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

    typedef vector<vector<logical_term>> table_type;

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
        table_.resize(func_.term_size() + 1, vector<logical_term>());
        for( auto term : stdspf_ )
            table_[term.num_of_value(true)].push_back(term);
        return table_;
    }

private:
    logical_function func_, stdspf_;
    table_type table_;
};



}   // namespace quine_mccluskey


#endif  // QUINE_MCCLUSKEY_HPP
