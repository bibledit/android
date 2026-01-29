package org.bibledit.android

import android.net.Uri
import android.webkit.WebView
import android.webkit.WebViewClient


class MyWebViewClient : WebViewClient() {

    override fun shouldOverrideUrlLoading(view: WebView?, url: String?): Boolean {
        if (Uri.parse(url).host == "www.example.com") {
            // This is your website, so don't override. Let your WebView load
            // the page.
            return false
        }
        // Don't override URL loading.
        return true
    }
}