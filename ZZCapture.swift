//
//  ZZCapture.swift
//  LiveShow
//
//  Created by admin on 2017/1/23.
//  Copyright © 2017年 admin. All rights reserved.
//

import UIKit
import AVFoundation

func zz_rtmp_state_changed_cb_in_oc(old_state : zz_rtmp_state , new_state : zz_rtmp_state )->Void {
    
}

protocol ZZCaptureDelegate {
    func avCaptureStateChangeFromTo(capture:ZZCapture,fromState:zz_rtmp_state,toState:zz_rtmp_state) -> Void;
}

class ZZCapture: NSObject {
    /**配置*/
    public var videoConfig : ZZVideoConfig?;
    
    public var audioConfig : ZZAudioConfig?;
    /**编码器类型*/
    public var audioEncoderType : ZZAudioEncoderType?;
    
    public var videoEncoderType : ZZAudioEncoderType?
    /**编码数据队列*/
    private(set) var encodeSampleQueue : DispatchQueue {
        get {
            return self.encodeSampleQueue;
        }
        set {
            self.encodeSampleQueue = DispatchQueue(label:"encodeSampleQueue");
        }
    }
    /**发送数据队列*/
    private(set) var sendSampleQueue : DispatchQueue {
        get {
            return self.encodeSampleQueue;
        }
        set {
            self.sendSampleQueue = DispatchQueue(label: "sendSampleQueue");
        }
    }
    /**状态变化回调*/
    public var stateDelegate :ZZCaptureDelegate?;
    /**是否将数据发送出去*/
    public var isCapturing : Bool?;
    /**预览view*/
    public var preview : UIView?;
    /**根据videoConfig获取当前CaptureSession preset分辨率*/
    public var captureSessionPreset : String?;
    /**是否已发送了sps/pps*/
    private var isSpsPpsAndAudioSpecificConfigSent : Bool?;
    /**编码管理*/
    private var encoderManager : ZZEncoderManager {
        set {
            self.encoderManager = ZZEncoderManager();
            
        }
        get{
            return self.encoderManager;
        }
    };
    /**进入后台后，不推视频流*/
    private var inBackground : Bool?
    /**初始化*/
    init(videoConfig:ZZVideoConfig,audioConfig:ZZAudioConfig) {
        
    }
    /**初始化*/
    func onInit() -> Void {
        
    }
    /**修改fps*/
    func updateFps(fps:NSInteger) -> Void {
        
    }
    /**切换摄像头*/
    func switchCamera() -> Void {
        
    }
    /**停止capture*/
    func stopCapture() -> Void {
        
    }
    /**停止*/
    func onStopCapture() -> Void {
        
    }
    /**用户开始*/
    func onStartCapture() -> Void {
        
    }
    /**开始capture*/
    func startCaptureWithRtmpUrl(rtmpUrl:String) -> Void {
        
    }
    /**使用rtmp协议发送数据*/
    func sendVideoSampleBuffer(sampleBuffer:CMSampleBuffer) -> Void {
        
    }
    
    func sendAudioSampleBuffer(sampleBuffer:CMSampleBuffer) -> Void {
        
    }
    
    func sendVideoYuvData(videoData:NSData) -> Void {
        
    }
    
    func sendAudioPcmData(audioData:NSData) -> Void {
        
    }

    func sendFlvVideoTag(flvVideoTag : zz_flv_video_tag) -> Void {
        
    }
    
    func sendFlvVideoTag(flvAudioTag : zz_flv_audio_tag) -> Void {
        
    }
    
}
