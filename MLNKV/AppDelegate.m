//
//  AppDelegate.m
//  MLNKV
//
//  Copyright © 2019 . All rights reserved.
//

#import "AppDelegate.h"
#import "MLNKV.h"

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
//    [[MLNKV defaultMLNKV] setKVString:@"试数据" forKey:@"key1"];
//    NSLog(@"%@", [[MLNKV defaultMLNKV] getKVStringForKey:@"key1" defaultValue:nil]);
//    
//    [[MLNKV defaultMLNKV] setKVString:@"测试你好(*￣︶￣)😊" forKey:@"key2"];
//    NSLog(@"%@", [[MLNKV defaultMLNKV] getKVStringForKey:@"key2" defaultValue:nil]);
//    
//    MLNKV *mlnkv = [MLNKV defaultMLNKV];
//    
//    // set
//    [mlnkv setKVString:@"value" forKey:@"key1"];
//    [mlnkv setKVBool:YES forKey:@"key2"];
//    [mlnkv setKVInt32:66666 forKey:@"key3"];
//    [mlnkv setKVInt64:88888888 forKey:@"key4"];
//    [mlnkv setKVFloat:66.666 forKey:@"key5"];
//    [mlnkv setKVDouble:8888888.888 forKey:@"key6"];
//    [mlnkv setKVObject:@{@"key":@"value"} forKey:@"key7"];
//    
//    // get
//    int value = [mlnkv getKVInt32ForKey:@"key3"];
//    ...
//    ...
//    
//    // obj
//    [mlnkv setKVObject:obj forKey:@"key" archiveBlock:^NSData * _Nullable(id  _Nonnull obj) {
//       // ...archive
//    }];
//    [mlnkv getKVObjectForKey:@"key" ofClass:clz unarchiveBlock:^id _Nullable(NSData * _Nonnull data) {
//       // ...unarchive
//    }];
//    
//    NSLog(@"---%f", [mlnkv getKVFloatForKey:@"key5"]);
//    NSLog(@"---%f", [mlnkv getKVDoubleForKey:@"key6"]);
    
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    
    [application beginBackgroundTaskWithExpirationHandler:^{
        
    }];
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}




@end
