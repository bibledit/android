package org.bibledit.android

import android.annotation.SuppressLint
import android.os.Bundle
import android.webkit.WebView
import androidx.appcompat.app.AppCompatActivity
import java.util.Timer
import kotlin.concurrent.schedule


class MainActivity : AppCompatActivity() {

    val timer = Timer()

    var show = true



    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

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

    // The native methods implemented by the bibledit native library wrapper.
    external fun StringFromJNI(): String


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

}