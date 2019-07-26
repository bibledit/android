#include <jni.h>
#include <string>

using namespace std;

extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_stringFromJNI (JNIEnv *env, jobject obj)
{
    string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
