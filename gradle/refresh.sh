#!/bin/bash


# Run this script from the directory where it is located.
# Example: ./refresh.sh


# Export environment variables to find the Android SDK and NDK tools. Todo go out.
# export ANDROID_HOME=~/scr/android-sdk-macosx
# export PATH=$PATH:~/scr/android-sdk-macosx/platform-tools:~/scr/android-sdk-macosx/tools:~/scr/android-ndk-r10e


echo Define the assets
ASSETSFOLDER=app/src/main/assets
EXTERNALFOLDER=$ASSETSFOLDER/external


echo Put all the code of the Bibledit kernel into the following folder:
echo $EXTERNALFOLDER
echo This is in preparation for subsequent steps.
rsync -a --delete --exclude .git ../../cloud/ $EXTERNALFOLDER/
if [ $? -ne 0 ]; then exit; fi


echo Clean the code up a bit by removing a couple of things.
pushd $EXTERNALFOLDER
rm -f *.gz
popd


echo Build several databases and other data for inclusion with the Android package.
echo The reason for this is that building them on Android takes a lot of time during the setup phase.
echo To include pre-built data, that will speed up the setup phase of Bibledit on Android.
echo This gives a better user experience.
echo At the end, it removes the journal entries that were logged in the process.
pushd $EXTERNALFOLDER
./configure
make --jobs=4
if [ $? -ne 0 ]; then exit; fi
./generate . locale
if [ $? -ne 0 ]; then exit; fi
./generate . mappings
if [ $? -ne 0 ]; then exit; fi
./generate . versifications
if [ $? -ne 0 ]; then exit; fi
popd


echo Clean the Bibledit kernel source code.
pushd $EXTERNALFOLDER
make distclean
if [ $? -ne 0 ]; then exit; fi
popd



CPPFOLDER=app/src/main/cpp
echo Synchronize the Bibledit kernel source code to the cpp folder at $CPPFOLDER.
rsync -av --delete --exclude bibleditjni.cpp --exclude CMakeLists.txt --exclude native.cpp --exclude stub.cpp --exclude stub.h $EXTERNALFOLDER/ $CPPFOLDER/
if [ $? -ne 0 ]; then exit; fi


echo Configure the code in the $CPPFOLDER folder for Android.
pushd $CPPFOLDER
./configure --enable-android
popd


echo Cleaning files out from the assets and the cpp folders.
function rm_rf_assets_cpp
{
  rm -rf $EXTERNALFOLDER/$1
  rm -rf $CPPFOLDER/$1
}
rm_rf_assets_cpp bibledit
rm_rf_assets_cpp autom4te.cache
rm_rf_assets_cpp dev
rm_rf_assets_cpp *.a
rm_rf_assets_cpp *.tar
rm_rf_assets_cpp *.tar.gz
rm_rf_assets_cpp reconfigure
rm_rf_assets_cpp server
rm_rf_assets_cpp unittest
rm_rf_assets_cpp generate
rm_rf_assets_cpp valgrind
rm_rf_assets_cpp cloud.xcodeproj
rm_rf_assets_cpp executable
rm_rf_assets_cpp aclocal.m4
rm_rf_assets_cpp AUTHORS
rm_rf_assets_cpp ChangeLog
rm_rf_assets_cpp compile
rm_rf_assets_cpp config.guess
rm_rf_assets_cpp config.h.in
rm_rf_assets_cpp config.log
rm_rf_assets_cpp config.status
rm_rf_assets_cpp config.sub
rm_rf_assets_cpp configure
rm_rf_assets_cpp configure.ac
rm_rf_assets_cpp COPYING
rm_rf_assets_cpp depcomp
rm_rf_assets_cpp DEVELOP
rm_rf_assets_cpp INSTALL
rm_rf_assets_cpp install-sh
rm_rf_assets_cpp Makefile
rm_rf_assets_cpp Makefile.in
rm_rf_assets_cpp missing
rm_rf_assets_cpp NEWS
rm_rf_assets_cpp README
rm_rf_assets_cpp stamp-h1
rm_rf_assets_cpp sources/hebrewlexicon
rm_rf_assets_cpp sources/morphgnt
rm_rf_assets_cpp sources/morphhb
rm_rf_assets_cpp sources/sblgnt
rm_rf_assets_cpp sources/oshb.xml.gz
rm_rf_assets_cpp unittests
rm_rf_assets_cpp config/local.server.key
rm -rf $CPPFOLDER/databases
find $EXTERNALFOLDER -name "*.h" -delete
find $EXTERNALFOLDER -name "*.cpp" -delete
find $EXTERNALFOLDER -name "*.c" -delete
find $EXTERNALFOLDER -name ".deps" -exec rm -r "{}" \; > /dev/null 2>&1
find $CPPFOLDER -name ".deps" -exec rm -r "{}" \; > /dev/null 2>&1
find $EXTERNALFOLDER -name ".dirstamp" -delete
find $CPPFOLDER -name ".dirstamp" -delete


# Android does not provide 'stoi' in C++.
# sed -i.bak '/HAVE_STOI/d' jni/config.h
# No libsword.
# sed -i.bak '/HAVE_SWORD/d' jni/config.h
# No file-upload possible from web view.
# sed -i.bak '/CONFIG_ENABLE_FILE_UPLOAD/d' jni/config/config.h
# Android does not need BSD memory profiling calls.
# sed -i.bak '/HAVE_MACH_MACH/d' jni/config.h
# Cleanup
# rm jni/config.h.bak
# rm jni/config/config.h.bak


# The following command saves all source files from Makefile.am to file.
# It uses several steps to obtain the result:
# * Obtain source files between the correct patterns.
# * Remove first line.
# * Remove last line.
# * Remove tabs.
# * Remove new lines.
# * Remove backslashes.
# sed -n "/libbibledit_a_SOURCES/,/bin_PROGRAMS/p" jni/Makefile.am | tail -n +2 | sed '$d' | strings | tr -d '\n' | sed 's/\\//g' > jni/sources.txt


# Create Android.mk Makefile from Android.am.
# sed "s|SOURCEFILES|$(cat jni/sources.txt)|" jni/Android.am > jni/Android.mk
# rm jni/sources.txt


