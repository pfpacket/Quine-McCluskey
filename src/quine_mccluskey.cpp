
#include <iostream>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <cmath>
#include "logical_expr.hpp"
#include "quine_mccluskey.hpp"

using namespace std;
using namespace logical_expr;

namespace quine_mccluskey {


typedef simplifier::property_type property_type;
typedef simplifier::term_type term_type;
typedef simplifier::set_type set_type;
typedef simplifier::table_type table_type;

// Make standard sum of products form
const logical_function<term_type>& simplifier::make_std_spf() {
    stdspf_.clear();
    arg_generator<> generator(0, std::pow(2, func_.term_size()), func_.term_size());
    for( auto arg : generator )
        if( func_(arg) )
            stdspf_ += logical_term<term_mark>(arg);
    return stdspf_;
}

const table_type& simplifier::make_min_table() {
    table_[0].resize(func_.term_size() + 1, set_type());
    for( auto term : stdspf_ )
        table_[0][term.num_of_value(true)].push_back(term);
    return table_[0];
}

void simplifier::compress_table(bool printable) {
    for( ;; ) {
        if( printable )
            cout << get_current_level() + 1 << "-level compression:" << endl;
        if( !compress_impl(printable) ) break;
    }
    for( auto table : table_  )
        for( auto set : table )
            for( const logical_term<term_mark> &term : set )
                if( !property_get(term) )
                    prime_imp.push_back(term);
    make_unique(prime_imp);
}

const vector<logical_function<term_type>>& simplifier::simplify() {
    vector<int> next_index(prime_imp.size());
    std::iota(next_index.begin(), next_index.end(), 0);
    // bool: wether simplifying finished  int: loop number that simplifying took
    std::pair<bool, int> end_flags = { false, 0 };
    for( int i = 1; i <= prime_imp.size(); ++i ) {
        if( end_flags.first && end_flags.second < i )
            break;
        do {
            logical_function<term_type> func;
            auto log_func_comp 
                = [&](const logical_function<term_type> &f){ return f.is_same(func); };
            for( int j = 0; j < i; ++j )
                func += prime_imp[next_index[j]];
            if( func == stdspf_ &&
                std::find_if(simplified_.begin(), simplified_.end(), log_func_comp)
                   == simplified_.end() ) {
                simplified_.push_back(func);
                end_flags = { true, i };
            }
        } while( next_permutation(next_index.begin(), next_index.end()) );
    }
    return simplified_;
}

void simplifier::add_table(const table_type& table) {
    table_.push_back(table);
}

void simplifier::clear_table() {
    for( int i = 0; i < table_.size(); ++i )
        table_[i].clear();
}

template<typename T>
void simplifier::make_unique(vector<T> &vec) {
    for( auto it = vec.begin(); it != vec.end(); ++it ) {
        auto rm_it = remove(it + 1, vec.end(), *it);
        vec.erase(rm_it, vec.end());
    }
}

// Try to find prime implicants
// Return true while trying to find them
// Return false if it finished
bool simplifier::compress_impl(bool printable) {
    table_type next_table;
    next_table.resize(func_.term_size(), set_type());
    int count = 0;
    for( int i = 0; i+1 < table_[min_level_].size(); ++i ) {
        for( int j = 0; j < table_[min_level_][i].size(); ++j ) {
            for( int k = 0; k < table_[min_level_][i+1].size(); ++k ) {
                try {
                    // Throw an exception if could not minimize
                    auto term = onebit_minimize(table_[min_level_][i][j], table_[min_level_][i+1][k], false);
                    if( printable )
                        cout << "COMPRESS(" << table_[min_level_][i][j] << ", " << table_[min_level_][i+1][k] << ") = " << term << endl;
                    if( std::find_if(next_table[term.num_of_value(true)].begin(), next_table[term.num_of_value(true)].end(), 
                                [&](const term_type &t){ return t.is_same(term); }) == next_table[term.num_of_value(true)].end())
                        next_table[term.num_of_value(true)].push_back(term);
                    ++count;
                    // Mark the used term for minimization
                    property_set(table_[min_level_][i][j], true);
                    property_set(table_[min_level_][i+1][k], true);
                } catch( std::exception &e ) {}
            }
        }
    }
    if( count ) {
        ++min_level_;
        add_table(next_table);
    }
    return (count ? true : false);
}


}   // namespace quine_mccluskey

