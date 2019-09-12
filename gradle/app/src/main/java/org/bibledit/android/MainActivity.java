package org.bibledit.android;

import androidx.appcompat.app.AppCompatActivity;

import android.app.DownloadManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.net.Uri;
import android.net.http.SslError;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.ActionMode;
import android.view.Menu;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.webkit.DownloadListener;
import android.webkit.SslErrorHandler;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.TabHost;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;


// The activity's data is at /data/data/org.bibledit.android.
// It writes files to subfolder files.


public class MainActivity extends AppCompatActivity
{

    WebView webview = null;
    TabHost tabhost = null;
    int resumecounter = 0;
    String webAppUrl = "http://localhost:8080/";
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

        InitializeLibrary (webroot, webroot);

        SetTouchEnabled (true);

        StartLibrary ();

        StartWebView (webAppUrl);

        // Install the assets if needed.
        installAssets (webroot);

        // Log information about where to find Bibledit's data.
        Log ("Bibledit data location: " + webroot);

        // Log information about whether running on Android or on Chrome OS.
        if (getApplicationContext().getPackageManager().hasSystemFeature("org.chromium.arc.device_management")) {
            Log ("Running on Chrome OS");
            // Enable Chrome OS in the library, for something specific to Chrome.
            // See https://github.com/bibledit/cloud/issues/282.
            RunOnChromeOS ();
        } else {
            Log ("Running on Android");
        }

        // Timer for running repeating tasks.
        startTimer ();

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
    }


    // List of native methods
    // that areimplemented by the native library which is packaged with this application.
    // There should be no understores (_) in the function name.
    // This avoids a "java.lang.UnsatisfiedLinkError: Native method not found" exception.
    public native String GetVersionNumber ();
    public native void SetTouchEnabled (boolean enabled);
    public native void InitializeLibrary (String resources, String webroot);
    public native void StartLibrary ();
    public native boolean IsRunning ();
    public native String IsSynchronizing ();
    public native String GetExternalUrl ();
    public native String GetPagesToOpen ();
    public native void StopLibrary ();
    public native void ShutdownLibrary ();
    public native void Log (String message);
    public native String GetLastPage ();
    public native void RunOnChromeOS ();
    public native String DisableSelectionPopupChromeOS ();
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
        StartLibrary ();
        startTimer ();
    }


    // Function is called when the user returns to the activity.
    @Override
    protected void onRestart ()
    {
        super.onRestart();
        StartLibrary ();
        startTimer ();
    }


    // Function is called when the app is moved to the foreground again.
    @Override
    public void onResume ()
    {
        super.onResume();
        StartLibrary ();
        startTimer ();
    }


    // Function is called when the app is obscured.
    @Override
    public void onPause ()
    {
        super.onPause ();
        StopLibrary ();
        stopTimer ();
    }


    // Function is called when the user completely leaves the activity.
    @Override
    protected void onStop ()
    {
        super.onStop();
        StopLibrary ();
        stopTimer ();
    }


    // Function is called when the app gets completely destroyed.
    @Override
    public void onDestroy ()
    {
        super.onDestroy ();
        StopLibrary ();
        stopTimer ();
        // Crashes: while (IsRunning ()) {};
        ShutdownLibrary ();
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
    // Library libbibledit calls the Bibledit library.
    // The library has already been unpacked into
    // /data/data/org.bibledit.android/lib/libbbibledit.so
    // at installation time by the package manager.
    static {
        System.loadLibrary("bibledit");
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
                String libraryVersion = GetVersionNumber ();
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
                preferences.edit ().putString ("version", GetVersionNumber ()).apply ();
            }
        };
        thread.start ();
    }



  /*

   Upon resume, it used to check that the URL loaded in the webview is a page
   served by the embedded webserver.
   The app was modified subsequently, as follows:
   An external page is no longer loaded in the embedded webview.
   It is now loaded in the system browser.
   Therefore this check on a local URL is no longer needed.
   It was removed.

   There was an idea that the app would shut down itself after it would be in the background for a while.
   This works well when another app is started and thus Bibledit goes to the background.
   But when the screen is powered off, then when Bibledit quits itself, Android keeps restarting it.
   And when the screen is powered on again, then Bibledit cannot find the page.
   Thus since this does not work well, it was not implemented.
   Here's the code that was supposed to do it:
   System.runFinalizersOnExit (true);
   this.finish ();
   Process.killProcess (Process.myPid());
   System.exit (0);

   */


    private void startTimer ()
    {
        stopTimer ();
        timer = new Timer();
        initializeTimerTask();
        timer.schedule (timerTask, 1000);
    }


    private void stopTimer ()
    {
        if (timer != null) {
            timer.cancel();
            timer = null;
        }
    }


    private void initializeTimerTask() {
        timerTask = new TimerTask() {
            public void run() {

                // Check whether to keep the screen on during send and receive.
                String syncState = IsSynchronizing ();
                if (syncState.equals ("true")) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                        }
                    });
                }
                if (syncState.equals ("false")) {
                    if (syncState.equals (previousSyncState)) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                            }
                        });
                    }
                }
                previousSyncState = syncState;

                // Check whether to open an external URL in the system browser.
                String externalUrl = GetExternalUrl ();
                if (externalUrl != null && !externalUrl.isEmpty ()) {
                    Log.d ("Bibledit start browser", externalUrl);
                    Intent browserIntent = new Intent (Intent.ACTION_VIEW, Uri.parse (externalUrl));
                    startActivity(browserIntent);
                }

                // Check whether to open tabbed views or return to the default single view.
                final String jsonString = GetPagesToOpen ();
                if (jsonString != null) {
                    if (!jsonString.equals (previousTabsState)) {
                        if (jsonString.isEmpty ()) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    StartWebView (webAppUrl);
                                }
                            });
                        } else {
                            try {
                                JSONArray jsonArray = new JSONArray (jsonString);
                                int length = jsonArray.length();
                                final List<String> URLs = new ArrayList<String>();
                                final List<String> labels = new ArrayList<String>();
                                Integer active = 0;
                                for (int i = 0; i < length; i++) {
                                    JSONObject jsonObject = jsonArray.getJSONObject(i);
                                    String URL = jsonObject.getString ("url");
                                    URLs.add (URL);
                                    String label = jsonObject.getString ("label");
                                    labels.add (label);
                                    if (URL.contains ("resource")) active = i;
                                    lastTabIdentifier = label;
                                    lastTabUrl = URL;
                                }
                                final Integer tab = active;
                                runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                        StartTabHost (URLs, labels, tab);
                                    }
                                });
                            } catch (JSONException e) {
                                Log.d ("Bibledit error", e.getMessage ());
                            }
                        }
                        previousTabsState = jsonString;
                    }
                }

                // Start timeout for next iteration.
                startTimer ();
            }
        };
    }


    @Override
    public void onBackPressed() {
        // The Android back button navigates back in the web view.
        // This is the behaviour people expect.
        if ((webview != null) && webview.canGoBack()) {
            webview.goBack();
            return;
        } else if (tabhost != null) {
            WebView webview = (WebView) tabhost.getCurrentView ();
            if (webview.canGoBack ()) {
                webview.goBack ();
                return;
            }
        }

        // Otherwise defer to system default behavior.
        super.onBackPressed();
    }


    // Open the single webview configuration.
    private void StartWebView (String PageToOpen)
    {
        // Indicate that the view is now plain.
        tabhost = null;
        // Set up the webview.
        webview = getNewWebViewWithSettings (true);
        setContentView (webview);
        // Enable file download.
        webview.setDownloadListener(new DownloadListener() {
            @Override
            public void onDownloadStart (String url, String userAgent, String contentDisposition, String mimetype, long contentLength) {
                DownloadManager.Request request = new DownloadManager.Request (Uri.parse (url));
                request.allowScanningByMediaScanner();
                // Notification once download is completed.
                request.setNotificationVisibility (DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
                Uri uri = Uri.parse (url);
                String filename = uri.getLastPathSegment ();
                request.setDestinationInExternalPublicDir (Environment.DIRECTORY_DOWNLOADS, filename);
                DownloadManager dm = (DownloadManager) getSystemService (DOWNLOAD_SERVICE);
                dm.enqueue (request);
                // Notification that the file is being downloaded.
                Toast.makeText (getApplicationContext(), "Downloading file", Toast.LENGTH_LONG).show ();
            }
        });
        // Set high quality client.
        webview.setWebChromeClient(new WebChromeClient() {
            // The undocumented method overrides.
            // The compiler fails if you try to put @Override here.
            // It needs three interfaces to handle the various versions of Android.
            public void openFileChooser(ValueCallback<Uri> uploadMsg) {
                myUploadMessage = uploadMsg;
                Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                intent.setType("*/*");
                MainActivity.this.startActivityForResult(Intent.createChooser(intent, "File Chooser"), FILECHOOSER_RESULTCODE);
            }
            public void openFileChooser( ValueCallback uploadMsg, String acceptType) {
                myUploadMessage = uploadMsg;
                Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                intent.setType("*/*");
                MainActivity.this.startActivityForResult(Intent.createChooser(intent, "File Browser"), FILECHOOSER_RESULTCODE);
            }
            public void openFileChooser(ValueCallback<Uri> uploadMsg, String acceptType, String capture) {
                myUploadMessage = uploadMsg;
                Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                intent.setType("*/*");
                MainActivity.this.startActivityForResult (Intent.createChooser (intent, "File Chooser"), MainActivity.FILECHOOSER_RESULTCODE);
            }
        });
        // Enable debugging this WebView from a developer's machine.
        // But this failed to work since December 2018 on some Android versions:
        // W/dalvikvm(14740): VFY: unable to resolve static method 45: Landroid/webkit/WebView;.setWebContentsDebuggingEnabled
        // webview.setWebContentsDebuggingEnabled (true);
        // Load page.
        webview.loadUrl (webAppUrl + PageToOpen);
    }


    // Open several webviews in tabs.
    private void StartTabHost (List<String> URLs, List<String> labels, Integer active)
    {
        webview = null;

        setContentView (R.layout.activity_main);

        tabhost = (TabHost) findViewById (R.id.tabhost);
        tabhost.setup ();

        TabHost.TabSpec tabspec;
        TabHost.TabContentFactory factory;

        for (int i = 0; i < URLs.size(); i++) {
            final String URL = URLs.get (i);
            final String label = labels.get (i);
            tabspec = tabhost.newTabSpec (label);
            tabspec.setIndicator (label);
            factory = new TabHost.TabContentFactory () {
                @Override
                public View createTabContent (String tag) {
                    WebView webview = getNewWebViewWithSettings (false);
                    webview.loadUrl (webAppUrl + URL);
                    return webview;
                }
            };
            tabspec.setContent(factory);
            tabhost.addTab (tabspec);
        }

        // It used to halve the height of the tabs on the screen.
        // The goal of that was to use less space on the screen,
        // leaving more space for the editing areas.
        // But a user made this remark:
        // "On my new 8 inch tablet,
        // the tabbed menu is so small
        // that I often miss and the top Android status bar pulls down instead."
        // So it is better to not halve the height, but use another reduction factor.
        for (int i = 0; i < tabhost.getTabWidget().getChildCount(); i++) {
            tabhost.getTabWidget().getChildAt(i).getLayoutParams().height *= 0.75;
        }

        tabhost.setCurrentTab (active);

        tabhost.setOnTabChangedListener(new TabHost.OnTabChangeListener(){
            @Override
            public void onTabChanged(String tabId) {
                // Check whether to reload the settings page.
                // The reason for this is as follows:
                // When the user clicks any of the links in the settings page,
                // there is no way to go back to the main settings page.
                // The above applies in tabbed mode, as there's no menu then.
                // So when the settings tab is activated,
                // it ensures that the main setting page is loaded.
                if (tabId.equals (lastTabIdentifier)) {
                    final WebView webview = (WebView) tabhost.getCurrentView ();
                    String actualUrl = webview.getUrl ();
                    final String desiredUrl = webAppUrl + lastTabUrl;
                    if (!actualUrl.equals (desiredUrl)) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                WebViewLoadURL (webview, desiredUrl);
                            }
                        });
                    }
                }
                // Hide the soft keyboard.
                // See https://github.com/bibledit/cloud/issues/269 for reasons.
                final WebView webview = (WebView) tabhost.getCurrentView ();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        hideKeyboard (webview);
                    }
                });
            }
        });
    }


    private void WebViewLoadURL (WebView webview, String url)
    {
        webview.loadUrl (url);
    }


    private void hideKeyboard (WebView webview)
    {
        // Get the input manager that has the keyboard.
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);

        // If no view currently has focus, then there's no keyboard to hide.
        if (webview == null) return;

        // Hide the soft keyboard.
        imm.hideSoftInputFromWindow (webview.getWindowToken(), 0);
    }


    @Override
    public void onActionModeStarted (ActionMode mode)
    {
        // https://developer.android.com/reference/android/view/ActionMode.html
        final String disable = DisableSelectionPopupChromeOS ();
        if (disable.equals ("true")) {
            Menu menu = mode.getMenu ();
            menu.clear();
            //mode.finish ();
            //mode.invalidate ();
        }
        super.onActionModeStarted(mode);
    }


    private WebView getNewWebViewWithSettings (boolean zoom)
    {
        WebView webview = new WebView (this);
        webview.getSettings().setJavaScriptEnabled (true);
        if (zoom) {
            // No build-in zoom controls,
            // because these may cover clickable links,
            // which then can't be clicked anymore.
            // https://github.com/bibledit/cloud/issues/321
            webview.getSettings().setBuiltInZoomControls (false);
            webview.getSettings().setSupportZoom (true);
        }
        webview.getSettings().setDomStorageEnabled (true);
        webview.setWebViewClient(new WebViewClient() {
            @Override
            public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) {
                // The embedded server at https://localhost has a known certificate.
                // But creating working certificates for localhost is impossible.
                // This error handler is going to check the certificate it received.
                // If the error handler is 100% sure that the certificate
                // is from the server embedded in the app, it proceeds.
                // In all other cases it cancels the operations.
                // This is the most secure solution possible.
                // https://developer.android.com/reference/android/net/http/SslError
                // https://developer.android.com/reference/android/webkit/SslErrorHandler
                // https://github.com/bibledit/cloud/issues/293
                // Check 1: The certificate has been issued to a specific known name.
                String issuedToName = error.getCertificate().getIssuedTo().getCName();
                boolean issuedToGood = (issuedToName.indexOf ("localhost.daplie.com") == 0);
                // Check 2: The URL is localhost at a known port.
                // That can only be this app's embedded webserver.
                String url = error.getUrl();
                boolean urlGood = url.indexOf ("https://localhost:8081") == 0;
                // Check 3: The certificate has been issued by a specific known name.
                String issuedByName = error.getCertificate().getIssuedBy().getCName();
                boolean issuedByGood = issuedByName.indexOf ("RapidSSL SHA256 CA") == 0;
                // If all checks passed, we're 100% sure this is our known certificate, so it's secure.
                boolean proceed = (issuedToGood && urlGood && issuedByGood);
                // Take decision what to do for full security.
                if (proceed) handler.proceed();
                else handler.cancel();
            }
        });
        return webview;
    }


}
