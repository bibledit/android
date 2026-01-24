#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_org_bibledit_android_MainActivity_stringFromJNI(JNIEnv* env, jobject activity)
{
    static int counter{0};
    const std::string message = std::to_string(++counter) + " hello from C++";
    return env->NewStringUTF(message.c_str());
}