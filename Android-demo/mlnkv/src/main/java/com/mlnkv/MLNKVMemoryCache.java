package com.mlnkv;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;
import java.util.HashMap;

class MLNKVCacheNode {
    WeakReference<MLNKVCacheNode> prev;
    WeakReference<MLNKVCacheNode> next;
    String key;
    Object value;
    long size;
}

class MLNKVCache {
    private MLNKVCacheNode head;
    private MLNKVCacheNode tail;
    private HashMap<String, MLNKVCacheNode> dict;

    public int totalCount() {
        return dict.size();
    }
    public long totalSize;
    public boolean releaseOnMainThread;

    public MLNKVCache() {
        dict = new HashMap<String, MLNKVCacheNode>();
    }

    @Override
    protected void finalize() throws Throwable {
        dict.clear();
        super.finalize();
    }

    public MLNKVCacheNode nodeForKey(String key) {
        return dict.get(key);
    }

    public void insertNodeAtHead(MLNKVCacheNode node){
        if (node == null) return;
        dict.put(node.key, node);
        totalSize += node.size;
        if (head != null) {
            node.next = new WeakReference<MLNKVCacheNode>(head);
            head.prev = new WeakReference<MLNKVCacheNode>(node);
            head = node;
        }else {
            head = tail = node;
        }
    }

    public void bringNodeToHead(MLNKVCacheNode node) {
        if (node == null) return;
        if (head == node) return;
        if (tail == node) {
            tail = node.prev.get();
            tail.next = null;
        }else {
            node.next.get().prev = node.prev;
            node.prev.get().next = node.next;
        }
        node.next = new WeakReference<MLNKVCacheNode>(head);
        node.prev = null;
        head.prev = new WeakReference<MLNKVCacheNode>(node);
        head = node;

    }

    public void removeNode(MLNKVCacheNode node) {
        if (node == null) return;
        dict.remove(node.key);
        totalSize -= node.size;
        if (node.next != null) node.next.get().prev = node.prev;
        if (node.prev != null) node.prev.get().next = node.next;
        if (head == node) head = (node.next == null)? null: node.next.get();
        if (tail == node) tail = (node.prev == null)? null: node.prev.get();

        if (releaseOnMainThread) {
            if (Looper.getMainLooper() != Looper.myLooper()) {
                Message message = new Message();
                message.what = 2;
                message.obj = node;
                uiHandler.sendMessage(message);
            }
        }
    }

    public MLNKVCacheNode removeTailNode() {
        if (tail == null) return null;
        MLNKVCacheNode tempTail = tail;
        dict.remove(tail.key);
        totalSize -= tail.size;
        if (head == tail) {
            head = tail = null;
        }else  {
            tail = tail.prev.get();
            tail.next = null;
        }
        if (releaseOnMainThread) {
            if (Looper.getMainLooper() != Looper.myLooper()) {
                Message message = new Message();
                message.what = 2;
                message.obj = tempTail;
                uiHandler.sendMessage(message);
            }
        }
        return tempTail;
    }

    public void removeAll() {
        totalSize = 0;
        head = null;
        tail = null;
        if (dict.size() > 0 && releaseOnMainThread) {
            if (Looper.getMainLooper() != Looper.myLooper()) {
                Message message = new Message();
                message.what = 1;
                message.obj = dict.clone();
                uiHandler.sendMessage(message);
            }
        }
        dict.clear();
    }

    private Handler uiHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case 1:
                    HashMap nodes = (HashMap) msg.obj;
                    nodes.clear();
                    msg.obj = null;
                    break;
                case 2:
                    msg.obj = null;
                    break;

                default:
                     break;
            }
        }
    };
}

public class MLNKVMemoryCache {

    public String name;

    public int LRUCountLimit; // 10000  if == 0 don't use LRU cache
    public int FIFOCountLimit; // 1000  if == 0 don't use FIFO cache
    public long memorySizeLimit; // 100M 100 << 20

    public boolean useFIFO; //  default YES use FIFO cache
    public boolean useLRU; // default YES  use LRU cache

    public MLNKVMemoryCache(String name) {
        this.name = name;
        LRUCountLimit = 10000;
        FIFOCountLimit = 1000;
        memorySizeLimit = 100 << 20;
        releaseOnMainThread = false;

        useFIFO = true;
        useLRU = true;

        weakMap = new HashMap<>();
        LRUCache = new MLNKVCache();
        FIFOCache = new MLNKVCache();

        LRUCache.releaseOnMainThread = releaseOnMainThread;
        FIFOCache.releaseOnMainThread = releaseOnMainThread;
    }

    @Override
    protected void finalize() throws Throwable {
        LRUCache.removeAll();
        FIFOCache.removeAll();
        weakMap.clear();
        super.finalize();
    }

    public long getTotoalCount() {
        synchronized (this) {
            return FIFOCache.totalCount() + LRUCache.totalCount();
        }
    }

    public long getTotoalMemorySize() {
        synchronized (this) {
            return FIFOCache.totalSize + LRUCache.totalSize;
        }
    }

    public void setReleaseOnMainThread(boolean releaseOnMainThread) {
        synchronized (this) {
            this.releaseOnMainThread = releaseOnMainThread;
            FIFOCache.releaseOnMainThread = releaseOnMainThread;
            LRUCache.releaseOnMainThread = releaseOnMainThread;
        }
    }

    public boolean isReleaseOnMainThread() {
        return this.releaseOnMainThread;
    }

    public boolean containsObjectForKey(String key) {
        if (key == null) return false;
        boolean contains = false;
        synchronized (this) {
            contains = weakMap.containsKey(key) || FIFOCache.nodeForKey(key) != null || LRUCache.nodeForKey(key) != null;
        }
        return contains;
    }

    public Object objectForKey(String key) {
        if (key == null) return null;
        synchronized (this) {
            MLNKVCacheNode node = (useFIFO == true && FIFOCountLimit > 0)? FIFOCache.nodeForKey(key): null;
            if (node != null) {
                FIFOCache.removeNode(node);
                if (useLRU == true && LRUCountLimit > 0) {
                    LRUCache.insertNodeAtHead(node);
                }
            }else  {
                if (useLRU == true && LRUCountLimit > 0) {
                    node = LRUCache.nodeForKey(key);
                    LRUCache.bringNodeToHead(node);
                }
            }
            if (useLRU == true && LRUCache.totalCount() > LRUCountLimit) {
                LRUCache.removeTailNode();
            }
            Object value = null;
            if (node != null) {
                value = node.value;
            }else  {
                WeakReference reference = weakMap.get(key);
                if (reference != null) {
                    value = reference.get();
                    if (value == null) {
                        weakMap.remove(key);
                    }
                }
            }
            return value;
        }
    }

    public void setObject(Object obj, String key, long size) {
        if (key == null) return;
        if (obj == null) {
            removeObjectForKey(key);
            return;
        }

       synchronized (this) {
           weakMap.put(key, new WeakReference<Object>(obj));
           MLNKVCacheNode node = (useLRU == true && LRUCountLimit > 0)? LRUCache.nodeForKey(key): null;
           if (node != null) {
               LRUCache.totalSize = (LRUCache.totalSize < node.size)? 0: (LRUCache.totalSize - node.size);
               LRUCache.totalSize += size;
               node.value = obj;
               node.size = size;
               LRUCache.bringNodeToHead(node);
           }else {
               if (useFIFO == true && FIFOCountLimit > 0) {
                   node = FIFOCache.nodeForKey(key);
                   if (node != null) {
                       FIFOCache.totalSize = (FIFOCache.totalSize < node.size)? 0: (FIFOCache.totalSize - node.size);
                       FIFOCache.totalSize += size;
                       node.value = obj;
                       node.size = size;
                       FIFOCache.bringNodeToHead(node);
                   }else {
                       node = new MLNKVCacheNode();
                       node.key = key;
                       node.value = obj;
                       node.size = size;
                       FIFOCache.insertNodeAtHead(node);
                   }
                   if (FIFOCache.totalCount() > FIFOCountLimit) {
                       FIFOCache.removeTailNode();
                   }
               }
           }
           while (FIFOCache.totalSize + LRUCache.totalSize > memorySizeLimit) {
               if (LRUCache.removeTailNode() == null) {
                   if (FIFOCache.removeTailNode() == null) {
                       break;
                   }
               }
           }
       }
    }

    public void setWeakObject(Object obj, String key) {
        if (key == null) return;
        synchronized (this) {
            if (obj == null) {
                weakMap.remove(key);
                return;
            }
            weakMap.put(key, new WeakReference<Object>(obj));
        }
    }

    public void removeObjectForKey(String key) {
        if (key == null) return;
        synchronized (this) {
            if (useFIFO == true && FIFOCountLimit > 0) {
                FIFOCache.removeNode(FIFOCache.nodeForKey(key));
            }
            if (useLRU == true && LRUCountLimit > 0) {
                LRUCache.removeNode(LRUCache.nodeForKey(key));
            }
            weakMap.remove(key);
        }
    }

    public void removeAllObjects() {
        synchronized (this) {
            FIFOCache.removeAll();
            LRUCache.removeAll();
            weakMap.clear();
        }
    }

    // private
    private boolean releaseOnMainThread;
    private HashMap<String, WeakReference<Object>> weakMap;
    private MLNKVCache FIFOCache;
    private MLNKVCache LRUCache;

}
