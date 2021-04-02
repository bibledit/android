/*
Copyright (©) 2003-2021 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include <unittests/date.h>
#include <unittests/utilities.h>
#include <filter/date.h>
#include <filter/string.h>


void test_date ()
{
  trace_unit_tests (__func__);

  // Test the date and time related functions.
  {
    int month = filter_date_numerical_month (filter_date_seconds_since_epoch ());
    if ((month < 1) || (month > 12)) evaluate (__LINE__, __func__, "current month", convert_to_string (month));
    int year = filter_date_numerical_year (filter_date_seconds_since_epoch ());
    if ((year < 2014) || (year > 2050)) evaluate (__LINE__, __func__, "current year", convert_to_string (year));
    struct timeval tv;
    gettimeofday (&tv, NULL);
    int reference_second = tv.tv_sec;
    int actual_second = filter_date_seconds_since_epoch ();
    if (abs (actual_second - reference_second) > 1) evaluate (__LINE__, __func__, reference_second, actual_second);
    int usecs = filter_date_numerical_microseconds ();
    if ((usecs < 0) || (usecs > 1000000)) evaluate (__LINE__, __func__, "0-1000000", convert_to_string (usecs));
  }
  
  // First business day of month.
  {
    // Sunday the 1st.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (1, 0));
    // Monday the 1st.
    evaluate (__LINE__, __func__, true, filter_date_is_first_business_day_of_month (1, 1));
    // Tuesday the 1st.
    evaluate (__LINE__, __func__, true, filter_date_is_first_business_day_of_month (1, 2));
    // Wednesday the 1st.
    evaluate (__LINE__, __func__, true, filter_date_is_first_business_day_of_month (1, 3));
    // Thirsday the 1st.
    evaluate (__LINE__, __func__, true, filter_date_is_first_business_day_of_month (1, 4));
    // Friday the 1st.
    evaluate (__LINE__, __func__, true, filter_date_is_first_business_day_of_month (1, 5));
    // Saturday the 1st.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (1, 6));
    // Sunday the 2nd.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (2, 0));
    // Monday the 2nd.
    evaluate (__LINE__, __func__, true, filter_date_is_first_business_day_of_month (2, 1));
    // Tuesday the 2nd.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (2, 2));
    // Sunday the 3nd.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (3, 0));
    // Monday the 3nd.
    evaluate (__LINE__, __func__, true, filter_date_is_first_business_day_of_month (3, 1));
    // Tuesday the 3nd.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (3, 2));
    // Sunday the 4nd.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (4, 0));
    // Monday the 4nd.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (4, 1));
    // Tuesday the 4nd.
    evaluate (__LINE__, __func__, false, filter_date_is_first_business_day_of_month (4, 2));
  }

  // Last business day of month.
  {
    evaluate (__LINE__, __func__, 30, filter_date_get_last_business_day_of_month (2013, 9));
    evaluate (__LINE__, __func__, 31, filter_date_get_last_business_day_of_month (2013, 10));
    evaluate (__LINE__, __func__, 29, filter_date_get_last_business_day_of_month (2013, 11));
    evaluate (__LINE__, __func__, 31, filter_date_get_last_business_day_of_month (2013, 12));
    evaluate (__LINE__, __func__, 31, filter_date_get_last_business_day_of_month (2014, 1));
    evaluate (__LINE__, __func__, 28, filter_date_get_last_business_day_of_month (2014, 2));
    evaluate (__LINE__, __func__, 31, filter_date_get_last_business_day_of_month (2014, 3));
    evaluate (__LINE__, __func__, 30, filter_date_get_last_business_day_of_month (2014, 4));
    evaluate (__LINE__, __func__, 30, filter_date_get_last_business_day_of_month (2014, 5));
    evaluate (__LINE__, __func__, 30, filter_date_get_last_business_day_of_month (2014, 6));
  }

  // Is business day.
  {
    evaluate (__LINE__, __func__, false, filter_date_is_business_day (2013, 9, 1));
    evaluate (__LINE__, __func__, true, filter_date_is_business_day (2013, 9, 2));
    evaluate (__LINE__, __func__, true, filter_date_is_business_day (2013, 9, 3));
    evaluate (__LINE__, __func__, true, filter_date_is_business_day (2013, 9, 4));
    evaluate (__LINE__, __func__, true, filter_date_is_business_day (2013, 9, 5));
    evaluate (__LINE__, __func__, true, filter_date_is_business_day (2013, 9, 6));
    evaluate (__LINE__, __func__, false, filter_date_is_business_day (2013, 9, 7));
    evaluate (__LINE__, __func__, false, filter_date_is_business_day (2013, 9, 8));
    evaluate (__LINE__, __func__, true, filter_date_is_business_day (2013, 9, 30));
    evaluate (__LINE__, __func__, false, filter_date_is_business_day (2015, 3, 1));
    evaluate (__LINE__, __func__, false, filter_date_is_business_day (2015, 2, 32));
    
  }

  // Seconds since Unix epoch.
  {
    int year, month, day, seconds;
    
    year = 2011;
    month = 2;
    day = 5;
    seconds = filter_date_seconds_since_epoch (year, month, day);
    evaluate (__LINE__, __func__, year, filter_date_numerical_year (seconds));
    evaluate (__LINE__, __func__, month, filter_date_numerical_month (seconds));
    evaluate (__LINE__, __func__, day, filter_date_numerical_month_day (seconds));
    
    year = 2015;
    month = 3;
    day = 15;
    seconds = filter_date_seconds_since_epoch (year, month, day);
    evaluate (__LINE__, __func__, year, filter_date_numerical_year (seconds));
    evaluate (__LINE__, __func__, month, filter_date_numerical_month (seconds));
    evaluate (__LINE__, __func__, day, filter_date_numerical_month_day (seconds));
    
    year = 2030;
    month = 12;
    day = 31;
    seconds = filter_date_seconds_since_epoch (year, month, day);
    evaluate (__LINE__, __func__, year, filter_date_numerical_year (seconds));
    evaluate (__LINE__, __func__, month, filter_date_numerical_month (seconds));
    evaluate (__LINE__, __func__, day, filter_date_numerical_month_day (seconds));
  }

}
