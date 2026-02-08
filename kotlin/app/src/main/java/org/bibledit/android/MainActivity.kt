package org.bibledit.android

import android.Manifest
import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.webkit.WebView
import android.widget.TabHost
import androidx.appcompat.app.AppCompatActivity
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.app.ActivityCompat
import androidx.core.app.ActivityCompat.requestPermissions
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

    var layout: ConstraintLayout? = null
    var webview: WebView? = null
    var tabhost: TabHost? = null // Todo deprecated: Use modern version, see to that later.

    val timer = Timer()
    var show = true

    var webAppBaseUrl: String = ""


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)
        layout = findViewById(R.id.main)

        // Handle permissions right at the start of the app.
        checkPermissions();

        // Get the free port number found by the Bibledit library.
        val port: String = GetNetworkPort()
        webAppBaseUrl = "http://localhost:" + port + "/"

        // Root folder for the web app.
        val webroot: String = getWebRoot()

        InitializeLibrary (webroot, webroot);

        SetTouchEnabled (true);

        StartLibrary ();

        StartWebView (webAppBaseUrl);

        // Install the assets if needed.
        installAssets (webroot);

        // Log information about where to find Bibledit's data. Todo test this.
        Log ("Bibledit data location: " + webroot);

        // Log information about whether running on Android or on Chrome OS. Todo test this on ChromeOS
        if (applicationContext.packageManager.hasSystemFeature("org.chromium.arc.device_management")) {
            Log ("Running on Chrome OS");
            // Enable Chrome OS in the library, for something specific to Chrome.
            // See https://github.com/bibledit/cloud/issues/282.
            RunOnChromeOS ();
        } else {
            Log ("Running on Android"); // Todo test this on Android whether it logs.
        }

        timer.schedule(2000L, 2000L) { // Todo better 5000 ms.
            onRepeatingTimeout()
        }
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
        // Modifying widgets must be done on the UI thread.
        runOnUiThread(Runnable {
            show = !show
            if (show) {
//                StartWebView(webAppBaseUrl)
            } else {
//                val msg : String = "webAppBaseUrl is " + webAppBaseUrl + " " + StringFromJNI()
//                Log.i("Timer", msg)
//                webview?.loadData(msg, "text/html", "utf-8")
            }
            val count : Int? = layout?.childCount
        })

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
    private fun StartWebView(url : String)
    {
        // Indicate that the view is now plain.
        tabhost = null

        webview = GetNewWebViewWithSettings()

        layout?.removeAllViews()
        layout?.addView(webview)

        @SuppressLint("SetJavaScriptEnabled")
        webview?.settings?.javaScriptEnabled = true

        // Without this line the URL will open in an external browser.
        // With this line, the URL will open within the app.
        //webview.webViewClient = MyWebViewClient()
        MyWebViewClient().also { webview?.webViewClient = it }

        webview?.loadUrl(webAppBaseUrl)
    }


    private fun GetNewWebViewWithSettings () : WebView
    {
        var newwebview = WebView(this).apply {
            layoutParams = ConstraintLayout.LayoutParams(
                ConstraintLayout.LayoutParams.MATCH_PARENT,
                ConstraintLayout.LayoutParams.MATCH_PARENT
            ).apply {
                bottomToBottom = ConstraintLayout.LayoutParams.BOTTOM
                endToEnd = ConstraintLayout.LayoutParams.END
                startToStart = ConstraintLayout.LayoutParams.START
                topToTop = ConstraintLayout.LayoutParams.TOP
            }
        }

        @SuppressLint("SetJavaScriptEnabled")
        newwebview.getSettings().setJavaScriptEnabled(true)

        // No built-in zoom controls,
        // because these may cover clickable links,
        // which then can't be clicked anymore.
        // https://github.com/bibledit/cloud/issues/321
        newwebview.getSettings().setBuiltInZoomControls(false)
        newwebview.getSettings().setSupportZoom(false)
        newwebview.getSettings().setDisplayZoomControls(false)

        newwebview.getSettings().setDomStorageEnabled(true)

        // Without this line the URL will open in an external browser.
        // With this line, the URL will open within the app.
        //webview.webViewClient = MyWebViewClient()
        MyWebViewClient().also { newwebview.webViewClient = it }

        return newwebview
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
                            val readAssetFile = { ->
                                val input = assetManager.open("external/$filename")
                                val size = input.available()
                                val buffer = ByteArray(size)
                                input.read(buffer)
                                input.close()
                                // The last statement is implicitly returned.
                                buffer
                            }
                            val buffer = readAssetFile();
                            // Optionally create output directories.
                            val createOutputDirectories = { ->
                                var file = File(filename)
                                val parent = file.parent
                                if (parent != null) {
                                    val parentFile = File(webroot, parent)
                                    if (!parentFile.exists()) {
                                        parentFile.mkdirs()
                                    }
                                }
                            }
                            createOutputDirectories();
                            // Write the file to the external webroot directory.
                            var outFile = File(webroot, filename)
                            var outStream = FileOutputStream(outFile)
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
                preferences.edit ().putString ("version", GetVersionNumber()).apply ();
            }
        }.start()
    }

}