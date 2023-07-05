package com.zz.example;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.zz.example.databinding.ActivityMainBinding;

import java.io.ByteArrayOutputStream;

public class MainActivity extends AppCompatActivity {

    private ImageView imageView;
    // Used to load the 'example' library on application startup.
    static {
        System.loadLibrary("example");
    }

    private ActivityMainBinding binding;




    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        imageView = findViewById(R.id.imageView);

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());
    }

    public void startServiceClick(View v){
        Log.i("FFMPEG-LOG", "startServiceClick");
        testYuv420Show();
        new Thread(){
            @Override
            public void run(){
                startPlayJNI();
            }
        }.start();
    }

    public void ffmpegYuv420Call(int width,int height,byte[] yuvData){
        try {
            Log.i("FFMPEG-LOG", "ffmpegYuv420Call "+width+" "+height+" "+yuvData.length);

            YuvImage yuvImage = new YuvImage(yuvData, ImageFormat.NV21, width, height, null);//NV21
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            yuvImage.compressToJpeg(new Rect(0, 0, width, height), 100, outputStream);
            byte[] jpegData = outputStream.toByteArray();


            Bitmap bitmap = BitmapFactory.decodeByteArray(jpegData, 0, jpegData.length);
            Log.i("FFMPEG-LOG", "setImageBitmap ");

            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    imageView.setImageBitmap(bitmap);
                }
            });

        } catch (Exception e) {
            Log.e("FFMPEG-LOG", "Error decoding bitmap from byte array: " + e.getMessage());
        }
    }

    private void testYuv420Show(){
        try {
            int width = 1920;
            int height = 1080;
            byte[] yuvData = new byte[width*height+width*height/2];

            for (int i = 0; i < width*height+width*height/2;i++) {
                int pix = i%255;
                yuvData[i] = (byte)pix;
            }

            YuvImage yuvImage = new YuvImage(yuvData, ImageFormat.NV21, width, height, null);
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            yuvImage.compressToJpeg(new Rect(0, 0, width, height), 100, outputStream);
            byte[] jpegData = outputStream.toByteArray();

            Bitmap bitmap = BitmapFactory.decodeByteArray(jpegData, 0, jpegData.length);
            //imageView.setImageBitmap(bitmap);

        } catch (Exception e) {
            Log.e("YUV420", "Error decoding bitmap from byte array: " + e.getMessage());
        }
    }

    /**
     * A native method that is implemented by the 'example' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();


    public native String startPlayJNI();
}