package org.bibledit.android;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.webkit.ValueCallback;
import android.webkit.WebView;
import android.widget.TabHost;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Timer;
import java.util.TimerTask;


// The activity's data is at /data/data/org.bibledit.android.
// It writes files to subfolder files.


public class MainActivity extends AppCompatActivity
{

    WebView webview = null;
    TabHost tabhost = null;
    int resumecounter = 0;
    String webAppUrl = "http://bibledit.org:8080/"; // Todo  "http://localhost:8080/";
    Timer timer;
    TimerTask timerTask;
    String previousSyncState;
    private ValueCallback<Uri> myUploadMessage;
    private final static int FILECHOOSER_RESULTCODE = 1;
    String previousTabsState;
    String lastTabUrl;
    String lastTabIdentifier;
    String previousDisableSelectionPopup = "false";


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        if (requestCode == FILECHOOSER_RESULTCODE) {
            if (myUploadMessage == null) return;
            Uri result = intent == null || resultCode != RESULT_OK ? null : intent.getData();
            myUploadMessage.onReceiveValue (result);
            myUploadMessage = null;
        }
    }


    // Function is called when the app gets launched.
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // The directory of the external files.
        // On a Nexus 10 this is /storage/emulated/0/Android/data/org.bibledit.android/files
        // Files in this directory cannot be made executable.
        // The system has a protection mechanism for this.
        String externalDirectory = getExternalFilesDir (null).getAbsolutePath ();

        // The protected directory that contains files that can be set executable.
        // This would be /data/data/org.bibledit.android/files
        // Files there can be set executable.
        String internalDirectory = getFilesDir ().getAbsolutePath ();

        // Take the external directory for the webroot, if it exists, else the internal directory.
        String webroot = externalDirectory;
        File file = new File (externalDirectory);
        if (!file.exists ()) webroot = internalDirectory;

        // Todo InitializeLibrary (webroot, webroot);

        // Todo SetTouchEnabled (true);

        // Todo StartLibrary ();

        // Todo StartWebView (webAppUrl);

        // Install the assets if needed.
        // Todo installAssets (webroot);

        // Log information about where to find Bibledit's data.
        // Todo Log ("Bibledit data location: " + webroot);

        // Log information about whether running on Android or on Chrome OS.
        if (getApplicationContext().getPackageManager().hasSystemFeature("org.chromium.arc.device_management")) {
            // Todo Log ("Running on Chrome OS");
            // Enable Chrome OS in the library, for something specific to Chrome.
            // See https://github.com/bibledit/cloud/issues/282.
            // Todo RunOnChromeOS ();
        } else {
            // Todo Log ("Running on Android");
        }

        // Timer for running repeating tasks.
        // Todo startTimer ();

        // On Chrome OS, open a web browser, to operate the app from there.
        // This has been disabled now. The reason is the following:
        // https://developer.android.com/topic/arc/index.html
        // Chromebooks run the entire Android OS in a container, similar to Docker or LXC. This means that Android will not have direct access to the system's LAN interface. Instead, IPv4 traffic will pass through an internal layer of network address translation (NAT), and IPv6 unicast traffic will be routed through an extra hop. Outbound unicast connections from an Android app to the internet should mostly work as-is; but in general, inbound connections are blocked. Multicast or broadcast packets from Android will not be forwarded to the LAN through the firewall.
        // As a special exception to the multicast restriction, Chrome OS runs a service that forwards mDNS traffic between Android and the LAN interface, so the standard Network Service Discovery APIs are the recommended way to discover other devices on the LAN segment. After finding a device on the LAN, an Android app can use standard TCP or UDP unicast sockets to communicate with it.
        // IPv4 connections originating from Android will use the Chrome OS host's IPv4 address. Internally, the Android app will see a private IPv4 address assigned to the network interface. IPv6 connections originating from Android will use a different address from the Chrome OS host, as the Android container will have a dedicated public IPv6 address.
        // if ((this.getPackageManager().hasSystemFeature("org.chromium.arc.device_management"))) {
        // Intent browserIntent = new Intent (Intent.ACTION_VIEW, Uri.parse (webAppUrl));
        // startActivity(browserIntent);
        // }
        // FORCHROMEOS
        // Intent browserIntent = new Intent (Intent.ACTION_VIEW, Uri.parse (webAppUrl));
        // startActivity(browserIntent);
        // FORCHROMEOS

        // Example of a call to a native method
        Log.d("bibledit", stringFromJNI());
    }


    // List of native methods
    // that areimplemented by the native library which is packaged with this application.
    // There should be no understores (_) in the function name.
    // This avoids a "java.lang.UnsatisfiedLinkError: Native method not found" exception.
    // Todo public native String GetVersionNumber ();
    // Todo public native void SetTouchEnabled (Boolean enabled);
    // Todo public native void InitializeLibrary (String resources, String webroot);
    // Todo public native void StartLibrary ();
    // Todo public native Boolean IsRunning ();
    // Todo public native String IsSynchronizing ();
    // Todo public native String GetExternalUrl ();
    // Todo public native String GetPagesToOpen ();
    // Todo public native void StopLibrary ();
    // Todo public native void ShutdownLibrary ();
    // Todo public native void Log (String message);
    // Todo public native String GetLastPage ();
    // Todo public native void RunOnChromeOS ();
    // Todo public native String DisableSelectionPopupChromeOS ();
    public native String stringFromJNI();


    @Override
    public boolean onCreateOptionsMenu (Menu menu)
    {
        return false;
    }


    // Function is called when the user starts the app.
    @Override
    protected void onStart ()
    {
        super.onStart();
        // Todo StartLibrary ();
        // Todo startTimer ();
    }


    // Function is called when the user returns to the activity.
    @Override
    protected void onRestart ()
    {
        super.onRestart();
        // Todo StartLibrary ();
        // Todo startTimer ();
    }


    // Function is called when the app is moved to the foreground again.
    @Override
    public void onResume ()
    {
        super.onResume();
        // Todo StartLibrary ();
        // Todo startTimer ();
    }


    // Function is called when the app is obscured.
    @Override
    public void onPause ()
    {
        super.onPause ();
        // Todo StopLibrary ();
        // Todo stopTimer ();
    }


    // Function is called when the user completely leaves the activity.
    @Override
    protected void onStop ()
    {
        super.onStop();
        // Todo StopLibrary ();
        // Todo stopTimer ();
    }


    // Function is called when the app gets completely destroyed.
    @Override
    public void onDestroy ()
    {
        super.onDestroy ();
        // Todo StopLibrary ();
        // Todo stopTimer ();
        // Crashes: while (IsRunning ()) {};
        // Todo ShutdownLibrary ();
    }



    // Function is called on device orientation and keyboard hide.
    // At least, it should be called. But it does not seem to happen.
    // Anyway the call is not needed because the webview reconfigures itself.
    // The app used to crash on device rotation.
    // The fix was adding
    // android:configChanges="orientation|keyboardHidden"
    // to the <activity> element in AndroidManifest.xml.
    // The app used to restart after a Bluetooth keyboard came on or went off.
    // This is according to the specifications.
    // But then the editor would go away, and the app would go back to the home screen after the restart.
    // The fix was to add "keyboard" to the above "configChanges" element.
    // https://developer.android.com/guide/topics/resources/runtime-changes
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }


    // This loads the native Bibledit library on application startup.
    // Library libbibleditjni calls the Bibledit library.
    // The library has already been unpacked into
    // /data/data/org.bibledit.android/lib/libbbibleditjni.so
    // at installation time by the package manager.
    // Used to load the 'native-lib' library on application startup. Todo out.
    static {
        // Todo System.loadLibrary("gnustl_shared");
        // Todo System.loadLibrary("bibleditjni");
        System.loadLibrary("native-lib");
    }


    private void installAssets (final String webroot)
    {

        Thread thread = new Thread ()
        {
            @Override
            public void run ()
            {
                SharedPreferences preferences = getPreferences (Context.MODE_PRIVATE);
                String installedVersion = preferences.getString ("version", "");
                String libraryVersion = ""; // Todo GetVersionNumber ();
                if (installedVersion.equals (libraryVersion)) return;

                try {

                    // The assets are not visible in the standard filesystem, but remain inside the .apk file.
                    // The manager accesses them.
                    AssetManager assetManager = getAssets();

                    // Read the asset index created by ant.
                    String [] files = null;
                    try {
                        InputStream input;
                        input = assetManager.open ("asset.external");
                        int size = input.available ();
                        byte [] buffer = new byte [size];
                        input.read (buffer);
                        input.close();
                        String text = new String (buffer);
                        files = text.split ("\\r?\\n");
                    } catch (IOException e) {
                        e.printStackTrace ();
                    }

                    // Iterate through the asset files.
                    for (String filename : files) {
                        try {
                            // Read the file into memory.
                            InputStream input = assetManager.open ("external/" + filename);
                            int size = input.available ();
                            byte [] buffer = new byte [size];
                            input.read (buffer);
                            input.close ();
                            // Optionally create output directories.
                            File file = new File (filename);
                            String parent = file.getParent ();
                            if (parent != null) {
                                File parentFile = new File (webroot, parent);
                                if (!parentFile.exists ()) {
                                    parentFile.mkdirs ();
                                }
                            }
                            file = null;
                            // Write the file to the external webroot directory.
                            File outFile = new File (webroot, filename);
                            OutputStream out = new FileOutputStream(outFile);
                            out.write (buffer, 0, size);
                            out.flush ();
                            out.close ();
                            outFile = null;
                            out = null;
                            //Log.i (filename, webroot);
                        } catch(IOException e) {
                            e.printStackTrace ();
                        }
                    }

                }
                catch (Exception e) {
                    e.printStackTrace ();
                }
                finally {
                }
                // Todo preferences.edit ().putString ("version", GetVersionNumber ()).apply ();
            }
        };
        thread.start ();
    }






}
