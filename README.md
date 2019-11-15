# iva
1. motion detection
2. line crossing
3. intrusion detection

# use

[options]  
-m : motion detection  
-l : motion detection and line crossing  
-i : motion detection and intrusion detection  

[parameters]  
--varThresh : threshold value of foreground and background (default = 20)  
--delta_w : delta-width for connect nearby rectangle (default = 1)  
--delta_h : delta-height for connect nearby rectangle (default = 2)  
--area_min : area min for object detected (default = 2)  
--learningRate : learning rate for update background (default = 30)  
--hostname : static ip of server (default = (null))  
--port : port connect to server (default = (null))  

usage:  
./iva (default)  
./iva [options] [[parameters] value] ...vv..  
