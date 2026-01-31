#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_stringFromJNI(JNIEnv *env, jobject activity)
{
    const std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
