package com.wwm.BeautyRendering;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.wwm.BeautyRendering.R;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        findViewById(R.id.btn_surface2).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, Camera2SurfaceActivity.class);
                intent.putExtra("btnNumber",1);
                startActivity(intent);
            }
        });
        findViewById(R.id.btn_gaussianBlur).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, Camera2SurfaceActivity.class);
                intent.putExtra("btnNumber",2);
                startActivity(intent);
            }
        });

        findViewById(R.id.btn_colorMap).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, Camera2SurfaceActivity.class);
                intent.putExtra("btnNumber",3);
                startActivity(intent);
            }
        });
        findViewById(R.id.btn_faceDetect).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, Camera2SurfaceActivity.class);
                intent.putExtra("btnNumber",4);
                startActivity(intent);
            }
        });

    }
}
