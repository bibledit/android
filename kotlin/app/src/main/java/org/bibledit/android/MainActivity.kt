package org.bibledit.android

import android.Manifest
import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.os.Bundle
import android.webkit.WebView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.app.ActivityCompat.requestPermissions
import java.util.Timer
import kotlin.concurrent.schedule

const val REQUEST_CODE = 1000


// The activity's data is at /data/data/org.bibledit.android.
// It writes files to subfolder files.

class MainActivity : AppCompatActivity() {

    val timer = Timer()

    var show = true



    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Handle permissions right at the start of the app.
        checkPermissions();




        setContentView(R.layout.activity_main)

        val webview: WebView = findViewById(R.id.webview)

        @SuppressLint("SetJavaScriptEnabled")
        webview.settings.javaScriptEnabled = true

        // Without this line the URL will open in an external browser.
        // With this line, the URL will open within the app.
        //webview.webViewClient = MyWebViewClient()
        MyWebViewClient().also { webview.webViewClient = it }

        webview.loadUrl("https://bibledit.org:8091")

        timer.schedule(2000L, 5000L) {
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
            var webview : WebView = this.findViewById(R.id.webview)
            println(StringFromJNI())
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


}