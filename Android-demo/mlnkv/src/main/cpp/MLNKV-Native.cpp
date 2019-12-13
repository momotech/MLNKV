//
//

#include "com_mlnkv_MLNKV.h"
#include <iostream>
#include <android/log.h>

#include "MLNKVBase.h"

#define MLNKV_LOG_TAG "MLNKV"
#define MLNKV_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MLNKV_LOG_TAG, __VA_ARGS__)
#define MLNKV_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MLNKV_LOG_TAG, __VA_ARGS__)
#define MLNKV_BRIDGE(method) ((MLNKVBase *)mlnkv)->method
#define MLNKV_JSTRING_TO_STRING(value) jstringTostring(env, value)
#define MLNKV_STRING_TO_JSTRING(value) stringTojstring(env, value)
#define MLNKV_EXIST_MLNKV(result) if (mlnkv == -1 || !(MLNKVBase *)mlnkv) {\
MLNKV_LOGE("mlnkv obj is nil...");\
return result;\
}

#define MLNKV_SETKEY_RETURN(result) if (!key) {\
MLNKV_LOGE("key can't be nil...");\
return result;\
}\
MLNKV_EXIST_MLNKV(result)

//#define MLNKV_SETKEY_RETURN(result)
//#define MLNKV_EXIST_MLNKV(result)

using namespace std;

namespace mkv {
//    static MLNKVBase *getmlnbaseobj(JNIEnv *env, jobject obj) {
//        jclass objClz = env->GetObjectClass(obj);
//        if (objClz == nullptr) {
//            return nullptr;
//        }
//        jfieldID fid = env->GetFieldID(objClz, "nativeMLNKVPtr", "J");
//        if (fid == nullptr) {
//            LOGE("env->GetFieldID error");
//            return nullptr;
//        }
//        jlong ptr = env->GetLongField(obj, fid);
//        return (MLNKVBase *) ptr;
//    }

    static string jstringTostring(JNIEnv *env, jstring str) {
        if (str) {
            const char *kstr = env->GetStringUTFChars(str, nullptr);
            if (kstr) {
                string result(kstr);
                env->ReleaseStringUTFChars(str, kstr);
                return result;
            }
        }
        return "";
    }

    static jstring stringTojstring(JNIEnv *env, const string &str) {
        return env->NewStringUTF(str.c_str());
    }

    static jobjectArray vectorTojarray(JNIEnv *env, const vector<string> &arr) {
        if (!arr.empty()) {
            jobjectArray result =
                    env->NewObjectArray(arr.size(), env->FindClass("java/lang/String"), nullptr);
            if (result) {
                for (size_t index = 0; index < arr.size(); index++) {
                    jstring value = mkv::stringTojstring(env, arr[index]);
                    env->SetObjectArrayElement(result, index, value);
                    env->DeleteLocalRef(value);
                }
            }
            return result;
        }
        return nullptr;
    }
}

using namespace mkv;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL Java_com_mlnkv_MLNKV_initMLNKV
        (JNIEnv *env, jobject obj, jstring path) {
    const char *filePath = env->GetStringUTFChars(path, nullptr);
    MLNKVBase *mlnkv = new MLNKVBase(filePath);
    env->ReleaseStringUTFChars(path, filePath);
    return (jlong)mlnkv;
}

JNIEXPORT void JNICALL Java_com_mlnkv_MLNKV_destoryMLNKV
        (JNIEnv *env, jobject obj, jlong ptr) {
    if (ptr == -1) {
        return;
    }
    delete ((MLNKVBase *)ptr);
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_setBytes
        (JNIEnv *env, jobject obj, jlong mlnkv, jbyteArray bytes, jlong length, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    jbyte *value = env->GetByteArrayElements(bytes, nullptr);
    jboolean result = (jboolean)MLNKV_BRIDGE(setBytes(value, (const size_t)length, MLNKV_JSTRING_TO_STRING(key)));
    if (value) {
        env->ReleaseByteArrayElements(bytes, value, 0);
    }
    return result;
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_setString
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring value, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    if (!value) {
        return (jboolean)MLNKV_BRIDGE(remove(MLNKV_JSTRING_TO_STRING(key)));
    }
    return (jboolean)MLNKV_BRIDGE(setString(MLNKV_JSTRING_TO_STRING(value), MLNKV_JSTRING_TO_STRING(key)));
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_setBool
        (JNIEnv *env, jobject obj, jlong mlnkv, jboolean value, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    return (jboolean)MLNKV_BRIDGE(setBool(value, MLNKV_JSTRING_TO_STRING(key)));
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_setInt32
        (JNIEnv *env, jobject obj, jlong mlnkv, jint value, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    return (jboolean)MLNKV_BRIDGE(setInt32(value, MLNKV_JSTRING_TO_STRING(key)));
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_setInt64
        (JNIEnv *env, jobject obj, jlong mlnkv, jlong value, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    return (jboolean)MLNKV_BRIDGE(setInt64(value, MLNKV_JSTRING_TO_STRING(key)));
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_setFloat
        (JNIEnv *env, jobject obj, jlong mlnkv, jfloat value, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    return (jboolean)MLNKV_BRIDGE(setFloat(value, MLNKV_JSTRING_TO_STRING(key)));
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_setDouble
        (JNIEnv *env, jobject obj, jlong mlnkv, jdouble value, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    return (jboolean)MLNKV_BRIDGE(setDouble(value, MLNKV_JSTRING_TO_STRING(key)));
}

JNIEXPORT jbyteArray JNICALL Java_com_mlnkv_MLNKV_getBytes
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key) {
    MLNKV_SETKEY_RETURN(nullptr);
    void *value;
    size_t length = 0;
    if (MLNKV_BRIDGE(getBytesForKey(MLNKV_JSTRING_TO_STRING(key), value, length)) && length > 0) {
        jbyteArray bytes = env->NewByteArray(length);
        env->SetByteArrayRegion(bytes, 0, length, (jbyte *)value);
        return bytes;
    }
    return nullptr;
}

JNIEXPORT jstring JNICALL Java_com_mlnkv_MLNKV_getString
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key) {
    MLNKV_SETKEY_RETURN(nullptr);
    string value;
    if (MLNKV_BRIDGE(getStringForKey(MLNKV_JSTRING_TO_STRING(key), value))) {
        return MLNKV_STRING_TO_JSTRING(value);
    }
    return nullptr;
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_getBool
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key, jboolean defaultValue) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    return (jboolean)MLNKV_BRIDGE(getBoolForKey(MLNKV_JSTRING_TO_STRING(key),defaultValue));
}

JNIEXPORT jint JNICALL Java_com_mlnkv_MLNKV_getInt32
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key, jint defaultValue) {
    MLNKV_SETKEY_RETURN(0);
    return (jint)MLNKV_BRIDGE(getInt32ForKey(MLNKV_JSTRING_TO_STRING(key), defaultValue));
}

JNIEXPORT jlong JNICALL Java_com_mlnkv_MLNKV_getInt64
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key, jlong defaultValue) {
    MLNKV_SETKEY_RETURN(0);
    return (jlong)MLNKV_BRIDGE(getInt64ForKey(MLNKV_JSTRING_TO_STRING(key), defaultValue));
}

JNIEXPORT jfloat JNICALL Java_com_mlnkv_MLNKV_getFloat
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key, jfloat defaultValue) {
    MLNKV_SETKEY_RETURN(0);
    return (jfloat)MLNKV_BRIDGE(getFloatForKey(MLNKV_JSTRING_TO_STRING(key), defaultValue));
}

JNIEXPORT jdouble JNICALL Java_com_mlnkv_MLNKV_getDouble
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key, jdouble defaultValue) {
    MLNKV_SETKEY_RETURN(0);
    return (jdouble)MLNKV_BRIDGE(getDoubleForKey(MLNKV_JSTRING_TO_STRING(key), defaultValue));
}

JNIEXPORT jint JNICALL Java_com_mlnkv_MLNKV_valueType
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key) {
    MLNKV_SETKEY_RETURN(0);
    MLNKVValueType valueType = MLNKVValueType_None;
    MLNKV_BRIDGE(getValueSizeForKey(MLNKV_JSTRING_TO_STRING(key), valueType));
    return (jint)valueType;
}

JNIEXPORT jlong JNICALL Java_com_mlnkv_MLNKV_getVauleSize
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key) {
    MLNKV_SETKEY_RETURN(0);
    MLNKVValueType type = MLNKVValueType_None;
    return (jlong)MLNKV_BRIDGE(getValueSizeForKey(MLNKV_JSTRING_TO_STRING(key), type));
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_containsKey
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean)false);
    return (jboolean)MLNKV_BRIDGE(containsKey(MLNKV_JSTRING_TO_STRING(key)));
}

JNIEXPORT jlong JNICALL Java_com_mlnkv_MLNKV_count
        (JNIEnv *env, jobject obj, jlong mlnkv) {
    MLNKV_EXIST_MLNKV(0);
    return (jlong)MLNKV_BRIDGE(count());
}

JNIEXPORT jlong JNICALL Java_com_mlnkv_MLNKV_totalUsedSize
        (JNIEnv *env, jobject obj, jlong mlnkv) {
    MLNKV_EXIST_MLNKV(0);
    return (jlong)MLNKV_BRIDGE(totalUsedSize());
}

JNIEXPORT jlong JNICALL Java_com_mlnkv_MLNKV_fileSize
        (JNIEnv *env, jobject obj, jlong mlnkv) {
    MLNKV_EXIST_MLNKV(0);
    return (jlong)MLNKV_BRIDGE(getFileSize());
}

JNIEXPORT jstring JNICALL Java_com_mlnkv_MLNKV_filePath
        (JNIEnv *env, jobject obj, jlong mlnkv) {
    MLNKV_EXIST_MLNKV(nullptr);
    string filePath = MLNKV_BRIDGE(getFilePath());
    return MLNKV_STRING_TO_JSTRING(filePath);
}

JNIEXPORT jobjectArray JNICALL Java_com_mlnkv_MLNKV_allKeys
        (JNIEnv *env, jobject obj, jlong mlnkv) {
    MLNKV_EXIST_MLNKV(nullptr);
    vector<string> keys = MLNKV_BRIDGE(allKeys());
    return vectorTojarray(env, keys);
}

JNIEXPORT jboolean JNICALL Java_com_mlnkv_MLNKV_remove
        (JNIEnv *env, jobject obj, jlong mlnkv, jstring key) {
    MLNKV_SETKEY_RETURN((jboolean) false);
    return (jboolean)MLNKV_BRIDGE(remove(MLNKV_JSTRING_TO_STRING(key)));
}

JNIEXPORT void JNICALL Java_com_mlnkv_MLNKV_clearAll
        (JNIEnv *env, jobject obj, jlong mlnkv) {
    MLNKV_EXIST_MLNKV();
    MLNKV_BRIDGE(clearAll());
}

JNIEXPORT void JNICALL Java_com_mlnkv_MLNKV_trim
        (JNIEnv *env, jobject obj, jlong mlnkv) {
    MLNKV_EXIST_MLNKV();
    MLNKV_BRIDGE(trim());
}

JNIEXPORT void JNICALL Java_com_mlnkv_MLNKV_sync
        (JNIEnv *env, jobject obj, jlong mlnkv, jboolean isSync) {
    MLNKV_EXIST_MLNKV();
    MLNKV_BRIDGE(sync(isSync));
}


#ifdef __cplusplus
}
#endif