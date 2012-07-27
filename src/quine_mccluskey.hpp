#ifndef QUINE_MCCLUSKEY_HPP
#define QUINE_MCCLUSKEY_HPP


#include <iostream>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include "logical_expr.hpp"

//
// TODO
// * create quine_mccluskey.cpp and 
//      disunite the interface and implementation parts
//
//
namespace quine_mccluskey {

using namespace std;
using namespace logical_expr;

//
// logical function simplifier
//
// How to simplify:
//  [*] In the case which simplifier is constructed with logical_function
//      In this case, constructor will prepare to simplify a function
//      1. compress_table()     // Compress the compression table
//      2. simplify()           // simplify the function and get simplified
//  [*] In the case which simplifier is default-constructed
//      1. set_function()       // set a target function
//      2. make_std_spf()       // make a standard sum of products form
//      3. make_min_table()     // create a compression table
//      4. same as the case above
//
class simplifier {
public:
    typedef term_mark property_type;
    typedef logical_term<property_type> term_type;
    typedef vector<term_type> set_type;
    typedef vector<set_type> table_type;

    simplifier() : min_level_(0) 
        { add_table(table_type()); make_min_table(); }
    simplifier(const logical_function<term_type> &function) : min_level_(0), func_(function)
        { add_table(table_type()); make_std_spf(); make_min_table(); }
    ~simplifier() {}

    void set_function(const logical_function<term_type> &func) { func_ = func; }
    int get_current_level() const { return min_level_; }
    const logical_function<term_type>& get_std_spf() const { return stdspf_; }
    const set_type& get_prime_implicants() const { return prime_imp; }

    // Make standard sum of products form
    const logical_function<term_type>& make_std_spf() {
        stdspf_.clear();
        arg_generator<> generator(0, std::pow(2, func_.term_size()), func_.term_size());
        for( auto arg : generator )
            if( func_(arg) )
                stdspf_ += logical_term<term_mark>(arg);
        return stdspf_;
    }

    const table_type& make_min_table() {
        table_[0].resize(func_.term_size() + 1, set_type());
        for( auto term : stdspf_ )
            table_[0][term.num_of_value(true)].push_back(term);
        return table_[0];
    }
    
    void compress_table(bool printable = false) {
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

    const vector<logical_function<term_type>>& simplify() {
        vector<int> next_index;
        std::pair<bool, int> end_flags = { false, 0 };
        for( int i = 0; i < prime_imp.size(); ++i )
            next_index.push_back(i);
        for( int i = 1; i <= prime_imp.size(); ++i ) {
            if( end_flags.first && end_flags.second < i )
                break;
            do {
                logical_function<term_type> func;
                for( int j = 0; j < i; ++j )
                    func += prime_imp[next_index[j]];
                if( func == stdspf_ 
                    && std::find_if(simplified_.begin(), simplified_.end(), 
                        [&](const logical_function<term_type> &f){ return f.is_same(func); }) == simplified_.end() ) {
                    simplified_.push_back(func);
                    end_flags = { true, i };
                }
            } while( next_permutation(next_index.begin(), next_index.end()) );
        }
        return simplified_;
    }
    
private:
    void add_table(const table_type& table) { table_.push_back(table); }
    void clear_table() {
        for( int i = 0; i < table_.size(); ++i )
            table_[i].clear();
    }

    template<typename T>
    static void make_unique(vector<T> &vec) {
        for( auto it = vec.begin(); it != vec.end(); ++it ) {
            auto rm_it = remove(it + 1, vec.end(), *it);
            vec.erase(rm_it, vec.end());
        }
    }

    // Try to find prime implicants
    // Return true while trying to find them
    // Return false if it finished
    bool compress_impl(bool printable = false) {
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

    int min_level_;
    logical_function<term_type> func_, stdspf_;
    vector<logical_function<term_type>> simplified_;
    vector<table_type> table_;
    set_type prime_imp;
};



}   // namespace quine_mccluskey


#endif  // QUINE_MCCLUSKEY_HPP
