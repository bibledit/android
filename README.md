# Bibledit for Android

## Developer information

To get a shell on the device: $ adb shell

To view the logbook from within the shell on the device: $ logcat

## Dependencies

Native dependencies are in folder "sqlite" and can be built from there.

## Build test release

Run script ./copy to copy the source to a temporal location separate from the source repository.
Run script ./native in the temporal location to build the core library.
Run script ./debug in the temporal location to create a debug version of the app.
Run script ./run to run the app on a connected Android device.
Run script ./release to create a release version for upload to the play store.

