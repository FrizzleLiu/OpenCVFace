package com.frizzle.opencvface;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;

import com.frizzle.opencvface.util.CameraHelper;
import com.frizzle.opencvface.util.Utils;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback,  Camera.PreviewCallback {

    private Button btnChange;
    private OpenCVJni openCVJni;
    private CameraHelper cameraHelper;
    private int cameraId = Camera.CameraInfo.CAMERA_FACING_FRONT;
    private SurfaceView surfaceView;
    private String filePath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_main);
        initView();
        requestPerms();
    }

    private void initView() {
        btnChange = findViewById(R.id.btn_change);
        surfaceView = findViewById(R.id.surface_view);
        surfaceView.getHolder().addCallback(this);
        btnChange.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (cameraHelper!=null){
                    cameraHelper.switchCamera();
                    cameraId = cameraHelper.getCameraId();
                }
            }
        });
        openCVJni=new OpenCVJni();
    }

    private void requestPerms() {
        //权限,简单处理下
        if (Build.VERSION.SDK_INT>Build.VERSION_CODES.N) {
            String[] perms= {Manifest.permission.CAMERA , Manifest.permission.WRITE_EXTERNAL_STORAGE};
            if (checkSelfPermission(perms[0]) == PackageManager.PERMISSION_DENIED || checkSelfPermission(perms[1]) == PackageManager.PERMISSION_DENIED ) {
                requestPermissions(perms,200);
            }else {
                initCamera();
            }
        } else {
            initCamera();
        }
    }

    private void initCamera() {
        Utils.copyAssets(this,"lbpcascade_frontalface.xml");
        filePath = new File(getCacheDir(), "lbpcascade_frontalface.xml").getAbsolutePath();
        openCVJni.init(filePath);
        surfaceView.getHolder().addCallback(this);
        cameraHelper = new CameraHelper(cameraId);
        cameraHelper.setPreviewCallback(this);
        cameraHelper.setPreviewDisplay(surfaceView.getHolder());
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        openCVJni.setSurface(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {

    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        openCVJni.postData(data, CameraHelper.WIDTH, CameraHelper.HEIGHT, cameraId);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (openCVJni!=null){
            openCVJni.init(filePath);
        }
        if (cameraHelper!=null){
            cameraHelper.startPreview();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (cameraHelper!=null){
            cameraHelper.stopPreview();
        }
        if (openCVJni!=null){
            openCVJni.release();
        }
    }
}
