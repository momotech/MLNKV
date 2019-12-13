MLNKV
=======
[![License MIT](https://img.shields.io/badge/license-MIT-green.svg?style=flat)](https://github.com/momotech/MLNKV/blob/master/LICENSE)

MLNKV是基于mmap实现的高性能、跨平台的(支持iOS与Android)的Key-Value组件。
<br>

# 性能对比
![img](https://github.com/momotech/MLNKV/setString@2x.png)<br>
![img](https://github.com/momotech/MLNKV/setInt@2x.png)<br>
![img](https://github.com/momotech/MLNKV/getString@2x.png)



# 用法
### iOS 基本用法<br>
`pod 'MLNKV'`
```

// init
MLNKV *mlnkv = [MLNKV defaultMLNKV];
// NSString *path = [MLNKVDEFAULTPATH stringByAppendingPathComponent:@".test"];
// MLNKV *mlnkv = [MLNKV mlnkvWithPath:path];

// set
    [mlnkv setKVString:@"value" forKey:@"key1"];
    [mlnkv setKVBool:YES forKey:@"key2"];
    [mlnkv setKVInt32:66666 forKey:@"key3"];
    [mlnkv setKVInt64:88888888 forKey:@"key4"];
    [mlnkv setKVFloat:66.666 forKey:@"key5"];
    [mlnkv setKVDouble:8888888.888 forKey:@"key6"];
    [mlnkv setKVObject:@{@"key":@"value"} forKey:@"key7"];
    [mlnkv setKVData:data forKey:@"key8"];
    
    // get
    int value = [mlnkv getKVInt32ForKey:@"key3"];
    ...
    ...
    
    // obj 自己实现序列化 or 使用NSKeyedArchiver
    [mlnkv setKVObject:obj forKey:@"key" archiveBlock:^NSData * _Nullable(id  _Nonnull obj) {
       // ...archive
    }];
    [mlnkv getKVObjectForKey:@"key" ofClass:clz unarchiveBlock:^id _Nullable(NSData * _Nonnull data) {
       // ...unarchive
    }];

```
### Android 基本用法<br>

```

// must call this in MainActivity
 MLNKV.initializeBasePath(this);

// init
MLNKV mlnkv = MLNKV.defaultMLNKV();
// String path = MLNKV.basPath() + "/.test";
// MLNKV mlnkv = new MLNKV(path);

// set 
    mlnkv.setBool(true, "key1");
    mlnkv.setInt32(1, "key2");
    mlnkv.setInt64(88888888, "key3");
    mlnkv.setDouble(8888.888, "key4");
    mlnkv.setString("value", "key5");
    mlnkv.setBytes(bytes, "key6");

// get 
    boolean value = mlnkv.getBool("key1");
    ...
    ...

// obj 使用java Serializable
    mlnkv.setObject(obj, "key");
    mlnkv.getObject(obj, clz);

```








