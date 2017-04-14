//
//  ZZCaptureConfig.swift
//  LiveShow
//
//  Created by admin on 2017/1/23.
//  Copyright © 2017年 admin. All rights reserved.
//

import UIKit

class ZZAudioConfig: NSObject ,NSCopying {
    /**可自由设置*/
    public var bitrate : NSInteger = 0;
    /**可选 1 2*/
    public var channelCount : NSInteger = 0;
    /**可选 44100 22050 11025 5500*/
    public var sampleRate : NSInteger = 0;
    /**可选 16 8*/
    public var sampleSize : NSInteger = 0;
    
    public var faacConfig : zz_faac_config?;
    
    override init() {
        self.bitrate = 100000;
        self.channelCount = 1;
        self.sampleSize = 16;
        self.sampleRate = 44100;
    }
    
    func copy(with zone: NSZone? = nil) -> Any {
        let audioConfig = ZZAudioConfig();
        audioConfig.bitrate = self.bitrate;
        audioConfig.channelCount = self.channelCount;
        audioConfig.sampleRate = self.sampleRate;
        audioConfig.sampleSize = self.sampleSize;
        return audioConfig;
    }
    
    deinit {
        
    }
}


class ZZVideoConfig: NSObject , NSCopying {
    /**可选，系统支持的分辨率，采集分辨率的高*/
    public var width : NSInteger = 0;
    /**可选，系统支持的分辨率，采集分辨率的高*/
    public var height : NSInteger = 0;
    /**自由设置*/
    public var bitrate : NSInteger = 0;
    /**自由设置*/
    public var fps : NSInteger = 0;
    /**目前软编码只能是X264_CSP_NV12，硬编码无需设置*/
    public var dataFormat : NSInteger = 0;
    /**推流方向*/
    public var orientation : UIInterfaceOrientation = .unknown;
    
    public var pushStreamWidth: NSInteger {
        get {
            if UIInterfaceOrientationIsLandscape(self.orientation) {
                return self.height;
            }
            return self.width;
        }
    }
    
    public var pushStreamHeight: NSInteger {
        get {
            if UIInterfaceOrientationIsPortrait(self.orientation) {
                return self.width;
            }
            return self.height;
        }
    }
    
    public var x264Config : zz_x264_config?;
    
    override init () {
        self.width = 540;
        self.height = 960;
        self.bitrate = 1000000;
        self.fps = 20;
        self.dataFormat = 0x0003;//X264_CSP_NV12;
    }
    
    deinit {
        
    }
    func copy(with zone: NSZone? = nil) -> Any {
        let videoConfig = ZZVideoConfig();
        
        return videoConfig;
    }
}
