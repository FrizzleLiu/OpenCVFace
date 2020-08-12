package com.frizzle.opencvface;

import android.view.Surface;

/**
 * author: LWJ
 * date: 2020/8/10$
 * description
 */
public class OpenCVJni {
    static {
        System.loadLibrary("native-lib");
    }

    public native void init(String filePath);

    public native void postData(byte[] data, int windowWidth, int windowHeight, int cameraId);

    public native void setSurface(Surface surface) ;

    public native void release();
}
