#!/bin/bash


echo Run this script from the directory where it is located: $ ./refresh.sh


echo Exit script on any error
set -e


echo Define the assets
ASSETS_FOLDER=app/src/main/assets
EXTERNAL_FOLDER=$ASSETS_FOLDER/external


echo Put all the code of the Bibledit kernel into the following folder:
echo $EXTERNAL_FOLDER
echo This is in preparation for subsequent steps

pushd ../../cloud
cmake -B build
rm build/*.gz
cmake --build build --target dist
popd
rm -rf "${EXTERNAL_FOLDER:?}"/*
tar xf ../../cloud/build/bibledit-*.tar.gz -C "$EXTERNAL_FOLDER" --strip-components=1

echo Build several databases and other data for inclusion with the Android package
echo Reason: Building them on Android takes a lot of time during the setup phase
echo Including pre-built data speeds up the setup phase of Bibledit on Android
echo This gives a better user experience
echo At the end it removes the journal entries that were logged in the process
pushd "$EXTERNAL_FOLDER"
cmake -B build
cmake --build build --target generate -j8
./build/generate . locale
./build/generate . mappings
./build/generate . versifications
popd

echo Configure for Android
pushd "$EXTERNAL_FOLDER"
cmake -B build -DHAVE_ANDROID=ON
popd

echo Clean the Bibledit kernel source code
pushd "$EXTERNAL_FOLDER"
rm -rf build
popd



CPP_FOLDER=app/src/main/cpp
echo Synchronize the Bibledit kernel source code to the cpp folder at "$CPP_FOLDER"
rsync -av --delete --exclude bibleditjni.cpp --exclude CMakeLists.txt --exclude native.cpp --exclude stub.cpp --exclude stub.h $EXTERNAL_FOLDER/ $CPP_FOLDER/


echo Configure the code in the "$CPP_FOLDER" folder for Android
pushd "$CPP_FOLDER"
rm -rf build
cmake -B build -DHAVE_ANDROID=ON
popd


echo Cleaning files out from the assets and the cpp folders.
function rm_rf_assets_cpp
{
  rm -rf "${EXTERNAL_FOLDER:?}"/"$1"
  rm -rf "${CPP_FOLDER:?}"/"$1"
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
rm_rf_assets_cpp config/local.privkey.pem
rm_rf_assets_cpp .git*
rm_rf_assets_cpp .github
rm_rf_assets_cpp xcode*
rm_rf_assets_cpp cloud-macos.entitlements
rm_rf_assets_cpp index.html
rm_rf_assets_cpp Makefile.am
rm_rf_assets_cpp build
rm -rf "$CPP_FOLDER"/databases
find "$EXTERNAL_FOLDER" -name "*.h" -delete
find "$EXTERNAL_FOLDER" -name "*.cpp" -delete
find "$EXTERNAL_FOLDER" -name "*.c" -delete
find "$EXTERNAL_FOLDER" -name ".deps" -print0 | xargs -0 rm -rf
find "$CPP_FOLDER" -name ".deps" -print0 | xargs -0 rm -rf
find "$EXTERNAL_FOLDER" -name ".dirstamp" -delete
find "$CPP_FOLDER" -name ".dirstamp" -delete
find "$CPP_FOLDER" -not -name "*.h" -not -name "*.c" -not -name "*.cpp" -not -name "*.hpp" -not -name CMakeLists.txt -delete
rm -rf "$ASSETS_FOLDER"/external/build


echo Adapting native source to Android
pushd $CPP_FOLDER

echo No libsword
sed -i. '/HAVE_SWORD/d' config.h

echo No file-upload possible from web view
sed -i. '/CONFIG_ENABLE_FILE_UPLOAD/d' config/config.h

echo Android does not need BSD memory profiling calls
sed -i. '/HAVE_MACH_MACH/d' config.h

echo No libicu
sed -i. '/HAVE_ICU/d' config.h

echo No libutf8proc
sed -i. '/HAVE_UTF8PROC/d' config.h

echo No pugixml library
sed -i. '/HAVE_PUGIXML/d' config.h

echo No execinfo.h
sed -i. '/HAVE_EXECINFO/d' config.h

popd


echo Create assets index.
pushd $EXTERNAL_FOLDER
find . -type f | cut -c 3- > ../asset.external
popd


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


echo The script completed successfully
