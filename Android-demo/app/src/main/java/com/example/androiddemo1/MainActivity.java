package com.example.androiddemo1;

import androidx.appcompat.app.AppCompatActivity;

import android.app.ActivityManager;
import android.os.Bundle;
import android.util.Log;

import com.mlnkv.MLNKV;

import java.lang.reflect.Array;
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
        mlnkv.setInt32("key1", -11);
        mlnkv.setString( "key2", "value我发😁1");

        Log.e("MMMM", String.valueOf(mlnkv.getInt32("key1", 0)));
        Log.e("MMMM", mlnkv.getString("key2", ""));

        for (String ss : mlnkv.allKeys()) {
            Log.e("MMMM", ss);
        }

        String[]  sss = new String[] {"ni", "22"};

        mlnkv.setObject("testKey", sss);
//
        Log.e("MMMM", mlnkv.getObject("testKey", null).toString());

    }



}
