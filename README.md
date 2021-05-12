EXAM2
=====
1.首先用 $git clone https://github.com/hun0905/EXAM2.git    將所需的檔案抓下來<br> 
2.調整mbed_app.json的WiFi SSID,PASSWORD 以及 wifi_mqtt/mqtt_client.py的host<br> 
3.然後 $ cd EXAM2/src/model_deploy<br> 
4.輸入$ sudo mbed compile --source . --source ~/ee2405/mbed-os/ -m B_L4S5I_IOT01A -t GCC_ARM -f 進行編譯<br> 
5.在另外一個terminal $ cd EXAM2/src/model_deploy  後用 $sudo python3 wifi_mqtt/mqtt_client.py 執行   python 檔<br> 
6.輸入$ sudo screen /dev/<devicename> (<devicename> ex:ttyACM0)<br> 
7.在成功連線後 輸入/gesture_UI/run 會進入角度的模式進行10組動作會記錄gesture_ID然後停下<br> 
8.輸入/acc/run 按著USER BUTTON 當角度 達一定程度 會 記下來 。<br> 
9.接著python  傳指令並畫圖。<br> 

  
 
