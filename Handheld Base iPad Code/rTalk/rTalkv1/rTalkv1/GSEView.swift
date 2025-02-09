//
//  GSEView.swift
//  rTalkv1
//
//  Created by Mike Brinker on 10/3/24.
//

import UIKit
import Foundation
import AVFoundation



class GSEView: UIViewController {

    
    @IBOutlet weak var labelTankPT: UILabel!
    @IBOutlet weak var labelChamberPT: UILabel!
    @IBOutlet weak var labelLoadCell: UILabel!
    @IBOutlet weak var labelBattery: UILabel!
    @IBOutlet weak var labelTemperature: UILabel!
    @IBOutlet weak var labelPadHot: UILabel!
    @IBOutlet weak var labelRocketConnected: UILabel!
    @IBOutlet weak var labelStatus: UILabel!
    @IBOutlet weak var buttonErrors: UIButton!
    @IBOutlet weak var buttonStatus: UIButton!
    @IBOutlet weak var labelArmed: UILabel!
    @IBOutlet weak var switchFuelValve: UISwitch!
    @IBOutlet weak var switchOxValve: UISwitch!
    @IBOutlet weak var switchFillValve: UISwitch!
    @IBOutlet weak var switchPurgeValve: UISwitch!
    @IBOutlet weak var switchOpenFuelOx: UISwitch!
    @IBOutlet weak var switchCloseAll: UISwitch!
    @IBOutlet weak var switchIgniterFire: UISwitch!
    @IBOutlet weak var buttonFuelValve: UIButton!
    @IBOutlet weak var buttonOxValve: UIButton!
    @IBOutlet weak var buttonFillValve: UIButton!
    @IBOutlet weak var buttonPurgeVavle: UIButton!
    @IBOutlet weak var buttonopenFuelOx: UIButton!
    @IBOutlet weak var buttonCloseAll: UIButton!
    @IBOutlet weak var buttonFire: UIButton!
    @IBOutlet weak var buttonArmed: UIButton!
    @IBOutlet weak var imageFuelValve: UIImageView!
    @IBOutlet weak var imageoxValve: UIImageView!
    @IBOutlet weak var imageFillValve: UIImageView!
    @IBOutlet weak var imagePurgeValve: UIImageView!
    @IBOutlet weak var imageIgniterCont1: UIImageView!
    @IBOutlet weak var imageIgniterCont2: UIImageView!
    @IBOutlet weak var switchArm: UISwitch!
    @IBOutlet weak var switchAutoFill: UISwitch!
    @IBOutlet weak var siwtchDAQ: UISwitch!
    @IBOutlet weak var switchLaunch: UISwitch!
    @IBOutlet weak var switchPurge10: UISwitch!
    @IBOutlet weak var switchAbort: UISwitch!
    @IBOutlet weak var textAutoFill: UITextField!
    @IBOutlet weak var buttonAutoFill: UIButton!
    @IBOutlet weak var buttonDAQ: UIButton!
    @IBOutlet weak var buttonLaunch: UIButton!
    @IBOutlet weak var buttonPurge10: UIButton!
    @IBOutlet weak var buttonAbort: UIButton!
    @IBOutlet weak var labelAutoFillCounter: UILabel!
    @IBOutlet weak var labelDAQ: UILabel!
    @IBOutlet weak var labelPurge10: UILabel!
    @IBOutlet weak var labelNoSync: UILabel!
    @IBOutlet weak var labelHClaunch: UILabel!
    @IBOutlet weak var labelLaunch: UILabel!
    @IBOutlet weak var labelRadioTimeoutCounter: UILabel!
    @IBOutlet weak var buttonRadioOnOff: UIButton!
    @IBOutlet weak var textPurge: UITextField!
    
    
    

    var currentCount: Int = 0
    var timerOne: Timer?
    var timerTwo: Timer?
    var player: AVAudioPlayer?
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        //STARTUP DEFAULTS
        // closed or open2 or question for valve state
        // bad or good for igniter
        buttonErrors.isHidden = true
        labelAutoFillCounter.isHidden = true
        labelDAQ.isHidden = true
        labelLaunch.isHidden = true
        labelPurge10.isHidden = true
        labelRadioTimeoutCounter.isHidden = true
        buttonRadioOnOff.isHidden = true
        dataRefresh()
        //tap gestures
        labelTankPT.isUserInteractionEnabled = true
        let tapGesture = UITapGestureRecognizer(target: self, action: #selector(tankPTtapped))
        labelTankPT.addGestureRecognizer(tapGesture)
        labelChamberPT.isUserInteractionEnabled = true
        let tapGesture2 = UITapGestureRecognizer(target: self, action: #selector(chamberPTtapped))
        labelChamberPT.addGestureRecognizer(tapGesture2)
        
        //round the buttons
        buttonFuelValve.layer.cornerRadius = 10;buttonFuelValve.clipsToBounds = true
        buttonOxValve.layer.cornerRadius = 10;buttonOxValve.clipsToBounds = true
        buttonFillValve.layer.cornerRadius = 10;buttonFillValve.clipsToBounds = true
        buttonPurgeVavle.layer.cornerRadius = 10;buttonPurgeVavle.clipsToBounds = true
        buttonopenFuelOx.layer.cornerRadius = 10;buttonopenFuelOx.clipsToBounds = true
        buttonCloseAll.layer.cornerRadius = 10;buttonCloseAll.clipsToBounds = true
        buttonFire.layer.cornerRadius = 10;buttonFire.clipsToBounds = true
        buttonArmed.layer.cornerRadius = 10;buttonArmed.clipsToBounds = true
        buttonAutoFill.layer.cornerRadius = 10;buttonAutoFill.clipsToBounds = true
        buttonDAQ.layer.cornerRadius = 10;buttonDAQ.clipsToBounds = true
        buttonLaunch.layer.cornerRadius = 10;buttonLaunch.clipsToBounds = true
        buttonPurge10.layer.cornerRadius = 10;buttonPurge10.clipsToBounds = true
        buttonAbort.layer.cornerRadius = 10;buttonAbort.clipsToBounds = true
        buttonRadioOnOff.layer.cornerRadius = 10;buttonRadioOnOff.clipsToBounds = true
        
        //housekeeping timer
        timerOne = Timer.scheduledTimer(timeInterval: 0.25, target: self, selector: #selector(GSEView.gseHousekeeping), userInfo: nil, repeats: true)
        //configu backup timer
        timerTwo = Timer.scheduledTimer(timeInterval: 15.0, target: self, selector: #selector(GSEView.configCheck), userInfo: nil, repeats: true)
        print("GSE View Did Load")

    }
    
    
    override func viewDidAppear(_ animated: Bool) { //refresh everything
        super.viewDidAppear(false)
        dataRefresh()
    }
    
    
    

    
    @objc func configCheck() {
        
        //We don't have config file after first contact so check every 15 seconds
        if(!padConfig.confirmed && GSEworking.firstContact) {
            workingData.radioSendMessage = "#S,033,!"
            workingData.radioSend = true
        }
    }
    
    

    @objc func gseHousekeeping() { //fast timer for data refresh
      
        
        //------ REFRESH LOGIC -----------------------------
        if(padStatus.refresh) {
            dataRefresh()
            padStatus.refresh = false
        }
        if(padStatus.pressureRefresh) {
            pressureRefresh()
            padStatus.pressureRefresh = false
        }
        if(GSEerrors.refresh) {
            errorsRefresh()
            GSEerrors.refresh = false
        }
        if(!padConfig.confirmed) {
            labelNoSync.isHidden = false
        } else {
            labelNoSync.isHidden = true
        }
        
    
        // Fill Timer Counter
        if(!GSEworking.fillTimer) {
            labelAutoFillCounter.isHidden = true
        } else {
            buttonAutoFill.setTitle("STOP", for: .normal)
            labelAutoFillCounter.isHidden = false
            let theDiff: Int = getDateDiff(start: GSEworking.fillTimerStart, end: Date())
            labelAutoFillCounter.text = String(theDiff)
        }
        
        // Purge Timer Counter
        if(!GSEworking.purgeTimer) {
            labelPurge10.isHidden = true
        } else {
            labelPurge10.isHidden = false
            let theDiff: Int = getDateDiff(start: GSEworking.purgeTimerStart, end: Date())
            labelPurge10.text = String(theDiff)
        }
        // Status
        if(workingData.statusTimeout! > Date()) {
            labelStatus.text = workingData.status
        } else {
            labelStatus.text = ""
        }
        
        //Radio timeout error counter
        let theDiff: Int = getDateDiff(start: GSEworking.radioLast, end: Date())
        if(theDiff > 30) {
            labelRadioTimeoutCounter.isHidden = false
            labelRadioTimeoutCounter.text = String(theDiff)
        } else {
            labelRadioTimeoutCounter.isHidden = true
        }
        
    }
    
    
    
    func statusUpdate(thes: String) {
        workingData.status = thes
        workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
    }
    
    
    //===========================================================================================================================
    //===========================================================================================================================
    //====================================================     REFRESH ACTIONS    ===============================================
    //===========================================================================================================================
    //===========================================================================================================================
    
    
    
  
    func doStatus(theUpdate: String) {
        workingData.status = theUpdate
        workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
    }
    
    func errorsRefresh() {
        
        if(GSEerrors.errorCount > 0) {
            buttonErrors.isHidden = false
            if(GSEerrors.errorCount == 1) {
                if(GSEerrors.mainBatt == 1) {doStatus(theUpdate: "GSE ERROR: Battery Low")}
                if(GSEerrors.MCUtemp == 1) {doStatus(theUpdate: "GSE ERROR: MCU Temperature")}
                if(GSEerrors.PT == 1) {
                    if(!padStatus.rocketConnected) {
                        doStatus(theUpdate: "GSE ERROR: Rocket Disconnected")}
                } else {
                    doStatus(theUpdate: "GSE ERROR: Pressure Transducer")}
                if(GSEerrors.radio == 1) {doStatus(theUpdate: "GSE ERROR: Radio Error")}
                if(GSEerrors.SD == 1) {doStatus(theUpdate: "GSE ERROR: SD Card")}
                if(GSEerrors.ADC == 1) {doStatus(theUpdate: "GSE ERROR: ADC")}
                if(GSEerrors.MCUcrash == 1) {doStatus(theUpdate: "GSE ERROR: MCU Crash")}
                if(GSEerrors.Fuse == 1) {doStatus(theUpdate: "GSE ERROR: Fuse Blown")}
            } else {
                doStatus(theUpdate: "GSE: Multiple ERRORS reported")
            }
            self.playSound(theFile: "booster2")
            
        } else {
            buttonErrors.isHidden = true
        }
        
    }
    
    func getDateDiff(start: Date, end: Date) -> Int  {
        let calendar = Calendar.current
        let dateComponents = calendar.dateComponents([Calendar.Component.second], from: start, to: end)
        let seconds = dateComponents.second
        return Int(seconds!)
    }
    
    func dataRefresh() {
        
        //--- ROCKET CONNECT -----------------------------------
        if(padStatus.rocketConnected) {
            labelRocketConnected.text = "Connected"
            labelRocketConnected.textColor = .green
        } else {
            labelRocketConnected.text = "Disconnected"
            labelRocketConnected.textColor = .red
        }
        //--- PAD HOT -----------------------------------
        if(padStatus.padHot) {
            labelPadHot.text = "HOT"
            labelPadHot.textColor = .red
        } else {
            labelPadHot.text = "SAFE"
            labelPadHot.textColor = .green
        }
        //--- ARMED -----------------------------------
        if(padStatus.armed) {
            labelArmed.text = "ARMED"
            buttonArmed.setTitle("DISARM", for: .normal)
            labelArmed.backgroundColor = .red
            labelArmed.textColor = .white
            
        } else {
            labelArmed.text = "DISARMED"
            buttonArmed.setTitle("ARM", for: .normal)
            labelArmed.backgroundColor = .green
            labelArmed.textColor = .black
        }
        //--- IGNITER -----------------------------------
        if(padStatus.igniterContinuity) {
            imageIgniterCont1.image = UIImage(named: "good")
            imageIgniterCont2.image = UIImage(named: "good")
        } else {
            if(padStatus.padHot) {
                imageIgniterCont1.image = UIImage(named: "bad")
                imageIgniterCont2.image = UIImage(named: "bad")
            } else {
                imageIgniterCont1.image = UIImage(named: "question")
                imageIgniterCont2.image = UIImage(named: "question")
            }
        }
        //--- VOLTAGE -----------------------------------% zzz need alarm
        labelBattery.text = "\(String(padStatus.mainVolts))" + "%"
        if(padStatus.mainVolts > 15) {
            labelBattery.textColor = .black
        } else {
            labelBattery.textColor = .red
        }
        //--- SFU Valve -----------------------------------
        if(padStatus.SFU == 1) {
            buttonFuelValve.setTitle("Close", for: .normal)
            imageFuelValve.image = UIImage(named: "open2")
        } else {
            buttonFuelValve.setTitle("Open", for: .normal)
            imageFuelValve.image = UIImage(named: "closed")
        }
        //--- SOX Valve -----------------------------------
        if(padStatus.SOX == 1) {
            buttonOxValve.setTitle("Close", for: .normal)
            imageoxValve.image = UIImage(named: "open2")
        } else {
            buttonOxValve.setTitle("Open", for: .normal)
            imageoxValve.image = UIImage(named: "closed")
        }
        //--- SFL Valve -----------------------------------
        if(padStatus.SFL == 1) {
            buttonFillValve.setTitle("Close", for: .normal)
            imageFillValve.image = UIImage(named: "open2")
        } else {
            buttonFillValve.setTitle("Open", for: .normal)
            imageFillValve.image = UIImage(named: "closed")
        }
        //--- SPG Valve -----------------------------------
        if(padStatus.SPG == 1) {
            buttonPurgeVavle.setTitle("Close", for: .normal)
            imagePurgeValve.image = UIImage(named: "open2")
        } else {
            buttonPurgeVavle.setTitle("Open", for: .normal)
            imagePurgeValve.image = UIImage(named: "closed")
        }
        //--- Auto Fill Processing -----------------------------------
        if(padStatus.processFill60 == 0) {
            buttonAutoFill.setTitle("START", for: .normal)
            labelAutoFillCounter.isHidden = true
            GSEworking.fillTimer = false
        } else if(padStatus.processFill60 == 1){
            buttonAutoFill.setTitle("RESTART", for: .normal)
            labelAutoFillCounter.isHidden = false
            labelAutoFillCounter.text = "Done"
            GSEworking.fillTimer = false
        } else if(padStatus.processFill60 == 2){
            buttonAutoFill.setTitle("STOP", for: .normal)
            labelAutoFillCounter.isHidden = false
        } else {
            buttonAutoFill.setTitle("START", for: .normal)
            labelAutoFillCounter.isHidden = true
            GSEworking.fillTimer = false
        }
        //--- DAQ Processing -----------------------------------
        if(!GSEworking.DAQrecording) {
            buttonDAQ.setTitle("START", for: .normal)
            labelDAQ.isHidden = true
        } else {
            buttonDAQ.setTitle("STOP", for: .normal)
            labelDAQ.text = "Recording"
            labelDAQ.isHidden = false
        }
        //--- LAUNCH Processing -----------------------------------
        if(padStatus.processLaunch == 0) {
            buttonLaunch.setTitle("START", for: .normal)
            labelLaunch.isHidden = true
        } else if(padStatus.processLaunch == 1){
            buttonLaunch.setTitle("RESTART", for: .normal)
            labelLaunch.isHidden = false
            labelLaunch.text = "Done"
        } else if(padStatus.processFill60 == 2){
            buttonLaunch.setTitle("STOP", for: .normal)
            labelLaunch.isHidden = false
            labelPurge10.text = "In Process"
        } else {
            buttonLaunch.setTitle("START", for: .normal)
            labelLaunch.isHidden = true
        }
        //--- PURGE Processing -----------------------------------
        if(padStatus.processPurge == 0) {
            buttonPurge10.setTitle("START", for: .normal)
            labelPurge10.isHidden = true
            GSEworking.purgeTimer = false
        } else if(padStatus.processPurge == 1){
            buttonPurge10.setTitle("RESTART", for: .normal)
            labelPurge10.isHidden = false
            labelPurge10.text = "Done"
            GSEworking.purgeTimer = false
        } else if(padStatus.processPurge == 2){
            buttonPurge10.setTitle("STOP", for: .normal)
            labelPurge10.isHidden = false
            GSEworking.purgeTimer = true
        } else {
            buttonPurge10.setTitle("START", for: .normal)
            labelPurge10.isHidden = true
            GSEworking.purgeTimer = false
        }
        //--- CPU TEMP -----------------------------------% zzz need alarm
        labelTemperature.text = "\(String(padStatus.CPUtemp))" + "°"
        if(padStatus.CPUtemp > 160) {
            labelTemperature.textColor = .red
        } else {
            labelTemperature.textColor = .black
        }
        //--- Static vs Launch logic -----------------------------------
        if(padConfig.PCHenabled == 1 || padConfig.PLDenabled == 1) {
            labelHClaunch.isHidden = true

        } else {
            labelHClaunch.isHidden = false
            self.view.bringSubviewToFront(labelHClaunch)
        }
        
        pressureRefresh()
    }
        
    func pressureRefresh() {

        //--- PTs -----------------------------------
        if(padStatus.PTK == 99999 && !padStatus.rocketConnected) {
            labelTankPT.text = "NO RKT"
            labelTankPT.textColor = .black
        } else if(padStatus.PTK == 99999) {
            labelTankPT.text = "ERROR"
            labelTankPT.textColor = .red
        } else {
            labelTankPT.text = "\(String(padStatus.PTK))"
            if(padConfig.confirmed && padStatus.PTK >= padConfig.PTKalarm) {
                labelTankPT.textColor = .red
            } else {
                labelTankPT.textColor = .black
            }
        }
        
        if(padStatus.PCH == 99999) {
            labelChamberPT.text = "ERROR"
            labelChamberPT.textColor = .red
        } else {
            labelChamberPT.text = "\(String(padStatus.PCH))"
            labelChamberPT.textColor = .black
        }
        labelLoadCell.text = "\(String(padStatus.PLD))"
        
        
        if(padConfig.confirmed) {
            if(padConfig.PTKenabled == 0) {labelTankPT.text = "N/A";labelTankPT.textColor = .black}
            if(padConfig.PLDenabled == 0) {labelLoadCell.text = "N/A";labelLoadCell.textColor = .black}
            if(padConfig.PCHenabled == 0) {labelChamberPT.text = "N/A";labelChamberPT.textColor = .black}
        }
    
    }
        
    
    func playSound(theFile: String) {
        if(configData.mute == false) {
            guard let url = Bundle.main.url(forResource: theFile, withExtension: "mp3") else { return }

            do {
                try AVAudioSession.sharedInstance().setCategory(.playback, mode: .default)
                try AVAudioSession.sharedInstance().setActive(true)
                player = try AVAudioPlayer(contentsOf: url, fileTypeHint: AVFileType.mp3.rawValue)
                guard let player = player else { return }
                player.play()
            } catch let error {
                print(error.localizedDescription)
            }
        }
    }
    
    
    
    
    //===========================================================================================================================
    //===========================================================================================================================
    //=====================================================     BUTTON ACTIONS    ===============================================
    //===========================================================================================================================
    //===========================================================================================================================
    
    /*
       #DAQSON      DAQ on from simple remote
       #DAQOFF      DAQ off - stop recording
       #S          Request Status
       #FCFG        Set full configuration values
       #RSET        Reset the pad controller
       #DEFAULT    Default all configuration
       #OSFU        Open SFU
       #OSOX        Open SOX
       #OSFL        Open SFL
       #OSPG        Open SPG
       #CSFU        Close SFU
       #CSOX        Close SOX
       #CSFL        Close SFL
       #CSPG        Close SPG
       #CLOSE    Close all valves
       #ARM1        Command to ARM
       #ARM0        Command to Disarm
       #FIRE        Fire the Igniter
       #PF60     Automatic Process - Fill 60
       #PPRG     Automatic Process - Purge 10
       #PLCH     Automatic Process - Launch (close valve, igniter, wait, valves)
       #ABORT    Automatic Process Close All and Safe - Abort
       #ZPTK        Zero Tank PT
       #ZPCH        Zero Chamber PT
     */
    
    @IBAction func buttonErrors(_ sender: Any) {
        
        var message = "GSE HEALTH STATUS: \n  \n"
        if(GSEerrors.mainBatt == 1) {message += "Battery Voltage = ERROR \n"} else {message += "Battery Voltage = GOOD \n"}
        if(GSEerrors.MCUtemp == 1) {message += "MCU Temperature = ERROR \n"} else {message += "MCU Temperature = GOOD \n"}
        if(GSEerrors.PT == 1) {message += "Pressure Transducer = ERROR \n"} else {message += "Pressure Transducers = GOOD \n"}
        if(GSEerrors.radio == 1) {message += "GSE Radio = ERROR \n"} else {message += "GSE Radio = GOOD \n"}
        if(GSEerrors.SD == 1) {message += "SD Card = ERROR \n"} else {message += "SD Card = GOOD \n"}
        if(GSEerrors.ADC == 1) {message += "ADC/I2C Bus = ERROR \n"} else {message += "ADC/I2C Bus = GOOD \n"}
        if(GSEerrors.MCUcrash == 1) {message += "MCU Crash Reported = ERROR \n"} else {message += "No MCU Crash Reported \n"}
        if(GSEerrors.Fuse == 1) {message += "External Fuse Status = ERROR \n"} else {message += "Fuse Status = GOOD \n"}
        if(!padStatus.rocketConnected) {message += "Rocket Disconnected ERROR\n"} else {message += "Rocket is Connected \n"}
        
        let alert = UIAlertController(title: "GSE Health", message: message, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true, completion: nil)
        
    }
    
    
    @IBAction func buttonKitty(_ sender: Any) {
        self.playSound(theFile: "meow2")
        print("Meow")
    }
    
    @IBAction func ButtonStatusAction(_ sender: Any) {
        workingData.radioSendMessage = "#S,033,!"
        workingData.radioSend = true
        statusUpdate(thes: "Sent GSE Status Request...")
    }
    
    //----------------------- SFU --------------------------------------------------
    @IBAction func buttonSFUaction(_ sender: Any) {
        if(!switchFuelValve.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchFuelValve.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        if(padStatus.SFU == 0) {
            workingData.radioSendMessage = "#OSFU,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Open SFU Request...")
        } else {
            workingData.radioSendMessage = "#CSFU,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Close SFU Request...")
        }
    }
    
    @IBAction func buttonArmAction(_ sender: Any) {
        if(!switchArm.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchArm.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {
            workingData.radioSendMessage = "#ARM1,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent ARM Request...")
        } else {
            workingData.radioSendMessage = "#ARM0,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent DISARM Request...")
        }
    }
    
    @IBAction func buttonSOXaction(_ sender: Any) {
        if(!switchOxValve.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchOxValve.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        if(padStatus.SOX == 0) {
            workingData.radioSendMessage = "#OSOX,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Open SOX Request...")
        } else {
            workingData.radioSendMessage = "#CSOX,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Close SOX Request...")
        }
    }
    
    @IBAction func buttonSFLaction(_ sender: Any) {
        if(!switchFillValve.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchFillValve.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        if(padStatus.SFL == 0) {
            workingData.radioSendMessage = "#OSFL,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Open SFL Request...")
        } else {
            workingData.radioSendMessage = "#CSFL,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Close SFL Request...")
        }
    }
    
    @IBAction func buttonSPGaction(_ sender: Any) {
        if(!switchPurgeValve.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchPurgeValve.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        if(padStatus.SPG == 0) {
            workingData.radioSendMessage = "#OSPG,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Open SPG Request...")
        } else {
            workingData.radioSendMessage = "#CSPG,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Close SPG Request...")
        }
    }
    
    @IBAction func buttonFOaction(_ sender: Any) {
        if(!switchOpenFuelOx.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchOpenFuelOx.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        workingData.radioSendMessage = "#OFAO,033,!"
        workingData.radioSend = true
        statusUpdate(thes: "GSE: Sent Open SFU & SOX Request...")
    }
    
    @IBAction func buttonCloseAll(_ sender: Any) {
        if(!switchCloseAll.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchCloseAll.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        workingData.radioSendMessage = "#CLOSE,033,!"
        workingData.radioSend = true
        statusUpdate(thes: "GSE: Sent Close All Request...")
    }
    
    @IBAction func switchIgniterWarning(_ sender: Any) {
        if(switchIgniterFire.isOn) {
            if(!padStatus.igniterContinuity) {
                let alert = UIAlertController(title: "WARNING", message: "No igniter continuity detected", preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
                self.present(alert, animated: true)
            }
        }
    }
    
    @IBAction func buttonIgniterFireAction(_ sender: Any) {
        if(!switchIgniterFire.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchIgniterFire.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        workingData.radioSendMessage = "#FIRE,033,!"
        workingData.radioSend = true
        statusUpdate(thes: "GSE: Sent Igniter FIRE Request...")
    }
    
    @IBAction func buttonAutoFillAction(_ sender: Any) {
        if(!switchAutoFill.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchAutoFill.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        
        if(padStatus.processFill60 == 2){ //already running
            workingData.radioSendMessage = "#PFILL0,33,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Fill Stop Request...")
        } else {
            if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
            var thes: String = ""
            thes = textAutoFill.text ?? "0"
            thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
            if(Int(thes) ?? 0 < 1 || Int(thes) ?? 0 > 200) {
                popup(TheS: "Out of Range. Must be 1-200 seconds.")
                textAutoFill.text = "60"
                return
            }
            thes = "#PFILL," + thes + ",!"
            workingData.radioSendMessage = thes
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent Fill Start Request...")
            GSEworking.fillTimerStart = Date() // start the clock
        }
    }
    
    @IBAction func buttonDAQaction(_ sender: Any) {
        if(!siwtchDAQ.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        siwtchDAQ.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!GSEworking.DAQrecording) {
            workingData.radioSendMessage = "#DAQSON,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent DAQ ON Request...")
        } else {
            workingData.radioSendMessage = "#DAQOFF,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent DAQ OFF Request...")
        }
    }
    
    @IBAction func switchIgniterWarning2(_ sender: Any) {
        if(!padStatus.igniterContinuity) {
            let alert = UIAlertController(title: "WARNING", message: "No igniter continuity detected", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
            self.present(alert, animated: true)
        }
    }
    
    
    @IBAction func buttonLaunchAction(_ sender: Any) {
        if(!switchLaunch.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchLaunch.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        workingData.radioSendMessage = "#PLCH,33,!"
        workingData.radioSend = true
        statusUpdate(thes: "GSE: Sent LAUNCH Request...")
        GSEworking.fillTimerStart = Date() // start the clock
        labelLaunch.isHidden = false
        labelLaunch.text = "Requested"
        buttonRadioOnOff.isHidden = false
    }
    
    @IBAction func buttonPurgeAction(_ sender: Any) {
        if(!switchPurge10.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchPurge10.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return;}
        if(padStatus.processPurge == 2) {
            workingData.radioSendMessage = "#PPRG0,033,!"
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent PURGE STOP Request...")
        } else {
            var thes: String = ""
            thes = textPurge.text ?? "0"
            thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
            if(Int(thes) ?? 0 < 1 || Int(thes) ?? 0 > 200) {
                popup(TheS: "Out of Range. Must be 1-200 seconds.")
                textPurge.text = "10"
                return
            }
            thes = "#PPRG," + thes + ",!"
            workingData.radioSendMessage = thes
            workingData.radioSend = true
            statusUpdate(thes: "GSE: Sent PURGE START Request...")
            GSEworking.purgeTimerStart = Date() // start the clock
        }
    }
    
    @IBAction func buttonAbortAction(_ sender: Any) {
        if(!switchAbort.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchAbort.isOn = false
        workingData.radioSendMessage = "#ABORT,033,!"
        workingData.radioSend = true
        statusUpdate(thes: "GSE: Sent ABORT Request...")
    }
    
    @IBAction func buttonRadioOff(_ sender: Any) {
        //Toggle Radio Silence mode on the GSE
        workingData.radioSendMessage = "#RAD,033,!"
        workingData.radioSend = true
        statusUpdate(thes: "GSE: Radio Off Toggle...")
    }
    
    @objc func tankPTtapped() {
        let alert = UIAlertController(title: "Are you sure you want to zero the Tank PT?", message: nil, preferredStyle: .alert)
        let okAction = UIAlertAction(title: "OK", style: .default) { _ in
            workingData.radioSendMessage = "#ZPTK,033,!"
            workingData.radioSend = true
            self.statusUpdate(thes: "GSE: Sent Zero PT Request...")
        }
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler: nil)
        alert.addAction(okAction)
        alert.addAction(cancelAction)
        present(alert, animated: true, completion: nil)
    }
    @objc func chamberPTtapped() {
        let alert = UIAlertController(title: "Are you sure you want to zero the Chamber PT?", message: nil, preferredStyle: .alert)
        let okAction = UIAlertAction(title: "OK", style: .default) { _ in
            workingData.radioSendMessage = "#ZPCH,033,!"
            workingData.radioSend = true
            self.statusUpdate(thes: "GSE: Sent Zero Chamber PT Request...")
        }
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler: nil)
        alert.addAction(okAction)
        alert.addAction(cancelAction)
        present(alert, animated: true, completion: nil)
    }
    
    
    
    
    
    
    //====================================================================================
    
    
    
    func popupNotConfirmed() { // popup alert
        let alert = UIAlertController(title: "ERROR", message: "The pad has not sent the config to the iPad", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true)
    }
    
    func popup(TheS: String) {
        let alert = UIAlertController(title: "ERROR", message: TheS, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true)
    }
    
    
    
    func denyArmed() { // popup alert
        
        let alert = UIAlertController(title: "Denied", message: "The pad must be ARMED before sending the request", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true)
    }
    
    
    

    func denySend() { // popup alert
        
        let alert = UIAlertController(title: "Denied", message: "You must unlock the safety before sending the request", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true)
    }
    
    
    func notHot() { // popup alert
        
        let alert = UIAlertController(title: "Denied", message: "The pad controller is not hot. Insert Safety Plug.", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true)
        
    }
    
    
    @IBAction func textAutoFillChange(_ sender: Any) {
        var thes: String = ""
        thes = textAutoFill.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 1 || Int(thes) ?? 0 > 200) {
            popup(TheS: "Out of Range. Must be 1-200 seconds.")
            textAutoFill.text = "60"
        }
    }
    
    @IBAction func textAutoPurgeChange(_ sender: Any) {
        var thes: String = ""
        thes = textPurge.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 1 || Int(thes) ?? 0 > 200) {
            popup(TheS: "Out of Range. Must be 1-200 seconds.")
            textPurge.text = "10"
        }
    }
    
    
    

} // end view



/*
 SEND COMMANDS:
 
 #DAQSON      DAQ on from simple remote
 #DAQOFF      DAQ off - stop recording
 #S          Request Status
 #FCFG1    Set full configuration values first half
 #FCFG2.   second half (64b limit with red cable)
 #RSET        Reset the pad controller
 #DEFAULT    Default all configuration
 #OSFU        Open SFU
 #OSOX        Open SOX
 #OSFL        Open SFL
 #OSPG        Open SPG
 #OFAO     Open SFU and SOX
 #CSFU        Close SFU
 #CSOX        Close SOX
 #CSFL        Close SFL
 #CSPG        Close SPG
 #CLOSE    Close all valves
 #ARM1        Command to ARM
 #ARM0        Command to Disarm
 #FIRE        Fire the Igniter
 #PFILL    Automatic Process - Fill for N seconds (#PFILL0 to stop)
 #PPRG     Automatic Process - Purge 10  (#PPRG0 to stop)
 #PLCH     Automatic Process - Launch (close valve, igniter, wait, valves)
 #ABORT    Automatic Process Close All and Safe - Abort
 #ZPTK        Zero Tank PT
 #ZPCH        Zero Chamber PT
 #RAD       Toggle radio silence mode
 */
 
