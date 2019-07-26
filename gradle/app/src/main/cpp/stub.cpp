#include "stub.h"


const char * bibledit_get_version_number ()
{
    return "1.0";
}

const char * bibledit_get_network_port ()
{
    return "8080";
}

void bibledit_initialize_library (const char * package, const char * webroot)
{
    (void) package;
    (void) webroot;
}

void bibledit_set_touch_enabled (bool enabled)
{
    (void) enabled;
}

void bibledit_start_library ()
{

}

const char * bibledit_get_last_page ()
{
    return "";
}

bool bibledit_is_running ()
{
    return true;
}

const char * bibledit_is_synchronizing ()
{
    return "";
}

const char * bibledit_get_external_url ()
{
    return "";
}

const char * bibledit_get_pages_to_open ()
{
    return "";
}

void bibledit_stop_library ()
{

}

void bibledit_shutdown_library ()
{

}

void bibledit_log (const char * message)
{
    (void) message;
}

void bibledit_run_on_chrome_os ()
{

}

const char * bibledit_disable_selection_popup_chrome_os ()
{
    return "";
}
