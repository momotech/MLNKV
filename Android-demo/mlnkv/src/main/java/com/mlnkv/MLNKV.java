package com.mlnkv;

//import android.provider.ContactsContract;
//import android.util.Log;


import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

// String filePath = getFilesDir() + "/mlnkv/.kv";

public class MLNKV {

    public static final int MLNKVValueType_None   = 0;
    public static final int MLNKVValueType_Bool   = 2;
    public static final int MLNKVValueType_Float  = 3;
    public static final int MLNKVValueType_Int32  = 4;
    public static final int MLNKVValueType_Double = 5;
    public static final int MLNKVValueType_Int64  = 6;
    public static final int MLNKVValueType_String = 7;
    public static final int MLNKVValueType_Bytes  = 8;

    public static final String MLNKVTAG = "MLNKV";

    private long nativeMLNKVObj = -1;
    private MLNKVMemoryCache memoryCache;

    private static String BASE_PATH = null;
    public static void initializeBasePath(Context context) {
        if (context != null && context.getFilesDir() != null) {
            BASE_PATH = context.getFilesDir().getAbsolutePath() + "/mlnkv";
        }
    }

    /**
     * context.getFilesDir().getAbsolutePath() + "/mlnkv";
     * */
    public static final String basPath() {
        return BASE_PATH;
    }

    private static MLNKV defaultmlnkv = null;
    public static MLNKV defaultMLNKV() {
        if (defaultmlnkv != null) {
            return defaultmlnkv;
        }
        if (BASE_PATH == null) {
            throw new IllegalStateException("MLNKV BASE_PATH can't be nil, must be call initializeBasePath first ...");
        }else {
            defaultmlnkv = new MLNKV(BASE_PATH + "/.mlnkv");
            return defaultmlnkv;
        }
    }

    public MLNKV(String path) {
        if (init == true && path != null && path.length() > 0) {
            nativeMLNKVObj = initMLNKV(path);
            memoryCache = new MLNKVMemoryCache("mlnkv");
        }else {
            if (init == true) {
                Log.e(MLNKVTAG, "path can't nil ");
            }else  {
                Log.e(MLNKVTAG, "mlnkv System.loadLibrary error... ");
            }
            if (path == null || path.length() <= 0) {
                throw new IllegalStateException("MLNKV path can't be nil...");
            }
        }
    }

    @Override
    protected void finalize() throws Throwable {
        destoryMLNKV(nativeMLNKVObj);
        super.finalize();
    }

    public MLNKVMemoryCache getMemoryCache() {
        return this.memoryCache;
    }

    // set

    public boolean setObject(@NonNull Object value, @NonNull String key) {
        if (value == null) {
            return remove(key);
        }
        if (value instanceof String) {
            return setString(value.toString(), key);
        }else if (value instanceof Boolean) {
            return setBool((Boolean)value, key);
        }else if (value instanceof Integer) {
            return setInt32((Integer)value, key);
        }else if (value instanceof Long) {
            return setInt64((Long)value, key);
        }else if (value instanceof Float) {
            return setFloat((Float)value, key);
        }else if (value instanceof Double) {
            return setDouble((Double)value, key);
        }else {
            try {
                byte[] bytes = mlnkvBytes(value);
                if (bytes == null || bytes.length == 0) {
                    return remove(key);
                }
                if (setBytes(nativeMLNKVObj, bytes, bytes.length, key)) {
                    memoryCache.setObject(value, key, 0);
                    return true;
                }
            }catch (Exception e){
                e.printStackTrace();
            }
        }
        return false;
    }

    public boolean setBytes(@NonNull byte[] value, @NonNull String key) {
        if (value == null) {
            return false;
        }
        long size = value.length;
        return setBytes(nativeMLNKVObj, value, size, key);
    }

    public boolean setString(@NonNull String value, @NonNull String key) {
        return setString(nativeMLNKVObj, value, key);
    }

    public boolean setBool(boolean value, @NonNull String key) {
        return setBool(nativeMLNKVObj, value, key);
    }

    public boolean setInt32(int value, @NonNull String key) {
        return setInt32(nativeMLNKVObj, value, key);
    }

    public boolean setInt64(long value, @NonNull String key) {
        return setInt64(nativeMLNKVObj, value, key);
    }

    public boolean setFloat(float value, @NonNull String key) {
        return setFloat(nativeMLNKVObj, value, key);
    }

    public boolean setDouble(double value, @NonNull String key) {
        return setDouble(nativeMLNKVObj, value, key);
    }

    // get

    public Object getObject(@NonNull String key, Class clz) {

        Object obj_r = memoryCache.objectForKey(key);
        if (obj_r != null) {
            return obj_r;
        }

        if (clz != null) {
            if (clz.equals(String.class)) {
                return getString(key, null);
            }else if (clz.equals(Boolean.class)) {
                return getBool(key, false);
            }else if (clz.equals(Integer.class)) {
                return getInt32(key, 0);
            }else if (clz.equals(Long.class)) {
                return getInt64(key, 0);
            }else if (clz.equals(Float.class)) {
                return getFloat(key, 0);
            }else if (clz.equals(Double.class)) {
                return getDouble(key, 0);
            }
        }
        byte[] bytes = getBytes(key);
        try {
            Object obj = mlnkvObject(bytes);
            if (clz == null || obj.getClass().equals(clz)) {
                memoryCache.setObject(obj, key, 0);
                return obj;
            }
        }catch (Exception e){
            e.printStackTrace();
        }

        return null;
    }

    public byte[] getBytes(@NonNull String key) {
        return getBytes(nativeMLNKVObj, key);
    }

    public String getString(@NonNull String key) {
        return getString(key, null);
    }
    public String getString(@NonNull String key, String defaultValue) {
        String value = getString(nativeMLNKVObj, key);
        if (value != null) {
            return value;
        }
        return defaultValue;
    }

    public boolean getBool(@NonNull String key) {
        return getBool(key, false);
    }
    public boolean getBool(@NonNull String key, boolean defaultValue) {
        return getBool(nativeMLNKVObj, key, defaultValue);
    }

    public int getInt32(@NonNull String key) {
        return getInt32(key, 0);
    }
    public int getInt32(@NonNull String key, int defaultValue) {
        return getInt32(nativeMLNKVObj, key, defaultValue);
    }

    public long getInt64(@NonNull String key) {
        return getInt64(key, 0);
    }
    public long getInt64(@NonNull String key, long defaultValue) {
        return getInt64(nativeMLNKVObj, key, defaultValue);
    }

    public float getFloat(@NonNull String key) {
        return getFloat(key, 0);
    }
    public float getFloat(@NonNull String key, float defaultValue) {
        return getFloat(nativeMLNKVObj, key, defaultValue);
    }

    public double getDouble(@NonNull String key) {
        return getDouble(key, 0);
    }
    public double getDouble(@NonNull String key, double defaultValue) {
        return getDouble(nativeMLNKVObj, key, defaultValue);
    }

    // ...

    public int valueType(@NonNull String key) {
        if (key == null) {
            return MLNKVValueType_None;
        }
        return valueType(nativeMLNKVObj, key);
    }

    public long getValueSize(@NonNull String key) {
        if (key == null) {
            return 0;
        }
        return getVauleSize(nativeMLNKVObj, key);
    }

    public boolean containsKey(@NonNull String key) {
        return containsKey(nativeMLNKVObj, key);
    }

    public long count() {
        return count(nativeMLNKVObj);
    }

    public long totalUsedSize() {
        return totalUsedSize(nativeMLNKVObj);
    }

    public long fileSize() {
        return fileSize(nativeMLNKVObj);
    }

    public String filePath() {
        return filePath(nativeMLNKVObj);
    }

    public String[] allKeys() {
        return allKeys(nativeMLNKVObj);
    }

    public boolean remove(String key) {
        memoryCache.removeObjectForKey(key);
        return remove(nativeMLNKVObj, key);
    }

    public void clearAll() {
        memoryCache.removeAllObjects();
        clearAll(nativeMLNKVObj);
    }

    public void clearMemoryCache() {
        memoryCache.removeAllObjects();
    }

    public void trim() {
        trim(nativeMLNKVObj);
    }

    public void sync() {
        sync(nativeMLNKVObj, true);
    }

    public void async() {
        sync(nativeMLNKVObj, false);
    }



    // private
    public static byte[] mlnkvBytes(Object obj) throws Exception {
        if (obj == null) {
            return null;
        }
        byte[] bytes = null;
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        try {
            ObjectOutputStream oos = new ObjectOutputStream(bos);
            oos.writeObject(obj);
            oos.flush();
            bytes = bos.toByteArray();
            oos.close();
            bos.close();
        }catch (IOException e) {
            throw e;
        }
        return bytes;
    }

    public static Object mlnkvObject(byte[] bytes) throws Exception {
        if (bytes == null || bytes.length == 0) {
            return null;
        }
        Object obj = null;
        try {
            ByteArrayInputStream bis = new ByteArrayInputStream (bytes);
            ObjectInputStream ois = new ObjectInputStream (bis);
            obj = ois.readObject();
            ois.close();
            bis.close();
        } catch (IOException ex) {
            throw ex;
        }
        return obj;
    }

    // MLNKV C++ methods

    // init or destory
    private native long initMLNKV(String path);
    private native void destoryMLNKV(long nativeMLNKVPtr);

    // set
    private native boolean setBytes(long nativeMLNKVObj, byte[] value, long size, String key);

    private native boolean setString(long nativeMLNKVObj, String value, String key);

    private native boolean setBool(long nativeMLNKVObj, boolean value, String key);

    private native boolean setInt32(long nativeMLNKVObj, int value, String key);

    private native boolean setInt64(long nativeMLNKVObj, long value, String key);

    private native boolean setFloat(long nativeMLNKVObj, float value, String key);

    private native boolean setDouble(long nativeMLNKVObj, double value, String key);

    //get
    private native byte[] getBytes(long nativeMLNKVObj, String key);

    private native String getString(long nativeMLNKVObj, String key);

    private native boolean getBool(long nativeMLNKVObj, String key, boolean defaultValue);

    private native int getInt32(long nativeMLNKVObj, String key, int defaultValue);

    private native long getInt64(long nativeMLNKVObj, String key, long defaultValue);

    private native float getFloat(long nativeMLNKVObj, String key, float defaultValue);

    private native double getDouble(long nativeMLNKVObj, String key, double defaultValue);

    // other

    private native long getVauleSize(long nativeMLNKVObj, String key);

    private native int valueType(long nativeMLNKVObj, String key);

    private native boolean containsKey(long nativeMLNKVObj, String key);

    private native long count(long nativeMLNKVObj);

    private native long totalUsedSize(long nativeMLNKVObj);

    private native long fileSize(long nativeMLNKVObj);

    private native String filePath(long nativeMLNKVObj);

    private native String[] allKeys(long nativeMLNKVObj);

    private native boolean remove(long nativeMLNKVObj, String key);

    private native void clearAll(long nativeMLNKVObj);

    private native void trim(long nativeMLNKVObj);

    private native void sync(long nativeMLNKVObj, boolean isSync);

    private static boolean init = false;
    static  {
        try {
            System.loadLibrary("mlnkv");
            init = true;
        }catch (Exception e) {
            init = false;
            Log.e(MLNKVTAG, "mlnkv init error:", e);
        }
    }

}

