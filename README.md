# iva_ttcn
1. motion ditection
2. line crossing
3. intrusion detection

# use

./iva :	default is motion detection, line crossing and intrusion detection

[options] 
-m : motion detection 
-l : motion detection and line crossing
-i : motion detection and intrusion detection

[parameters]
--varThresh : threshold value of foreground and background (default = 20)
--delta_w : delta-width for connect nearby rectangle (default = 2)
--delta_h : delta-height for connect nearby rectangle (default = 3)
--area_min : area min for object deteted (default = 2)
--learningRate : learning rate for update background (default = 15)
--hostname : static ip of server (default = (null))
--port : port connect to server (default = (null))
--camera : in (indoor) or out (outdoor)

usage:
./iva --parameter value --parameter value ...vv..
./iva [options] --parameter value --parameter value ...vv..
