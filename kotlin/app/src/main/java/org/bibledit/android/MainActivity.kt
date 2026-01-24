package org.bibledit.android

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import org.bibledit.android.databinding.ActivityMainBinding
import java.util.Timer
import kotlin.concurrent.schedule

class MainActivity : AppCompatActivity() {

    val timer = Timer()


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)

        timer.schedule(2000L, 5000L) {
            onRepeatingTimeout()
        }
    }

    // The native methods implemented by the bibledit native library.
    external fun stringFromJNI(): String

    companion object {
        // Load the bibledit kernel library on application startup.
        init {
            System.loadLibrary("bibledit")
        }
    }

    fun onRepeatingTimeout ()
    {
        // Modifying widgets must be done on the UI thread.
        runOnUiThread(Runnable {
            var textview : TextView = this.findViewById(R.id.sample_text)
            textview.text = stringFromJNI()
        })

    }
}