//
//  ZZEncoderManager.swift
//  LiveShow
//
//  Created by admin on 2017/1/25.
//  Copyright © 2017年 admin. All rights reserved.
//

import UIKit

enum ZZVideoEncoderType : NSInteger {
    case None
    case HWH264
    case SWX264
}

enum ZZAudioEncoderType : NSInteger {
    case None
    case HWAACLC
    case SWFAAC
}

class ZZEncoderManager: NSObject {

    /**编码器类型*/
    public var audioEncoderType : ZZAudioEncoderType {
        set {
            self.audioEncoderType = newValue;
        }
        get {
            return self.audioEncoderType;
        }
    }
    public var videoEncoderType : ZZVideoEncoderType {
        set {
            self.videoEncoderType = newValue;
        }
        get {
            return self.videoEncoderType;
        }
    }
    /**编码器*/
    private(set) var videoEncoder : ZZVideoEncoder  {
        get {
            return self.videoEncoder;
        }
        set {
            
        }
    }
    
    private(set) var audioEncoder : ZZAudioEncoder  {
        get {
            return self.audioEncoder;
        }
        set {
            
        }
    }
    /**时间戳*/
    public var timestamp : __uint32_t?;
    /**开启关闭*/
    func openWithAudioConfig(audioConfig : ZZAudioConfig , videoConfig : ZZVideoConfig ) -> Void {
        switch self.audioEncoderType {
        case .HWAACLC :
            self.audioEncoder = ZZHWAACEncoder();
            break;
        case .SWFAAC :
            self.audioEncoder = ZZSWFaacEncoder();
            break;
        default:
            print("[E] AWEncoderManager.open please assin for audioEncoderType");
            break;
            
        }
        switch self.videoEncoderType {
        case .HWH264 :
            self.videoEncoder = ZZHWH264Encoder();
            break;
        case .SWX264 :
            self.videoEncoder = ZZSWX264Encoder();
            break;
        default:
            print("[E] AWEncoderManager.open please assin for videoEncoderType");
            break;
        }
        
        self.audioEncoder.audioConfig = audioConfig;
        self.videoEncoder.videoConfig = videoConfig;
        
        self.audioEncoder.manager = self;
        self.videoEncoder.manager = self;
        
        self.audioEncoder.open();
        self.videoEncoder.open();
    }
    
    func close() -> Void {
        
    }
}
