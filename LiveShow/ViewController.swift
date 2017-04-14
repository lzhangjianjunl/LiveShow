//
//  ViewController.swift
//  LiveShow
//
//  Created by admin on 2017/1/17.
//  Copyright © 2017年 admin. All rights reserved.
//

import UIKit

import PromiseKit
import AFNetworking
import Masonry
import SDWebImage
import NSLogger
import ReactiveSwift
import AVFoundation


class ViewController: UIViewController {
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        self.view.backgroundColor = UIColor.cyan;
        LoggerSetOptions(OpaquePointer!.init(nilLiteral: ()), UInt32(
            kLoggerOption_BufferLogsUntilConnection
                | kLoggerOption_UseSSL
                | kLoggerOption_CaptureSystemConsole
                | kLoggerOption_BrowseBonjour
                | kLoggerOption_BrowseOnlyLocalDomain
                | 0));
        
         avcodec_register_all();
    }
    
    func buttonAction(sender: UIButton!) {
        LogMessageRaw("Button tapped")
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated);
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

