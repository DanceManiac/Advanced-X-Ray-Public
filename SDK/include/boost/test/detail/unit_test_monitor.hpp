//  (C) Copyright Gennadiy Rozental & Ullrich Koethe 2001.
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied warranty,
//  and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version including documentation.
//
//  File        : $RCSfile: unit_test_monitor.hpp,v $
//
//  Version     : $Id: unit_test_monitor.hpp,v 1.8 2002/12/08 17:36:32 rogeeff Exp $
//
//  Description : defines specific version of execution monitor used to run unit 
// test cases. Translates executioin exception into error level
// ***************************************************************************

#ifndef BOOST_UNIT_TEST_MONITOR_HPP
#define BOOST_UNIT_TEST_MONITOR_HPP

#include <boost/test/execution_monitor.hpp>

namespace boost {

namespace unit_test_framework {

class test_case;

namespace detail {

// ************************************************************************** //
// **************               unit_test_monitor              ************** //
// ************************************************************************** //

class unit_test_monitor : public execution_monitor {
    typedef void (test_case::*function_to_monitor)();
public:
    enum error_level { 
        test_fail               =  1,
        test_ok                 =  0,
        constructor_error       = -1, 
        unexpected_exception    = -2, 
        os_exception            = -3, 
        os_timeout              = -4, 
        fatal_error             = -5,  // includes both system and user
        destructor_error        = -6
    };
    static bool         is_critical_error( error_level e_ ) { return e_ <= fatal_error; }

    // Constructor
    unit_test_monitor( test_case& target_test_case_, function_to_monitor f_ )
    : m_test_case_function( f_ ), m_test_case( target_test_case_ ) {}

    // management method; same for all monitors
    static void         catch_system_errors( bool yes_no = true ) { s_catch_system_errors = yes_no; }

    // monitor method
    error_level         execute_and_translate( int timeout_ );

    // execution monitor hook implementation
    virtual int         function();

private:
    // Data members
    function_to_monitor m_test_case_function;
    test_case&          m_test_case;

    static bool         s_catch_system_errors;
}; // unit_test_monitor

} // namespace detail

} // namespace unit_test_framework

} // namespace boost

// ***************************************************************************
//  Revision History :
//  
//  $Log: unit_test_monitor.hpp,v $
//  Revision 1.8  2002/12/08 17:36:32  rogeeff
//  catch system errors switch introduced
//
//  Revision 1.7  2002/11/02 19:31:05  rogeeff
//  merged into the main trank
//

// ***************************************************************************

#endif // BOOST_UNIT_TEST_MONITOR_HPP
