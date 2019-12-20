# iva
1. Motion detection
2. Line crossing
3. Intrusion detection

# use

[options]  
-m : motion detection  
-r : motion detection region  
-l : line crossing  
-i : intrusion detection  
-a : all - MD, MDR, LC, ID

[parameters]  
--varThresh : 	threshold value of foreground and background (default = 20)  
--delta_w : 	delta-width for connect nearby rectangle (default = 1)  
--delta_h : 	delta-height for connect nearby rectangle (default = 2)  
--area_min : 	area min for object detected (default = 2)  
--learningRate :learning rate for update background (default = 30)  
--hostname : 	static ip of server (default = (null))  
--port : 		port connect to server (default = (null))  

usage:  
./iva (default of file-cfg)  
./iva [options] [[parameters] value] ...vv..  


# cfg file: iva.json

1. Motion detection  
"resolution": 	width x height of Web GUI  
"enable_md": 	true/false - alarm for motion detection with raw-frame (1280x720, ..)  
"enable_mdr": 	true/false  - alarm for motion detection with region in frame (123x456, 456x321 ..)  
"p_x": 			start point x for region  
"p_y": 			start point y for region    
"width": 		width of region  
"height": 		height of region  

2. Line crossing  
"resolution": 	width x height of Web GUI  
"enable_lc": 	true/false - alarm for line crossing  
"sensitivity":
"direction": 	1/2/3 - 0: A<->B, 1: A->B, 2: B->A  
"startX": 		start point x for line  
"startY": 		start point y for line  
"endX": 		end point x for line  
"endY": 		end point y for line  

3. Intrusion detection  
"resolution": 	width x height of Web GUI  
"enable_its": 	true/false - alarm for motion detection  
"sensitivity":   
"direction": 1,  
"p_x1", "p_y1", "p_x2", "p_y2", "p_x3", "p_y3", "p_x4", "p_y4":	point of region  
