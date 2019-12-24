//
//  BaseViewController.m
//  MLNKV
//
//  Copyright © 2019. All rights reserved.
//

#import "BaseViewController.h"
#import "MLNKV.h"
#import <mach/mach.h>
#import <sys/sysctl.h>


@interface BaseViewController ()
@property (weak, nonatomic) IBOutlet UISegmentedControl *SoureTypeSegmented;
@property (weak, nonatomic) IBOutlet UITextField *numTextField;
@property (weak, nonatomic) IBOutlet UITextField *lengthTextField;
@property (weak, nonatomic) IBOutlet UISegmentedControl *typeSegmented;

@property (weak, nonatomic) IBOutlet UILabel *setActionTimeLabel;
@property (weak, nonatomic) IBOutlet UILabel *getActionTimeLabel;
@property (weak, nonatomic) IBOutlet UILabel *removeActionTimeLabel;
@property (weak, nonatomic) IBOutlet UILabel *clearActionTimeLabel;
@property (weak, nonatomic) IBOutlet UILabel *trimTimeLabel;
@property (weak, nonatomic) IBOutlet UILabel *syncActionTimeLabel;

@property (weak, nonatomic) IBOutlet UILabel *setupTimeLabel;
@property (weak, nonatomic) IBOutlet UILabel *saveCountLabel;

@property (nonatomic, strong) NSArray<NSString *> *preArray;

@property (nonatomic, strong) NSMutableArray<NSString *> *keysArray;
@property (nonatomic, strong) NSMutableArray *valuesArray;
@property (nonatomic, strong) NSMutableArray<NSString *> *hasSetKeys;

@property (nonatomic, strong) MLNKV *mlnkv;

@end

@implementation BaseViewController

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [super touchesEnded:touches withEvent:event];
    
    [self.view endEditing:YES];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    _preArray = @[@"mlnkv"];
    
    _keysArray = NSMutableArray.new;
    _valuesArray = NSMutableArray.new;
    _hasSetKeys = NSMutableArray.new;
    
    [self mlnkv];
}

- (double)usedMemory {
    int64_t memoryUsageInByte = 0;
    task_vm_info_data_t vmInfo;
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    kern_return_t kernelReturn = task_info(mach_task_self(), TASK_VM_INFO, (task_info_t) &vmInfo, &count);
    if(kernelReturn == KERN_SUCCESS) {
        memoryUsageInByte = (int64_t) vmInfo.phys_footprint;
//        NSLog(@"Memory in use (in bytes): %lld", memoryUsageInByte);
    } else {
//        NSLog(@"Error with task_info(): %s", mach_error_string(kernelReturn));
    }
    return memoryUsageInByte / 1024.0 / 1024.0;
}

- (MLNKV *)mlnkv {
    if (!_mlnkv) {
        NSLog(@"initmlnkv-memory-b:%f", [self usedMemory]);
        CFAbsoluteTime begin = CFAbsoluteTimeGetCurrent();
        _mlnkv = [MLNKV defaultMLNKV];
        CFAbsoluteTime space = CFAbsoluteTimeGetCurrent() - begin;
        self.setupTimeLabel.text = [NSString stringWithFormat:@"启动耗时：%f ms", space * 1000];
        self.saveCountLabel.text = [NSString stringWithFormat:@"数据存储量：%ld", _mlnkv.count];
        NSLog(@">>启动耗时：%f", space);
        NSLog(@"initmlnkv-memory-e:%f", [self usedMemory]);
    }
    return _mlnkv;
}


#pragma mark - action

- (IBAction)SourceSegmentedAction:(UISegmentedControl *)sender {
    [self valueTypeSegmentedAction:nil];
    if (sender.selectedSegmentIndex == 0) {
        [self mlnkv];
    }
}

- (IBAction)valueTypeSegmentedAction:(UISegmentedControl *)sender {
    self.setActionTimeLabel.text = @"耗时：";
    self.getActionTimeLabel.text = @"耗时：";
    self.removeActionTimeLabel.text = @"耗时：";
    self.clearActionTimeLabel.text = @"耗时：";
    self.trimTimeLabel.text = @"耗时：";
    self.syncActionTimeLabel.text = @"耗时：";
    self.setupTimeLabel.text = @"启动耗时";;
    self.saveCountLabel.text = @"数据存储量";
}
- (IBAction)reloadDataAction:(UIButton *)sender {
    @autoreleasepool {
        [_valuesArray removeAllObjects];
        [_keysArray removeAllObjects];
    }
    
    [self.view endEditing:YES];
    
    NSUInteger count = self.numTextField.text.integerValue ? : 100000;
    NSUInteger length = self.lengthTextField.text.integerValue? : 10;
    NSInteger selected = self.typeSegmented.selectedSegmentIndex;
    [sender setTitle:@"生成中..." forState:UIControlStateDisabled];
    sender.enabled = NO;
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        NSLog(@"reload-memory-b:%f", [self usedMemory]);
        @autoreleasepool {
            for (int i = 0; i < count; i++) {
                if (selected == 0) {
                    int len = arc4random() % length +1;
                    NSMutableString *st = [NSMutableString stringWithCapacity:len];
                    for (int s = 0; s < length; s++) {
                        [st appendFormat:@"%c", arc4random()%30 + 'A'];
                    }
                    if (st.length <= 0) {
                        continue;
                    }
                    [self.valuesArray addObject:st];
                }else if (selected == 1) {
                    [self.valuesArray addObject:@(arc4random() % length +1)];
                }
                
//                int key = arc4random() % 100000;
                [self.keysArray addObject:[NSString stringWithFormat:@"p%d", i]];
            }
        }
        NSLog(@"reload-memory-e:%f", [self usedMemory]);
        dispatch_async(dispatch_get_main_queue(), ^{
            sender.enabled = YES;
        });
    });
}


- (IBAction)setValueAction:(id)sender {
    NSString *cmd = [self.preArray objectAtIndex:0];
    cmd = [cmd stringByAppendingString:@"SetValueAction"];
    [self performSelector:NSSelectorFromString(cmd)];
}

- (IBAction)getValueAction:(id)sender {
    NSString *cmd = [self.preArray objectAtIndex:0];
    cmd = [cmd stringByAppendingString:@"GetValueAction"];
    [self performSelector:NSSelectorFromString(cmd)];
}

- (IBAction)removeAction:(id)sender {
    NSString *cmd = [self.preArray objectAtIndex:0];
    cmd = [cmd stringByAppendingString:@"RemoveAction"];
    [self performSelector:NSSelectorFromString(cmd)];
}

- (IBAction)clearAction:(id)sender {
    NSString *cmd = [self.preArray objectAtIndex:0];
    cmd = [cmd stringByAppendingString:@"ClearAction"];
    [self performSelector:NSSelectorFromString(cmd)];
}

- (IBAction)trimAction:(id)sender {
    NSString *cmd = [self.preArray objectAtIndex:0];
    cmd = [cmd stringByAppendingString:@"TrimAction"];
    [self performSelector:NSSelectorFromString(cmd)];
}

- (IBAction)syncAction:(id)sender {
    NSString *cmd = [self.preArray objectAtIndex:0];
    cmd = [cmd stringByAppendingString:@"SyncAction"];
    [self performSelector:NSSelectorFromString(cmd)];
}

#pragma mark - MLNKV
- (void)mlnkvSetValueAction {
    NSLog(@"setmlnkv-memory-b:%f", [self usedMemory]);
    @autoreleasepool {
        int index = 0;
        CFAbsoluteTime beginTime = CFAbsoluteTimeGetCurrent();
        if (self.typeSegmented.selectedSegmentIndex == 0) {
            for (NSString *key in _keysArray) {
                NSString *value = _valuesArray[index];
                index ++;
                [_mlnkv setKVString:value forKey:key];
            }
        }else if (self.typeSegmented.selectedSegmentIndex == 1) {
            for (NSString *key in _keysArray) {
                NSNumber *value = _valuesArray[index];
                index ++;
                [_mlnkv setKVInt32:value.intValue forKey:key];
            }
        }
        CFAbsoluteTime space = CFAbsoluteTimeGetCurrent() - beginTime;
        self.setActionTimeLabel.text = [NSString stringWithFormat:@"耗时：%.4f ms", space * 1000];
        NSLog(@">>写入时间：%f", space);
        self.saveCountLabel.text = [NSString stringWithFormat:@"数据存储量：%ld", [_mlnkv count]];
    }
    NSLog(@"setmlnkv-memory-e:%f", [self usedMemory]);
    NSLog(@"--mem count:%zi", self.mlnkv.memoryCache.totalCount);
}

- (void)mlnkvGetValueAction {
    CFAbsoluteTime beginTime = CFAbsoluteTimeGetCurrent();
    if (self.typeSegmented.selectedSegmentIndex == 0) {
        for (NSString *key in _keysArray) {
            [_mlnkv getKVStringForKey:key];
            //        NSLog(@"%@", [_mlnkv getKVStringForKey:key]);
        }
    }else if (self.typeSegmented.selectedSegmentIndex == 1){
        for (NSString *key in _keysArray) {
            [_mlnkv getKVInt32ForKey:key];
            //        NSLog(@"%@", [_mlnkv getKVStringForKey:key]);
        }
    }
    
    CFAbsoluteTime space = CFAbsoluteTimeGetCurrent() - beginTime;
    self.getActionTimeLabel.text = [NSString stringWithFormat:@"耗时：%.4f ms", space * 1000];
    NSLog(@">>读取时间：%f", space);

}

- (void)mlnkvRemoveAction {
    CFAbsoluteTime beginTime = CFAbsoluteTimeGetCurrent();
    [_mlnkv removeValuesForKeys:_keysArray];
    CFAbsoluteTime space = CFAbsoluteTimeGetCurrent() - beginTime;
    self.removeActionTimeLabel.text = [NSString stringWithFormat:@"耗时：%.4f ms", space * 1000];
    NSLog(@">>删除时间：%f", space);
}

- (void)mlnkvClearAction {
    CFAbsoluteTime beginTime = CFAbsoluteTimeGetCurrent();
    [_mlnkv clearAll];
    CFAbsoluteTime space = CFAbsoluteTimeGetCurrent() - beginTime;
    self.clearActionTimeLabel.text = [NSString stringWithFormat:@"耗时：%.4f ms", space * 1000];
    NSLog(@">>清空时间：%f", space);
}

- (void)mlnkvTrimAction {
    CFAbsoluteTime beginTime = CFAbsoluteTimeGetCurrent();
    [_mlnkv trim];
    CFAbsoluteTime space = CFAbsoluteTimeGetCurrent() - beginTime;
    self.trimTimeLabel.text = [NSString stringWithFormat:@"耗时：%.4f ms", space * 1000];
    NSLog(@">>整理时间：%f", space);
}

- (void)mlnkvSyncAction {
    CFAbsoluteTime beginTime = CFAbsoluteTimeGetCurrent();
    [_mlnkv sync];
    CFAbsoluteTime space = CFAbsoluteTimeGetCurrent() - beginTime;
    self.syncActionTimeLabel.text = [NSString stringWithFormat:@"耗时：%.4f ms", space * 1000];
    NSLog(@">>同步时间：%f", space);
}


@end
