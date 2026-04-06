package org.bibledit.android

import android.Manifest
import android.annotation.SuppressLint
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.net.Uri
import android.os.Bundle
import android.util.Log
import android.view.ActionMode
import android.view.Menu
import android.view.View
import android.view.WindowManager
import android.view.inputmethod.InputMethodManager
import android.webkit.WebView
import android.widget.TabHost
import android.widget.TabHost.OnTabChangeListener
import android.widget.TabHost.TabContentFactory
import android.widget.TabHost.TabSpec
import androidx.appcompat.app.AppCompatActivity
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.app.ActivityCompat
import androidx.core.app.ActivityCompat.requestPermissions
import androidx.core.content.edit
import com.google.android.material.tabs.TabLayout
import com.google.android.material.tabs.TabLayout.OnTabSelectedListener
import org.json.JSONArray
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.util.Timer
import kotlin.concurrent.schedule


const val WRITE_EXTERNAL_STORAGE_REQUEST_CODE = 1000


// The activity's data is at /data/data/org.bibledit.android.
// It writes files to subfolder files.

class MainActivity : AppCompatActivity() {

    var webview: WebView? = null
    var tabhost: TabHost? = null
    var tablayout : TabLayout? = null
    var displayingSplashScreen: Boolean = false

    var timer: Timer? = null

    var webAppPortNumber: Int = 0
    var webAppBaseUrl: String = ""

    var previousTabsState: String? = null
    var lastTabUrl: String? = null
    var lastTabIdentifier: String? = null

    var previousSyncState: String? = null


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.splash_screen)
        displayingSplashScreen = true

        // Handle permissions right at the start of the app.
        checkPermissions()

        // Get the free port number found by the Bibledit library.
        webAppPortNumber = GetNetworkPort().toInt()
        webAppBaseUrl = "http://localhost:" + webAppPortNumber.toString() + "/"

        // Root folder for the web app.
        val webroot: String = getWebRoot()

        InitializeLibrary (webroot, webroot)

        SetTouchEnabled (true)

        StartLibrary ()

        // Install the assets if needed.
        installAssets (webroot)

        // Log information about where to find Bibledit's data.
        Log ("Bibledit data location: " + webroot)

        // Log information about whether running on Android or on Chrome OS.
        if (applicationContext.packageManager.hasSystemFeature("org.chromium.arc.device_management")) {
            Log ("Running on Chrome OS")
            // Enable Chrome OS in the library, for something specific to Chrome.
            // See https://github.com/bibledit/cloud/issues/282.
            RunOnChromeOS ()
        } else {
            Log ("Running on Android")
        }

        // Configure all the app's WebViews for debugging.
        WebView.setWebContentsDebuggingEnabled(true)
    }


    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Don't create the menu.
        return false
    }


    // Function is called when the user starts the app.
    override fun onStart() {
        super.onStart()
        StartLibrary()
        startTimer()
    }


    // Function is called when the user returns to the activity.
    override fun onRestart() {
        super.onRestart()
        StartLibrary()
        startTimer()
    }


    // Function is called when the app is moved to the foreground again.
    public override fun onResume() {
        super.onResume()
        StartLibrary()
        startTimer()
    }


    // Function is called when the app is obscured.
    public override fun onPause() {
        super.onPause()
        StopLibrary()
        stopTimer()
    }


    // Function is called when the user completely leaves the activity.
    override fun onStop() {
        super.onStop()
        StopLibrary()
        stopTimer()
    }


    // Function is called when the app gets completely destroyed.
    public override fun onDestroy() {
        super.onDestroy()
        StopLibrary()
        stopTimer()
        // Crashes: while (IsRunning ()) {};
        ShutdownLibrary()
    }


    // https://developer.android.com/guide/topics/resources/runtime-changes
    // The attribute android:configChanges is added to the <activity> element in AndroidManifest.xml.
    // Function below is called on device orientation and keyboard hide.
    // This changes the following behaviour:
    // 1. It prevents app restart on device orientation change.
    // 2. It prevents app restart on Bluetooth keyboard (dis)connect.
    // The result is that any Bibledit editor windows open remain open and are not lost.
    // https://developer.android.com/guide/topics/resources/runtime-changes
    override fun onConfigurationChanged(newConfig: Configuration)
    {
        super.onConfigurationChanged(newConfig)
    }


    // The native methods implemented by the bibledit native library wrapper
    // which is packaged with this application.
    // There should be no understores (_) in the function name.
    // This avoids a "java.lang.UnsatisfiedLinkError: Native method not found" exception.
    external fun StringFromJNI(): String
    external fun GetVersionNumber(): String
    external fun GetNetworkPort(): String
    external fun SetTouchEnabled(enabled: Boolean)
    external fun InitializeLibrary(resources: String, webroot: String)
    external fun StartLibrary()
    external fun IsRunning(): Boolean
    external fun IsSynchronizing(): String
    external fun GetExternalUrl(): String
    external fun GetPagesToOpen(): String
    external fun StopLibrary()
    external fun ShutdownLibrary()
    external fun Log(message: String)
    external fun GetLastPage(): String
    external fun RunOnChromeOS()
    external fun DisableSelectionPopupChromeOS(): String
    external fun InternalServerIsUp(port: Int) : Boolean


    companion object {
        // Load the native Bibledit library on application startup.
        // The library has already been unpacked into
        // /data/data/org.bibledit.android
        // at installation time by the package manager.
        init {
            System.loadLibrary("bibledit")
        }
    }

    fun onRepeatingTimeout ()
    {
        // On app startup is displays a splash screen.
        // If the embedded webserver does not yet run, quit right here.
        if (webAppPortNumber == 0) {
            return
        }
        if (displayingSplashScreen) {
            if (InternalServerIsUp(webAppPortNumber)) {
                displayingSplashScreen = false
            }
            return
        }
        // From here on and below, the embedded webserver is running.

        // Get the pages to open in JSON.
        // Take action if it differs from previous time.
        val jsonString: String = GetPagesToOpen ()
        if (jsonString != previousTabsState) {

            // If no pages to open are given, it means the app is in advanced mode.
            if (jsonString.isEmpty()) {
                // Modifying widgets must be done on the UI thread.
                runOnUiThread {
                    startSingleView(webAppBaseUrl)
                }
            }

            // Pages to open are given: Open the tabs for basic mode.
            else {
                // Modifying widgets must be done on the UI thread.
                runOnUiThread {
                    startTabbedViewV2(jsonString)
                }
            }

            // Save the JSON for next time.
            previousTabsState = jsonString
        }

        // Check whether the Bibledit kernel has an external URL to be opened.
        // If so open it in the system browser.
        val externalUrl: String? = GetExternalUrl()
        if (externalUrl != null && !externalUrl.isEmpty()) {
            Log.d("Bibledit start browser", externalUrl)
            val browserIntent = Intent(Intent.ACTION_VIEW, Uri.parse(externalUrl))
            startActivity(browserIntent)
        }

        // Check whether to keep the screen on during send and receive.
        // Keeping the screen on is needed because that will keep the app in the foreground.
        // If the app went into the background, then it would not complete the send/receive cycle.
        val syncState: String? = IsSynchronizing()
        if (syncState == "true") {
            runOnUiThread {
                getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
            }
        }
        if (syncState == "false") {
            if (syncState == previousSyncState) {
                runOnUiThread {
                    getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
                }
            }
        }
        previousSyncState = syncState
    }


    private fun checkPermissions(): Boolean
    {
        // https://developer.android.com/training/permissions/requesting

        // Determine whether the app was already granted the permission.
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED) {
            // Indicate to the caller that the requested permissions were already granted.
            return true
        }

        // Check whether the user should be informed about the reason for requesting this permission.
        if (ActivityCompat.shouldShowRequestPermissionRationale(
                this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            // Show an educational UI to the user.
            // In this UI, describe why the feature, which the user wants to enable,
            // needs a particular permission.
            // But for the permission to write to the external storage,
            // Bibledit does not show this UI.
        }

        // Request the permission.
        // Users will see a system permission dialog,
        // where they can choose whether to grant this permission to the app.
        // Traditionally, you manage a request code yourself as part of the permission request
        // and include this request code in your permission callback logic.
        // Another option is to use the RequestPermission contract,
        // included in an AndroidX library,
        // where you allow the system to manage the permission request code for you.
        // Because using the RequestPermission contract simplifies your logic,
        // it's recommended that you use it when possible.
        requestPermissions(
            this,
            arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE), WRITE_EXTERNAL_STORAGE_REQUEST_CODE)

        // Indicate to the caller that the permission was not granted (yet).
        // But the dialog for requesting permissions will be visible to the user now.
        return false
    }

    override fun onRequestPermissionsResult(requestCode: Int,
                                            permissions: Array<String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            WRITE_EXTERNAL_STORAGE_REQUEST_CODE -> {
                // If request is cancelled, the result arrays are empty.
                if ((grantResults.isNotEmpty() &&
                            grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    // Permission is granted. Continue the action or workflow
                    // in your app.
                } else {
                    // Explain to the user that the feature is unavailable because
                    // the feature requires a permission that the user has denied.
                    // At the same time, respect the user's decision. Don't link to
                    // system settings in an effort to convince the user to change
                    // their decision.
                }
                return
            }

            // Add other 'when' lines to check for other
            // permissions this app might request.
            else -> {
                // Ignore all other requests.
            }
        }
    }


    // Open the single webview configuration.
    private fun startSingleView(url : String)
    {
        tabhost = null
        tablayout = null
        setContentView(R.layout.single_view)
        webview = findViewById<WebView>(R.id.singleview)
        applySettingsToWebView(webview)
        webview?.loadUrl(webAppBaseUrl)
    }


    // Apply settings to the passed WebView.
    // Kotlin always use pass-by-value.
    // When passing objects or non-primitive types, the function copies the reference, simulating pass-by-reference.
    // Changes inside the method affect the external object due to the shared reference.
    private fun applySettingsToWebView (webView: WebView?)
    {
        @SuppressLint("SetJavaScriptEnabled")
        webView!!.getSettings().setJavaScriptEnabled(true)

        // No built-in zoom controls,
        // because these may cover clickable links,
        // which then can't be clicked anymore.
        // https://github.com/bibledit/cloud/issues/321
        webView!!.getSettings().setBuiltInZoomControls(false)
        webView!!.getSettings().setSupportZoom(false)
        webView!!.getSettings().setDisplayZoomControls(false)

        webView!!.getSettings().setDomStorageEnabled(true)

        // Without this line the URL will open in an external browser.
        // With this line, the URL will open within the app.
        MyWebViewClient().also { webView!!.webViewClient = it }
    }


    private fun startTabbedViewV2(tabsJSON: String) { // Todo
        webview = null

        setContentView(R.layout.tabbed_view_v2)

        tablayout = findViewById(R.id.tabLayout2)

        tablayout!!.tabGravity = TabLayout.GRAVITY_FILL

        val jsonArray = JSONArray(tabsJSON)
        val length = jsonArray.length()
        var active = 0

        fun getWebView (i: Int) : WebView {
            when (i) {
                0 -> return findViewById<WebView>(R.id.testwebview1)
                1 -> return findViewById<WebView>(R.id.testwebview2)
                2 -> return findViewById<WebView>(R.id.testwebview3)
                3 -> return findViewById<WebView>(R.id.testwebview4)
                4 -> return findViewById<WebView>(R.id.testwebview5)
                // If the input is out of range, return the last WebView.
                else -> return return findViewById<WebView>(R.id.testwebview5)
            }
        }

        for (i in 0..<length) {
            val jsonObject = jsonArray.getJSONObject(i)

            val label = jsonObject.getString("label")
            tablayout!!.addTab(tablayout!!.newTab().setText(label))

            val url = jsonObject.getString("url")

            val tabwebview: WebView = getWebView(i)
            applySettingsToWebView(tabwebview)
            tabwebview?.loadUrl(webAppBaseUrl + url)

            if (url.contains("resource"))
                active = i
            lastTabIdentifier = label
            lastTabUrl = url
        }

        tablayout!!.getTabAt(active)?.select()
        val activeWebView = getWebView(active)
        activeWebView.visibility = View.VISIBLE

        tablayout!!.addOnTabSelectedListener(object : OnTabSelectedListener {
            override fun onTabSelected(tab: TabLayout.Tab) {
                val webview = getWebView(tab.position)
                webview.visibility = View.VISIBLE
            }
            override fun onTabUnselected(tab: TabLayout.Tab) {
                val webview = getWebView(tab.position)
                webview.visibility = View.GONE
            }
            override fun onTabReselected(tab: TabLayout.Tab) {
            }
        })




//        tabhost!!.setOnTabChangedListener(object : OnTabChangeListener {
//            override fun onTabChanged(tabId: String) {
//                // Check whether to reload the settings page.
//                // The reason for this is as follows:
//                // When the user clicks any of the links in the settings page,
//                // there is no way to go back to the main settings page.
//                // The above applies in tabbed mode, as there's no menu then.
//                // So when the settings tab is activated,
//                // it ensures that the main setting page is loaded.
//                if (tabId == lastTabIdentifier) {
//                    val webview = tabhost!!.getCurrentView() as WebView
//                    val actualUrl = webview.getUrl()
//                    val desiredUrl = webAppBaseUrl + lastTabUrl
//                    if (actualUrl != desiredUrl) {
//                        runOnUiThread {
//                            webview.loadUrl(desiredUrl)
//                        }
//                    }
//                }
//                // Hide the soft keyboard.
//                // See https://github.com/bibledit/cloud/issues/269 for reasons.
//                val webview = tabhost!!.getCurrentView() as WebView?
//                runOnUiThread {
//                    val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager?
//                    imm?.hideSoftInputFromWindow (webview!!.getWindowToken(), 0);
//                }
//            }
//        })
    }

    private fun getWebRoot() : String
    {
        // The directory of the external files.
        // Usually this is /storage/emulated/0/Android/data/org.bibledit.android/files
        // Files in this directory cannot be made executable
        // because system has a protection mechanism for this.
        val externalDirectory: String = getExternalFilesDir(null)!!.getAbsolutePath()

        // Take the external directory for the webroot if it exists.
        val file = File(externalDirectory)
        if (file.exists()) {
            return externalDirectory
        }

        // External directory does not exist.
        // The protected directory.
        // Usually this is /data/user/0/org.bibledit.android/files normally.
        // Files there can be set executable.
        val internalDirectory: String = getFilesDir().getAbsolutePath()
        return internalDirectory
    }

    private fun installAssets (webroot: String)
    {
        Thread {

            val logTag = "InstallAssets"

            // Check whether the Bibledit kernel version has been installed, if not install it.
            val libraryVersion = GetVersionNumber()
            val preferences = getPreferences(MODE_PRIVATE)
            val installedVersion = preferences.getString("version", "")!!
            Log.i(logTag,"Library version is $libraryVersion and installed version is $installedVersion")
            if (installedVersion != libraryVersion) {

                try {

                    Log.i(logTag, "Install version $libraryVersion over version $installedVersion")

                    // The assets are not visible in the standard filesystem, but remain inside the .apk file.
                    // The manager accesses them.
                    val assetManager = assets

                    // Read the asset index and convert it to a list of file names.
                    val readAssetIndex = { ->
                        val input: InputStream = assetManager.open("asset.external")
                        val size = input.available()
                        val buffer = ByteArray(size)
                        input.read(buffer)
                        input.close()
                        val text = String(buffer)
                        text.split("\\r?\\n".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
                    }
                    val files = readAssetIndex()
                    Log.i(logTag, "Install ${files.count()} assets")

                    // Iterate through the asset files.
                    for (filename in files) {
                        try {
                            // Read the file into memory.
                            val readAssetFile = {
                                val input = assetManager.open("external/$filename")
                                val size = input.available()
                                val buffer = ByteArray(size)
                                input.read(buffer)
                                input.close()
                                // The last statement is implicitly returned.
                                buffer
                            }
                            val buffer = readAssetFile()
                            // Optionally create output directories.
                            val createOutputDirectories = {
                                val file = File(filename)
                                val parent = file.parent
                                if (parent != null) {
                                    val parentFile = File(webroot, parent)
                                    if (!parentFile.exists()) {
                                        parentFile.mkdirs()
                                    }
                                }
                            }
                            createOutputDirectories()
                            // Write the file to the external webroot directory.
                            val outFile = File(webroot, filename)
                            val outStream = FileOutputStream(outFile)
                            outStream.write(buffer, 0, buffer.size)
                            outStream.flush()
                            outStream.close()
                            //Log.i(logtag, "Writing $filename to $webroot")
                        } catch (e: IOException) {
                            e.printStackTrace()
                        }
                    }
                }
                catch (e : Exception) {
                    e.printStackTrace ();
                }
                finally {
                }

                // Store the Bibledit kernel version number as the installed version.
                preferences.edit { putString("version", GetVersionNumber()) };
            }
        }.start()
    }


    private fun startTimer()
    {
        stopTimer()
        timer = Timer()
        timer!!.schedule(1000L, 1000L) {
            onRepeatingTimeout()
        }
    }


    private fun stopTimer()
    {
        if (timer != null) {
            timer!!.cancel();
            timer = null;
        }
    }

    public override fun onActionModeStarted(mode: ActionMode) {
        // https://developer.android.com/reference/android/view/ActionMode.html
        val disable = DisableSelectionPopupChromeOS()
        if (disable == "true") {
            val menu: Menu = mode.getMenu()
            menu.clear()
            //mode.finish ();
            //mode.invalidate ();
        }
        super.onActionModeStarted(mode)
    }

    override fun onBackPressed()
    {
        Log.i("Back", "on back pressed")
        // The Android back button navigates back in the web view.
        // This is the behaviour people expect.
        if ((webview != null) && webview!!.canGoBack()) {
            webview!!.goBack()
            return
        } else if (tabhost != null) {
            val webview = tabhost!!.    getCurrentView() as WebView
            if (webview.canGoBack()) {
                webview.goBack()
                return
            }
        }

        // Otherwise defer to system default behavior.
        super.onBackPressed()
    }


}