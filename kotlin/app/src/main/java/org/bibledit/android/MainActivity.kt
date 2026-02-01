package org.bibledit.android

import android.Manifest
import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.os.Bundle
import android.webkit.WebView
import android.widget.TabHost
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.app.ActivityCompat
import androidx.core.app.ActivityCompat.requestPermissions
import java.io.File
import java.util.Timer
import kotlin.concurrent.schedule


const val REQUEST_CODE = 1000


// The activity's data is at /data/data/org.bibledit.android.
// It writes files to subfolder files.

class MainActivity : AppCompatActivity() {

    var layout: ConstraintLayout? = null
    var textview: TextView? = null
    var webview: WebView? = null
    var tabhost: TabHost? = null // Todo deprecated: Use modern version, see to that later.

    val timer = Timer()

    var show = true

    var webAppUrl: String = ""


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Handle permissions right at the start of the app.
        checkPermissions();

        // Get the free port number found by the Bibledit library.
        val port: String = GetNetworkPort()
        webAppUrl = "http://localhost:" + port + "/"

        // The directory of the external files.
        // Usually this is /storage/emulated/0/Android/data/org.bibledit.android/files
        // Files in this directory cannot be made executable
        // because system has a protection mechanism for this.
        val externalDirectory: String = getExternalFilesDir(null)!!.getAbsolutePath()

        // The protected directory.
        // Usually this is /data/user/0/org.bibledit.android/files normally.
        // Files there can be set executable.
        val internalDirectory: String = getFilesDir().getAbsolutePath()

        // Take the external directory for the webroot, if it exists, else the internal directory.
        var webroot: String = externalDirectory
        run {
            val file = File(externalDirectory)
            if (!file.exists()) webroot = internalDirectory
        }

        InitializeLibrary (webroot, webroot);

        SetTouchEnabled (true);

        StartLibrary ();

        setContentView(R.layout.activity_main)

        val layout: ConstraintLayout = findViewById(R.id.main)

        // Create and center the dynamic TextView.
        textview = TextView(this).apply {
            text = "Created text"
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
//        layout.addView(textview)


        webview = WebView(this).apply {
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
        layout.addView(webview)




//        StartWebView ("");

        @SuppressLint("SetJavaScriptEnabled")
        webview?.settings?.javaScriptEnabled = true

        // Without this line the URL will open in an external browser.
        // With this line, the URL will open within the app.
        //webview.webViewClient = MyWebViewClient()
        MyWebViewClient().also { webview?.webViewClient = it }

        webview?.loadUrl("https://bibledit.org:8091")

        val html : String = "Hello World"
        webview?.loadData(html, "text/html", "utf-8")


        timer.schedule(2000L, 50L) {
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
    external fun Log(message: String, string: String)
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
            val msg : String = StringFromJNI()
            println(msg)
            textview?.text = StringFromJNI()
            webview?.loadData(msg, "text/html", "utf-8")
            show = !show
            if (show) {
                //webview.loadUrl("https://bibledit.org:8091")
            } else {
                //webview.loadUrl("https://bibledit.org:8091/editone/index")
            }
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
            arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE), REQUEST_CODE)

        // Indicate to the caller that the permission was not granted (yet).
        // But the dialog for requesting permissions will be visible to the user now.
        return false
    }

    override fun onRequestPermissionsResult(requestCode: Int,
                                            permissions: Array<String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            REQUEST_CODE -> {
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
    private fun StartWebView(PageToOpen : String) // Todo implement.
    {
        // Indicate that the view is now plain.
        tabhost = null

        // Set up the webview.
        webview = GetNewWebViewWithSettings (true);
        //setContentView (webview);

        layout?.addView(webview)

        // Enable debugging this WebView from a developer's machine.
        // But this failed to work since December 2018 on some Android versions:
        // W/dalvikvm(14740): VFY: unable to resolve static method 45: Landroid/webkit/WebView;.setWebContentsDebuggingEnabled
        // webview.setWebContentsDebuggingEnabled (true);
        // Load page.
        //val url : String = webAppUrl + PageToOpen
//        val url = "http://bibledit.org:8090"
//        webview?.loadUrl (url);

        val html : String = "Hello World"
        webview?.loadData(html, "text/html", "utf-8")


    }


    private fun GetNewWebViewWithSettings (zoom: Boolean) : WebView
    {
        val webview = WebView(this).apply {
//            text = "Created text"
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
        webview.getSettings().setJavaScriptEnabled(true)

        // No built-in zoom controls,
        // because these may cover clickable links,
        // which then can't be clicked anymore.
        // https://github.com/bibledit/cloud/issues/321
        webview.getSettings().setBuiltInZoomControls(false)
        webview.getSettings().setSupportZoom(false)
        webview.getSettings().setDisplayZoomControls(false)

        webview.getSettings().setDomStorageEnabled(true)

        // Without this line the URL will open in an external browser.
        // With this line, the URL will open within the app.
        //webview.webViewClient = MyWebViewClient()
        MyWebViewClient().also { webview.webViewClient = it }

        return webview
    }


}