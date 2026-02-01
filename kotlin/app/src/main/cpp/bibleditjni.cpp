/*
 Copyright (©) 2003-2026 Teus Benschop.

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

#include <jni.h>
#include <string>
#include "library/bibledit.h"


extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_StringFromJNI(JNIEnv* env, jobject activity)
{
    return env->NewStringUTF("Hello World From C++");
}


extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_GetVersionNumber(JNIEnv* env, jobject activity)
{
    return env->NewStringUTF(bibledit_get_version_number ());
}


extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_GetNetworkPort (JNIEnv* env, jobject activity)
{
    return env->NewStringUTF(bibledit_get_network_port ());
}


extern "C" JNIEXPORT void JNICALL
Java_org_bibledit_android_MainActivity_InitializeLibrary (JNIEnv* env, jobject activity, jstring package, jstring webroot)
{
    const char * native_package = env->GetStringUTFChars(package, 0);
    const char * native_webroot = env->GetStringUTFChars(webroot, 0);
    bibledit_initialize_library (native_package, native_webroot);
}


extern "C" JNIEXPORT void JNICALL
Java_org_bibledit_android_MainActivity_SetTouchEnabled (JNIEnv* env, jobject activity, jboolean enabled)
{
    // This fails to work on Android 6.
    bibledit_set_touch_enabled ((enabled == JNI_TRUE));
    // Set it always to true.
    bibledit_set_touch_enabled (true);
}


extern "C" JNIEXPORT void JNICALL
Java_org_bibledit_android_MainActivity_StartLibrary (JNIEnv* env, jobject activity)
{
    bibledit_start_library ();
}


extern "C" JNIEXPORT jboolean JNICALL
Java_org_bibledit_android_MainActivity_IsRunning (JNIEnv* env, jobject activity)
{
    return bibledit_is_running ();
}


extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_IsSynchronizing (JNIEnv* env, jobject activity)
{
    return env->NewStringUTF(bibledit_is_synchronizing ());
}


extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_GetExternalUrl (JNIEnv* env, jobject activity)
{
    return env->NewStringUTF(bibledit_get_external_url ());
}


extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_GetPagesToOpen (JNIEnv* env, jobject activity)
{
    return env->NewStringUTF(bibledit_get_pages_to_open ());
}


extern "C" JNIEXPORT void JNICALL
Java_org_bibledit_android_MainActivity_StopLibrary (JNIEnv* env, jobject activity)
{
    bibledit_stop_library ();
}


extern "C" JNIEXPORT void JNICALL
Java_org_bibledit_android_MainActivity_ShutdownLibrary (JNIEnv* env, jobject activity)
{
    bibledit_shutdown_library ();
}


extern "C" JNIEXPORT void JNICALL
Java_org_bibledit_android_MainActivity_Log (JNIEnv* env, jobject activity, jstring message)
{
    const char * native_message = env->GetStringUTFChars(message, 0);
    bibledit_log (native_message);
    env->ReleaseStringUTFChars(message, native_message);
}


extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_GetLastPage (JNIEnv* env, jobject activity)
{
    return env->NewStringUTF(bibledit_get_last_page ());
}


extern "C" JNIEXPORT void JNICALL
Java_org_bibledit_android_MainActivity_RunOnChromeOS (JNIEnv* env, jobject activity)
{
  bibledit_run_on_chrome_os ();
}


extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_DisableSelectionPopupChromeOS (JNIEnv* env, jobject activity)
{
  return env->NewStringUTF(bibledit_disable_selection_popup_chrome_os ());
}

