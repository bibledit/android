#ifndef BIBLEDIT_H
#define BIBLEDIT_H
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

const char * bibledit_get_version_number ();
const char * bibledit_get_network_port ();
void bibledit_initialize_library (const char * package, const char * webroot);
void bibledit_set_touch_enabled (bool enabled);
void bibledit_start_library ();
const char * bibledit_get_last_page ();
bool bibledit_is_running ();
const char * bibledit_is_synchronizing ();
const char * bibledit_get_external_url ();
const char * bibledit_get_pages_to_open ();
void bibledit_stop_library ();
void bibledit_shutdown_library ();
void bibledit_log (const char * message);
void bibledit_run_on_chrome_os ();
const char * bibledit_disable_selection_popup_chrome_os ();

#ifdef __cplusplus
}
#endif


#endif
