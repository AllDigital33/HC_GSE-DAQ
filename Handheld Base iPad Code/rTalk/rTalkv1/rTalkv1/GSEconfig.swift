//
//  GSEconfig.swift
//  rTalkv1
//
//  Created by Mike Brinker on 10/6/24.
//

import UIKit
import AVFoundation

class CatConfigView: UIViewController {
    

    @IBOutlet weak var labelStatus: UILabel!
    @IBOutlet weak var segmentSwitch: UISegmentedControl!
    @IBOutlet weak var labelPT: UILabel!
    @IBOutlet weak var labelServo: UILabel!
    @IBOutlet weak var textTemp: UITextField!
    @IBOutlet weak var textPTKrange: UITextField!
    @IBOutlet weak var textPTKalarm: UITextField!
    @IBOutlet weak var textPCHrange: UITextField!
    @IBOutlet weak var switchPTKenable: UISwitch!
    @IBOutlet weak var switchPCHenable: UISwitch!
    @IBOutlet weak var switchLoadEnable: UISwitch!
    @IBOutlet weak var switchSave: UISwitch!
    
    @IBOutlet weak var switchTestUnlock: UISwitch!
    @IBOutlet weak var textSFUopen: UITextField!
    @IBOutlet weak var textSFUclose: UITextField!
    @IBOutlet weak var textSOXopen: UITextField!
    @IBOutlet weak var textSOXclose: UITextField!
    @IBOutlet weak var textSFLopen: UITextField!
    @IBOutlet weak var textSFLclose: UITextField!
    @IBOutlet weak var textSPGopen: UITextField!
    @IBOutlet weak var textSPGclose: UITextField!
    @IBOutlet weak var buttonSave: UIButton!
    @IBOutlet weak var buttonReset: UIButton!
    @IBOutlet weak var buttonRequestStatus: UIButton!
    @IBOutlet weak var switchRadioToggle: UISwitch!
    
    var timerOne: Timer?
    
    
    override func viewDidLoad() {
        
        labelPT.layer.borderWidth = 2.0  // Border width
        labelPT.layer.borderColor = UIColor.black.cgColor  // Border color
        labelPT.layer.cornerRadius = 5.0  // Optional: Rounded corners
        labelPT.clipsToBounds = true  // Required to clip the corners
        labelServo.layer.borderWidth = 2.0  // Border width
        labelServo.layer.borderColor = UIColor.black.cgColor  // Border color
        labelServo.layer.cornerRadius = 5.0  // Optional: Rounded corners
        labelServo.clipsToBounds = true  // Required to clip the corners
        refreshData()
        
        //housekeeping timer
        timerOne = Timer.scheduledTimer(timeInterval: 0.25, target: self, selector: #selector(CatConfigView.gseHousekeeping), userInfo: nil, repeats: true)
    }
    
    override func viewDidAppear(_ animated: Bool) { //refresh everything
        super.viewDidAppear(false)
        refreshData()
       
    }
    
    @objc func gseHousekeeping() { //fast timer for data refresh
        
        // Status
        if(workingData.statusTimeout! > Date()) {
            labelStatus.text = workingData.status
        } else {
            labelStatus.text = ""
        }
        
        if(padConfig.refresh) {
            refreshData();
            padConfig.refresh = false
        }
        
    }
    
    func refreshData() {
        
        if(!padConfig.confirmed) {
            workingData.status = "No GSE Config File Yet"
            workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
            return
        } else {
            labelStatus.text = ""
        }
        if(padConfig.PLDenabled == 1 && padConfig.PCHenabled == 1) {
            segmentSwitch.selectedSegmentIndex = 0
        } else {
            segmentSwitch.selectedSegmentIndex = 1
        }
        textTemp.text = String(padConfig.CPUtempAlarm)
        //Rocket Tank PT
        if(padConfig.PTKenabled == 1) {
            switchPTKenable.isOn = true
        } else {
            switchPTKenable.isOn = false
        }
        textPTKrange.text = String(padConfig.PTKrange)
        textPTKalarm.text = String(padConfig.PTKalarm)
        //Chamber PT
        if(padConfig.PCHenabled == 1) {
            switchPCHenable.isOn = true
        } else {
            switchPCHenable.isOn = false
        }
        textPCHrange.text = String(padConfig.PCHrange)
        //Load Cell
        if(padConfig.PLDenabled == 1) {
            switchLoadEnable.isOn = true
        } else {
            switchLoadEnable.isOn = false
        }
        switchSave.isOn = false;
        
        //==========================  SERVO REFRESH ===========================
        
        textSFUopen.text = String(padConfig.SFUopen)
        textSFUclose.text = String(padConfig.SFUclose)
        textSOXopen.text = String(padConfig.SOXopen)
        textSOXclose.text = String(padConfig.SOXclose)
        textSFLopen.text = String(padConfig.SFLopen)
        textSFLclose.text = String(padConfig.SFLclose)
        textSPGopen.text = String(padConfig.SPGopen)
        textSPGclose.text = String(padConfig.SPGclose)
        
        
    }
    
    
    
    
    
    //================================ TEXT FIELDS ==============================
    
    @IBAction func TempEditChange(_ sender: Any) {
        
        if(!padConfig.confirmed) {popupNotConfirmed();refreshData();return}
        var thes: String = ""
        thes = textTemp.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 200) {
            popup(TheS: "Out of Range. Must be 0-200.")
            textTemp.text = String(padConfig.CPUtempAlarm)
        }
    }
    @IBAction func tankPTchange(_ sender: Any) {
        if(!padConfig.confirmed) {popupNotConfirmed();refreshData();return}
        var thes: String = ""
        thes = textPTKrange.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 5000) {
            popup(TheS: "Out of Range. Must be 0-5000.")
            textPTKrange.text = String(padConfig.PTKrange)
        }
    }
    @IBAction func textPTKalarm(_ sender: Any) {
        if(!padConfig.confirmed) {popupNotConfirmed();refreshData();return}
        var thes: String = ""
        thes = textPTKalarm.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 5000) {
            popup(TheS: "Out of Range. Must be 0-5000.")
            textPTKalarm.text = String(padConfig.PTKalarm)
        }
    }
    @IBAction func textChamberChange(_ sender: Any) {
        if(!padConfig.confirmed) {popupNotConfirmed();refreshData();return}
        var thes: String = ""
        thes = textPCHrange.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 5000) {
            popup(TheS: "Out of Range. Must be 0-5000.")
            textPCHrange.text = String(padConfig.PCHrange)
        }
    }
    @IBAction func textSFUchange(_ sender: Any) {
        var thes: String = ""
        thes = textSFUopen.text ?? "0"
        if(!validateRange(theS: thes)) {
            textSFUopen.text = String(padConfig.SFUopen)
        }
    }
    @IBAction func textSFUcloseChange(_ sender: Any) {
        var thes: String = ""
        thes = textSFUclose.text ?? "0"
        if(!validateRange(theS: thes)) {
            textSFUclose.text = String(padConfig.SFUclose)
        }
    }
    
    @IBAction func textSOWopenChange(_ sender: Any) {
        var thes: String = ""
        thes = textSOXopen.text ?? "0"
        if(!validateRange(theS: thes)) {
            textSOXopen.text = String(padConfig.SOXopen)
        }
    }
    @IBAction func textSOXcloseChange(_ sender: Any) {
        var thes: String = ""
        thes = textSOXclose.text ?? "0"
        if(!validateRange(theS: thes)) {
            textSOXclose.text = String(padConfig.SOXclose)
        }
    }
    @IBAction func textSFLopenChange(_ sender: Any) {
        var thes: String = ""
        thes = textSFLopen.text ?? "0"
        if(!validateRange(theS: thes)) {
            textSFLopen.text = String(padConfig.SFLopen)
        }
    }
    @IBAction func textSFLcloseChange(_ sender: Any) {
        var thes: String = ""
        thes = textSFLclose.text ?? "0"
        if(!validateRange(theS: thes)) {
            textSFLclose.text = String(padConfig.SFLclose)
        }
    }
    
    @IBAction func textSPGopen(_ sender: Any) {
        var thes: String = ""
        thes = textSPGopen.text ?? "0"
        if(!validateRange(theS: thes)) {
            textSPGopen.text = String(padConfig.SPGopen)
        }
    }
    @IBAction func textSPGcloseChange(_ sender: Any) {
        var thes: String = ""
        thes = textSPGclose.text ?? "0"
        if(!validateRange(theS: thes)) {
            textSPGclose.text = String(padConfig.SPGclose)
        }
    }

    func validateRange(theS: String) -> Bool  {
     
        var testS: String = theS
        testS = testS.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(testS) ?? 0 < 0 || Int(testS) ?? 0 > 5000) {
            popup(TheS: "Out of Range. Must be 0-5000.")
            return false
        } else {
            return true
        }
    }
    

    //================================ SEND FULL CONFIG ==============================
    
    
    func createRadioMessage1() {
        
        var thes: String = ""
        workingData.radioSendMessage = "#FCFG1,"
        
        if(switchPTKenable.isOn) {addRS(newAdd: "1")} else {addRS(newAdd: "0")}
        thes = trimIt(testS: textPTKrange.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textPTKalarm.text ?? "99"); addRS(newAdd: thes)
        if(switchLoadEnable.isOn) {addRS(newAdd: "1")} else {addRS(newAdd: "0")}
        if(switchPCHenable.isOn) {addRS(newAdd: "1")} else {addRS(newAdd: "0")}
        thes = trimIt(testS: textPCHrange.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textTemp.text ?? "99"); addRS(newAdd: thes)
        workingData.radioSendMessage = (workingData.radioSendMessage ?? "BAD") + "!"

    }
    
    func createRadioMessage2() {
        
        var thes: String = ""
        workingData.radioSendMessage = "#FCFG2,"
        thes = trimIt(testS: textSFUopen.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textSFUclose.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textSOXopen.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textSOXclose.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textSFLopen.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textSFLclose.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textSPGopen.text ?? "99"); addRS(newAdd: thes)
        thes = trimIt(testS: textSPGclose.text ?? "99"); addRS(newAdd: thes)
        workingData.radioSendMessage = (workingData.radioSendMessage ?? "BAD") + "!"

    }
    
    func addRS(newAdd: String) {
        workingData.radioSendMessage = (workingData.radioSendMessage ?? "0") + newAdd + ","
    }

    func trimIt(testS: String) -> String {
        var tmp: String = testS
        tmp = tmp.trimmingCharacters(in: .whitespacesAndNewlines)
        return tmp
    }
    
    func validate() -> Bool {
     
        
        return true
    }
    
    //================================ BUTTON ACTIONS ==============================
    
    @IBAction func BUTTONRESETACTION(_ sender: Any) {
        refreshData()
    }
    
    @IBAction func buttonSaveAction(_ sender: Any) {
        
        if(!switchSave.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        if(!padConfig.confirmed) {popupNotConfirmed();refreshData();switchSave.isOn = false; return}
        //Good to go -- make the string
        switchSave.isOn = false
        let checkit = validate() //not needed for now - using real time edits
        createRadioMessage1()
        workingData.radioSend = true
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            self.createRadioMessage2()
            workingData.radioSend = true
            workingData.status = "Sent GSE Config Changes..."
            workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
        }
        
    }
    
    @IBAction func buttonRadio(_ sender: Any) {
        
        //Toggle Radio Silence mode on the GSE
        if(!switchRadioToggle.isOn){popup(TheS: "Unlock Two Factor Switch First");return;}
        switchRadioToggle.isOn = false
        workingData.radioSendMessage = "#RAD,033,!"
        workingData.radioSend = true
        workingData.status = "Sent GSE Radio Toggle..."
        workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
        
    }
    
    
    @IBAction func buttonStatus(_ sender: Any) {
        workingData.radioSendMessage = "#S,033,!"
        workingData.radioSend = true
        workingData.status = "Sent GSE Config Request..."
        workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
    }
    
    @IBAction func segmentChange(_ sender: Any) {
        
        if(!padConfig.confirmed) {popupNotConfirmed();refreshData();return}
        if(segmentSwitch.selectedSegmentIndex == 0) {
            switchPCHenable.isOn = true
            switchLoadEnable.isOn = true
            padConfig.staticFire = true
        } else {
            switchPCHenable.isOn = false
            switchLoadEnable.isOn = false
            padConfig.staticFire = false
        }
    }
    
    @IBAction func buttonZeroPTK(_ sender: Any) {
        if(!switchPTKenable.isOn){popup(TheS: "Enable the PT first");return;}
        workingData.radioSendMessage = "#ZPTK,033,!"
        workingData.radioSend = true
        workingData.status = "Sent GSE PTK Request..."
        workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
    }
    
    @IBAction func buttonZeroPCH(_ sender: Any) {
        if(!switchPCHenable.isOn){popup(TheS: "Enable the PT first");return;}
        workingData.radioSendMessage = "#ZPCH,033,!"
        workingData.radioSend = true
        workingData.status = "Sent GSE PCH Request..."
        workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
    }
    
    
   //---------------------------------------------------- SERVO TESTS -------------------------------------------------
    
    func checkAll() -> Bool {
        if(!switchTestUnlock.isOn){popup(TheS: "Unlock Two Factor Switch First");return false;}
        switchTestUnlock.isOn = false
        if(!padConfig.confirmed) {popupNotConfirmed(); return false}
        if(!padStatus.armed) {popup(TheS: "The Controller is not ARMED");return false;}
        return true
    }
    
    @IBAction func buttonTestSFUopen(_ sender: Any) {
        if !checkAll() {return}
        var thes: String = ""
        thes = textSFUopen.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 3000) {
            popup(TheS: "Out of Range. Must be 0-3000")
            textSFUopen.text = String(padConfig.SFUopen)
            return
        }
        thes = "#TSERV,SFU," + thes + ",!"
        workingData.radioSendMessage = thes
        finishSend()
    }
    
    @IBAction func buttonTestSFUclose(_ sender: Any) {
        if !checkAll() {return}
        var thes: String = ""
        thes = textSFUclose.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 3000) {
            popup(TheS: "Out of Range. Must be 0-3000")
            textSFUclose.text = String(padConfig.SFUclose)
            return
        }
        thes = "#TSERV,SFU," + thes + ",!"
        workingData.radioSendMessage = thes
        finishSend()
    }
    @IBAction func buttonSOXopen(_ sender: Any) {
        if !checkAll() {return}
        var thes: String = ""
        thes = textSOXopen.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 3000) {
            popup(TheS: "Out of Range. Must be 0-3000")
            textSOXopen.text = String(padConfig.SOXopen)
            return
        }
        thes = "#TSERV,SOX," + thes + ",!"
        workingData.radioSendMessage = thes
        finishSend()
    }
    @IBAction func buttonSOXclose(_ sender: Any) {
        if !checkAll() {return}
        var thes: String = ""
        thes = textSOXclose.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 3000) {
            popup(TheS: "Out of Range. Must be 0-3000")
            textSOXclose.text = String(padConfig.SOXclose)
            return
        }
        thes = "#TSERV,SOX," + thes + ",!"
        workingData.radioSendMessage = thes
        finishSend()
    }
    
    @IBAction func buttonTestSFLopen(_ sender: Any) {
        if !checkAll() {return}
        var thes: String = ""
        thes = textSFLopen.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 3000) {
            popup(TheS: "Out of Range. Must be 0-3000")
            textSFLopen.text = String(padConfig.SFLopen)
            return
        }
        thes = "#TSERV,SFL," + thes + ",!"
        workingData.radioSendMessage = thes
        finishSend()
    }
    
    @IBAction func buttonTestSFLclose(_ sender: Any) {
        if !checkAll() {return}
        var thes: String = ""
        thes = textSFLclose.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 3000) {
            popup(TheS: "Out of Range. Must be 0-3000")
            textSFLclose.text = String(padConfig.SFLclose)
            return
        }
        thes = "#TSERV,SFL," + thes + ",!"
        workingData.radioSendMessage = thes
        finishSend()
    }
    
    @IBAction func buttonTestSPGopen(_ sender: Any) {
        if !checkAll() {return}
        var thes: String = ""
        thes = textSPGopen.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 3000) {
            popup(TheS: "Out of Range. Must be 0-3000")
            textSPGopen.text = String(padConfig.SPGopen)
            return
        }
        thes = "#TSERV,SPG," + thes + ",!"
        workingData.radioSendMessage = thes
        finishSend()
    }
    
    @IBAction func buttonTestSPGclose(_ sender: Any) {
        if !checkAll() {return}
        var thes: String = ""
        thes = textSPGclose.text ?? "0"
        thes = thes.trimmingCharacters(in: .whitespacesAndNewlines)
        if(Int(thes) ?? 0 < 0 || Int(thes) ?? 0 > 3000) {
            popup(TheS: "Out of Range. Must be 0-3000")
            textSPGclose.text = String(padConfig.SPGclose)
            return
        }
        thes = "#TSERV,SPG," + thes + ",!"
        workingData.radioSendMessage = thes
        finishSend()
    }

    func finishSend() {
        workingData.radioSend = true
        workingData.status = "Sent Test Request..."
        workingData.statusTimeout = (Date() + TimeInterval(configData.statusTime))
    }
    

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
    
    
    
    
    
}



/*

  configuration.PTKenabled = atoi(tF[1]);
  configuration.PTKrange = atoi(tF[2]);
  configuration.PTKalarm = atoi(tF[3]);
  configuration.PLDenabled = atoi(tF[4]);
  configuration.PCHenabled = atoi(tF[5]);
  configuration.PCHrange = atoi(tF[6]);
  configuration.CPUtempAlarm = atoi(tF[7]);
  configuration.SFUopen = atoi(tF[8]);
  configuration.SFUclose = atoi(tF[9]);
  configuration.SFUopen = atoi(tF[10]);
  configuration.SFUclose = atoi(tF[11]);
  configuration.SFUopen = atoi(tF[12]);
  configuration.SFUclose = atoi(tF[13]);
  configuration.SFUopen = atoi(tF[14]);
  configuration.SFUclose = atoi(tF[15]);

*/
