package org.bibledit.android

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import org.bibledit.android.databinding.ActivityMainBinding
import java.util.Timer
import kotlin.concurrent.schedule

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    val timer = Timer()
    var counter = 0


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()

        timer.schedule(2000L, 5000L) {
            onRepeatingTimeout()
        }
    }

    // A native method that is implemented by the bibledit native library,
    // which is packaged with this application.
    external fun stringFromJNI(): String

    companion object {
        // Load the bibledit kernel library on application startup.
        init {
            System.loadLibrary("bibledit")
        }
    }

    fun onRepeatingTimeout ()
    {
        counter++
        println("Timer tick " + counter.toString())
    }
}