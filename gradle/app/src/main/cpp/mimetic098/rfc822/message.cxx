#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"

#pragma GCC diagnostic ignored "-Wconversion"

/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: message.cxx,v 1.4 2008-10-07 11:06:27 tat Exp $
 ***************************************************************************/
#include <mimetic098/rfc822/message.h>
#include <mimetic098/strutils.h>
#include <mimetic098/utils.h>


namespace mimetic 
{

using namespace std;

ostream& operator<<(ostream& os, const Message& m)
{
    // header field
    Rfc822Header::const_iterator hbit, heit;
    hbit = m.header().begin();
    heit = m.header().end();
    for(; hbit != heit; ++hbit)
        os << *hbit;
    // empty line, header/body separator
    os << crlf;
    // body
    os << m.body();
    os.flush();
    return os;    
}

Rfc822Header& Message::header()
{
    return m_header;
}

const Rfc822Header& Message::header() const
{
    return m_header;
}

Rfc822Body& Message::body()
{    
    return m_body;    
}

const Rfc822Body& Message::body() const
{    
    return m_body;    
}

}
