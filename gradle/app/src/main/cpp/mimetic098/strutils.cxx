/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: strutils.cxx,v 1.3 2008-10-07 11:06:26 tat Exp $
 ***************************************************************************/
#include <mimetic098/strutils.h>

namespace mimetic
{
using namespace std;

const string nullstring;


string canonical(const string& s, bool no_ws)
{
    if(s.empty())
        return s;
    string input = s;
    // removes leading spaces
    int idx = 0;
    while(input[idx] == ' ')
        idx++;
    if(idx)
        input.erase(0, idx);
    // removes trailing spaces
    idx = (int)input.length() - 1;
    while(input[idx] == ' ')
        idx--;
    int idx1 = idx;
    int idx2 = ++idx;
    input.erase(idx1, input.length() - idx2);
    // removes rfc822 comments and non-required spaces
    bool in_dquote = false, has_brack = false;
    int in_par = 0, in_brack = 0, par_last = 0;
    for(int t = (int)input.length() - 1; t >= 0; --t)
    {
        if(input[t] == '"') {
            in_dquote = !in_dquote;
        } else if(in_dquote) {
            continue;
        } else if(input[t] == '<') {
            in_brack--;
        } else if(input[t] == '>') {
            has_brack = true;
            in_brack++;
        } else if(input[t] == ')') {
            in_par++;
            if(in_par == 1)
                par_last = t;
        } else if(input[t] == '(') {
            in_par--;
            if(in_par == 0)
            {
                input.erase(t, 1 + par_last - t);
                // comments will be replaced with a space in
                // !no_ws
                if(!no_ws) 
                    input.insert(t, " ");
            }
        } else if(no_ws && input[t] == ' ' && !in_par && !has_brack) {
            input.erase(t, 1);
        }
    }
    return input;
}

}
