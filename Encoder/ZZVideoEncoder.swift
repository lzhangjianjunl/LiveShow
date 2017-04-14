//
//  ZZVideoEncoder.swift
//  LiveShow
//
//  Created by admin on 2017/1/25.
//  Copyright © 2017年 admin. All rights reserved.
//
/*
 视频编码器基类，只声明接口，和一些公共转换数据函数。
 */

import UIKit

class ZZVideoEncoder: ZZEncoder {

    public var videoConfig : ZZVideoConfig?;
    
    //旋转
    func rotateNV12Data(nv12Data : NSData) -> NSData {
        var degree : Int = 0;
        switch self.videoConfig?.orientation {
        case .landscapeLeft:
            degree = 90;
            break;
        case .landscapeRight:
            degree = 270;
            break;
        default:
            break;
        }
        
        if degree != 0
        {
            
        }
    }
    //编码
    func encodeYUVDataToFlvTag(yuvData : NSData) -> zz_flv_video_tag {
        
    }
    
    func encodeVideoSampleBufToFlvTag(videoSample : CMSampleBuffer) -> zz_flv_video_tag {
        
    }
    //根据flv，h264，aac协议，提供首帧需要发送的tag
    //创建sps pps
    func createSpsPpsFlvTag() -> zz_flv_video_tag {
        
    }
    //转换
    func convertVideoSmapleBufferToYuvData(videoSample : CMSampleBuffer) -> NSData {
        
    }
}
