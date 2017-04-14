//
//  ZZEncoder.swift
//  LiveShow
//
//  Created by admin on 2017/1/25.
//  Copyright © 2017年 admin. All rights reserved.
//

import UIKit

enum ZZEncoderErrorCode {
    case VTSessionCreateFailed
    case VTSessionPrepareFailed
    case LockSampleBaseAddressFailed
    case EncodeVideoFrameFailed
    case EncodeCreateBlockBufFailed
    case EncodeCreateSampleBufFailed
    case EncodeGetSpsPpsFailed
    case EncodeGetH264DataFailed
    case CreateAudioConverterFailed
    case AudioConverterGetMaxFrameSizeFailed
    case AudioEncoderFailed
}



class ZZEncoder: NSObject {
    
    public var manager :ZZEncoderManager?;
    
    public func open () -> Void {
        
    }
    
    public func close () -> Void {
        
    }
    
    public func onErrorWithCode(code : ZZEncoderErrorCode ,description : String) -> Void {
        zz_log("[ERROR] encoder error code:%ld des:%s", code, description.UTF8String);
    }

}
