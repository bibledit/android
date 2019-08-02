# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        bibledit

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native.cpp
        bibleditjni.cpp
        stub.cpp
        # Start Bibledit kernel.
        miniz/miniz.c
        microtar/microtar.c
        library/locks.c
        mbedtls/aes.c
        mbedtls/aesni.c
        mbedtls/arc4.c
        mbedtls/asn1parse.c
        mbedtls/asn1write.c
        mbedtls/base64.c
        mbedtls/bignum.c
        mbedtls/blowfish.c
        mbedtls/camellia.c
        mbedtls/ccm.c
        mbedtls/certs.c
        mbedtls/chacha20.c
        mbedtls/chachapoly.c
        mbedtls/cipher.c
        mbedtls/cipher_wrap.c
        mbedtls/ctr_drbg.c
        mbedtls/debug.c
        mbedtls/des.c
        mbedtls/dhm.c
        mbedtls/ecdh.c
        mbedtls/ecdsa.c
        mbedtls/ecjpake.c
        mbedtls/ecp.c
        mbedtls/ecp_curves.c
        mbedtls/entropy.c
        mbedtls/entropy_poll.c
        mbedtls/error.c
        mbedtls/gcm.c
        mbedtls/havege.c
        mbedtls/hmac_drbg.c
        mbedtls/md.c
        mbedtls/md2.c
        mbedtls/md4.c
        mbedtls/md5.c
        mbedtls/md_wrap.c
        mbedtls/memory_buffer_alloc.c
        mbedtls/net_sockets.c
        mbedtls/oid.c
        mbedtls/padlock.c
        mbedtls/pem.c
        mbedtls/pk.c
        mbedtls/pk_wrap.c
        mbedtls/pkcs11.c
        mbedtls/pkcs12.c
        mbedtls/pkcs5.c
        mbedtls/pkparse.c
        mbedtls/pkwrite.c
        mbedtls/platform_util.c
        mbedtls/platform.c
        mbedtls/poly1305.c
        mbedtls/ripemd160.c
        mbedtls/rsa.c
        mbedtls/rsa_internal.c
        mbedtls/sha1.c
        mbedtls/sha256.c
        mbedtls/sha512.c
        mbedtls/ssl_cache.c
        mbedtls/ssl_ciphersuites.c
        mbedtls/ssl_cli.c
        mbedtls/ssl_cookie.c
        mbedtls/ssl_srv.c
        mbedtls/ssl_ticket.c
        mbedtls/ssl_tls.c
        mbedtls/threading.c
        mbedtls/timing.c
        mbedtls/version.c
        mbedtls/version_features.c
        mbedtls/x509.c
        mbedtls/x509_create.c
        mbedtls/x509_crl.c
        mbedtls/x509_crt.c
        mbedtls/x509_csr.c
        mbedtls/x509write_crt.c
        mbedtls/x509write_csr.c
        mbedtls/xtea.c

        # End Bibledit kernel.
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log
        )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        bibledit

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        )

# To locate the header files during compile time,
# add the path(s) to the header files.
include_directories(
        .
)
