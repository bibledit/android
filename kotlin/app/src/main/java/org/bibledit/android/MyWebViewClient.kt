package org.bibledit.android

import android.net.Uri
import android.net.http.SslError
import android.webkit.SslErrorHandler
import android.webkit.WebView
import android.webkit.WebViewClient


class MyWebViewClient : WebViewClient() {

    override fun shouldOverrideUrlLoading(view: WebView?, url: String?): Boolean
    {
        if (Uri.parse(url).host == "www.example.com") {
            // This is your website, so don't override. Let your WebView load
            // the page.
            return false
        }
        // Don't override URL loading.
        return false
    }

    override fun onReceivedSslError(view: WebView?, handler: SslErrorHandler, error: SslError) // Todo test probably not needed anymore?
    {
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
        val issuedToName = error.getCertificate().getIssuedTo().getCName()
        val issuedToGood = (issuedToName.indexOf("localhost.daplie.com") == 0)
        // Check 2: The URL is localhost at a known port.
        // That can only be this app's embedded webserver.
        val url = error.getUrl()
        val urlGood = url.indexOf("https://localhost:8081") == 0
        // Check 3: The certificate has been issued by a specific known name.
        val issuedByName = error.getCertificate().getIssuedBy().getCName()
        val issuedByGood = issuedByName.indexOf("RapidSSL SHA256 CA") == 0
        // If all checks passed, we're 100% sure this is our known certificate, so it's secure.
        val proceed = (issuedToGood && urlGood && issuedByGood)
        // Take decision what to do for full security.
        if (proceed) handler.proceed()
        else handler.cancel()
    }


}