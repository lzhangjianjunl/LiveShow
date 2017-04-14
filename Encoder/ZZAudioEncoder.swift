//
//  ZZAudioEncoder.swift
//  LiveShow
//
//  Created by admin on 2017/1/25.
//  Copyright © 2017年 admin. All rights reserved.
//
/*
 音频编码器基类，只声明接口，和一些公共转换数据函数。
 */

import UIKit

class ZZAudioEncoder: ZZEncoder {
    public var audioConfig : ZZAudioConfig?;
    
    func encodePCMDataToFlvTag(pcmData : NSData) -> zz_flv_audio_tag {
        return zz_flv_audio_tag.init();
    }
    
    func encodeAudioSampleBufToFlvTag(audioSample : CMSampleBuffer) -> zz_flv_audio_tag {
        return self.encodePCMDataToFlvTag(pcmData: self.convertAudioSmapleBufferToPcmData(audioSample: audioSample));
    }
    
    func createAudioSpecificConfigFlvTag() -> zz_flv_audio_tag {
        return zz_flv_audio_tag.init();
    }
    
    func convertAudioSmapleBufferToPcmData(audioSample : CMSampleBuffer) -> NSData {
        //获取pcm数据大小
        let audioDataSize  = CMSampleBufferGetTotalSampleSize(audioSample);
        //分配空间
        let audio_data = zz_alloc_detail(audioDataSize, "", 0);
        //获取CMBlockBufferRef
        //这个结构里面就保存了 PCM数据
        let dataBuffer = CMSampleBufferGetDataBuffer(audioSample);
        //直接将数据copy至我们自己分配的内存中
        CMBlockBufferCopyDataBytes(dataBuffer!, 0, audioDataSize, audio_data!);
        return NSData.init(bytesNoCopy: audio_data!, length: audioDataSize);
    }
    
    
}
