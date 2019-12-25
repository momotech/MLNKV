package com.example.androiddemo1;

import androidx.appcompat.app.AppCompatActivity;

import android.app.ActivityManager;
import android.os.Bundle;
import android.util.Log;

import com.mlnkv.MLNKV;

import java.util.HashMap;
import java.util.Map;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;


public class MainActivity extends AppCompatActivity {

    private MLNKV mlnkv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        long time = System.currentTimeMillis();

        MLNKV.initializeBasePath(this);
        mlnkv = MLNKV.defaultMLNKV();
        mlnkv.setInt32(-11, "key1");
        mlnkv.setString("value我发😁1", "key2");

        Log.e("MMMM", String.valueOf(mlnkv.getInt32("key1", 0)));
        Log.e("MMMM", mlnkv.getString("key2", ""));

        for (String ss : mlnkv.allKeys()) {
            Log.e("MMMM", ss);
        }

    }



}
