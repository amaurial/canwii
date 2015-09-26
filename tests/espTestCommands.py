#!/usr/bin/python3
from optparse import OptionParser
import serial, time


parser = OptionParser()
parser.add_option("-p", "--port", dest="port", default="/dev/ttyUSB0",
                  help="Serial port", metavar="PORT")
parser.add_option("-b", "--binary", action="store_true", dest="binary", default=False,
                  help="Print strings in binary", metavar="BINARY")
parser.add_option("-t", "--test", action="store_true", dest="test", default=False,
                  help="Do some tests", metavar="TEST")
parser.add_option("-l", "--listen", action="store_true", dest="listen", default=False,
                  help="Just listen the port", metavar="LISTEN")
parser.add_option("-s", "--server", action="store_true", dest="server", default=False,
                  help="Emulate the JMRI", metavar="SERVER")
#parser.add_option("-q", "--quiet",
#                  action="store_false", dest="verbose", default=True,
#                  help="don't print status messages to stdout")

(options, args) = parser.parse_args()

if options.port == None:
    print ("Inform the port.")
    options.usage
    exit()


#initialization and open the port

#possible timeout values:

#    1. None: wait forever, block call

#    2. 0: non-blocking mode, return immediately

#    3. x, x is bigger than 0, float allowed, timeout block call

#stringcoding="windows-1252"
stringcoding="ascii"

CMD_AT = "\x0a"
CMD_RST = "\x0b"
CMD_GMR = "\x0c"
CMD_GSLP = "\x0d"
CMD_IPR = "\x0e"
CMD_CWMODE = "\x0f"
CMD_CWJAP = "\x10"
CMD_CWLAP = "\x11"
CMD_CWQAP = "\x12"
CMD_CWSAP = "\x13"
CMD_CWLIF = "\x14"
CMD_CWDHCP = "\x15"
CMD_CIFSR = "\x16"
CMD_CIPSTAMAC = "\x17"
CMD_CIPAPMAC = "\x18"
CMD_CIPSTA = "\x19"
CMD_CIPAP = "\x1a"
CMD_CIPSTATUS = "\x1b"
CMD_CIPSTART = "\x1c"
CMD_CIPCLOSE = "\x1d"
CMD_CIPSEND = "\x1e"
CMD_CIPMUX = "\x1f"
CMD_CIPSERVER = "\x20"
CMD_CIPMODE = "\x21"
CMD_CIPSTO = "\x22"
CMD_CIUPDATE = "\x23"
CMD_CIPING = "\x24"
CMD_CIPAPPUP = "\x25"
CMD_ATE = "\x26"
CMD_MPINFO = "\x27"
CMD_MERG = "\x29"
CMD_MERG_AP= "\x2a"
CMD_VERSION= "\x2b"

CANWII_SOH = "\x01"
CANWII_EOH = "\x04"
CANWII_OK = "\x02"
CANWII_ERR = "\x03"

CANWII_TEST = "=?"

CMDS=dict(AT=CMD_AT,RST=CMD_RST,GMR=CMD_GMR,GSLP=CMD_GSLP,IPR=CMD_IPR,CWMODE=CMD_CWMODE,CWJAP=CMD_CWJAP,
          CWQAP=CMD_CWQAP,CWASAP=CMD_CWSAP,CWLIF=CMD_CWLIF,CWDHCP=CMD_CWDHCP,
        CIFSR=CMD_CIFSR,CIPSTAMAC=CMD_CIPSTAMAC,CIPAPMAC=CMD_CIPAPMAC,CIPSTA=CMD_CIPSTA,CIPAP=CMD_CIPAP,
        CIPSTATUS=CMD_CIPSTATUS,CIPSTART=CMD_CIPSTART,CIPCLOSE=CMD_CIPCLOSE,CIPSEND=CMD_CIPSEND,
        CIPMUX=CMD_CIPMUX,CIPSERVER=CMD_CIPSERVER,CIPMODE=CMD_CIPMODE,CIPSTO=CMD_CIPSTO,CIPUPDATE=CMD_CIUPDATE,
        CIPING=CMD_CIPING,CIPAPPUP=CMD_CIPAPPUP,ATE=CMD_ATE,MPINFO=CMD_MPINFO,MERG=CMD_MERG,MERG_AP=CMD_MERG_AP)

def sendCommand(command,timewait=0):

    if ser.isOpen():
        try:
            #write data
            if options.binary:
                print("write data binary:" , command, end='\n')
            print("write data ascii:" , command.encode(stringcoding), end='\n')
            ser.write(command.encode(stringcoding))
            #ser.flush()
            time.sleep(timewait)  #give the serial port sometime to receive the data
            numOfLines = 0
            #print("read1");
            response=ser.read(255)

            #response = ser.readline()
            cmdResponse = response
            while True:
                if ser.isOpen():
                    numOfLines = numOfLines + 1
                    if len(cmdResponse)>0:
                        #print("read2");
                        print("read data ascii: " , cmdResponse.decode(stringcoding,'ignore'),end='\n')
                        if options.binary:
                            print("read data binary: " , cmdResponse,end='\n')
                        
                        if checkReceived(cmdResponse.decode(stringcoding,'ignore'))>=0:
                            #print("read3");
                            return cmdResponse.decode(stringcoding,'ignore')
                            break
                    if ((numOfLines >= 30) and (len(response) == 0)):
                        return cmdResponse.decode(stringcoding,'ignore')
                        break
                    #response = ser.readline()
                    #print("read4");
                    response=ser.read(255)
                    cmdResponse = cmdResponse + response
                    if len(response)>0:
                        print("read data ascii: " , cmdResponse.decode(stringcoding,'ignore'),end='\n')
                        if options.binary:
                            print("read data binary: " , cmdResponse,end='\n')
                else:
                    print ("Reopening Serial Port.")
                    reopenSerial()

        except Exception as e1:
            print('sendc command error communicating...: ',e1,end='\n')
            ser.close()
    else:

        print("send command - cannot open serial port.")

def resetEsp():
    if ser.isOpen():
        try:
            #write data
            print("write data:" , "RESET", end='\n')
            command=CANWII_SOH + CMD_RST + CANWII_EOH
            ser.write(command.encode(stringcoding))
            time.sleep(5)  #give the serial port sometime to receive the data


        except Exception as e1:
            print('error communicating...: ',e1,end='\n')
            ser.close()
    else:

        print("cannot send reset ")

def checkReceived(data):
    #check the end of string sent by the esp
    #we are looking for OK\n

    temp=" "
    temp=data
    print("check command received\n")
    #print ("data:",len(temp),end='\n')
    if len(temp)<1:
        return -1

    if (temp.find("OK\n")>=0):
        print("Found OK\n")
        return 0

    if (temp.find("NO_CHANGE\n")>=0):
        print("Found NO Change\n")
        return 0
    if (temp.find("ERROR\n")>=0):
        print("Found Error\n")
        return 1

    if (temp.find("\n>")>=0):
        print("Found >\n")
        return 2
    if (temp.find("FAIL\n")>=0):
        print("Found FAIL\n")
        return 3
    if (temp.find((CANWII_OK))>=0):
        print("Found OK\n")
        return 0
    if (temp.find((CANWII_ERR))>=0):
        print("Found ERR\n")
        return 1
    if (temp.find("\n")>=0):
        print("Found \\n\n")
        return 0
    #print("Not found OK\n")
    return -1

def setupWifiClient():
    #set mode
    resp=sendCommand(CANWII_SOH + CMD_CWMODE + "=" + "1" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set the mode\n")
        return False

    #connect to ap
    resp=sendCommand(CANWII_SOH + CMD_CWJAP + "=" + "\"dlink\",\"\"" + CANWII_EOH,10)
    if checkReceived(resp)!=0:
        print ("Failed to connect\n")
        return False

    #check connection to ap
    resp=sendCommand(CANWII_SOH + CMD_CWJAP + "?" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to connect\n")
        return False

    #check IP
    resp=sendCommand(CANWII_SOH + CMD_CIFSR + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to get ip\n")
        sendCommand(CANWII_SOH + CMD_CWQAP + CANWII_EOH)
        return False

    if resp.find("STAIP")>0:
        print("Wifi connected Success")
    return True

def setupWifiServer():
    #set mode
    resp=sendCommand(CANWII_SOH + CMD_CWMODE + "=" + "3" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set the mode\n")
        return False

    #set multiple connections
    # resp=sendCommand(CANWII_SOH + CMD_RST + CANWII_EOH)
    # if checkReceived(resp)!=0:
    #     print ("Failed to set multiple connections\n")
    #     return False
    # ser.close()
    # time.sleep(5)
    # ser.open()

    #set multiple connections
    resp=sendCommand(CANWII_SOH + CMD_CIPMUX + "=1" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set multiple connections\n")
        return False

    #connect to ap
    resp=sendCommand(CANWII_SOH + CMD_CWJAP + "=" + "\"dlink\",\"\"" + CANWII_EOH,10)
    if checkReceived(resp)!=0:
        print ("Failed to connect\n")
        return False

    #check connection to ap
    resp=sendCommand(CANWII_SOH + CMD_CWJAP + "?" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to connect\n")
        return False

    #check IP
    resp=sendCommand(CANWII_SOH + CMD_CIFSR + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to get ip\n")
        sendCommand(CANWII_SOH + CMD_CWQAP + CANWII_EOH)
        return False

    if resp.find("STAIP")>0:
        print("Wifi connected to client")

#set Reset
    resp=sendCommand(CANWII_SOH + CMD_CIPSTATUS + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed get status\n")
        return False


    #set the esp in server mode
    resp=sendCommand(CANWII_SOH + CMD_CIPSERVER + "=1,30" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set server mode\n")
        sendCommand(CANWII_SOH + CMD_CWQAP + CANWII_EOH)
        return False


     #set timeout
    resp=sendCommand(CANWII_SOH + CMD_CIPSTO + "=30" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set timeout\n")
        sendCommand(CANWII_SOH + CMD_CWQAP + CANWII_EOH)
        return False


    return True

def setupApServer():

    #set multiple connections
    print ("Setting multiple connections\n")
    resp=sendCommand(CANWII_SOH + CMD_CIPMUX + "=1" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set multiple connections\n")
        return False

    #check IP
    print ("Getting the IP\n")
    resp=sendCommand(CANWII_SOH + CMD_CIFSR + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to get ip\n")
        sendCommand(CANWII_SOH + CMD_CWQAP + CANWII_EOH)
        return False

    if resp.find("STAIP")>0:
        print("Wifi connected to client")

    print ("Printing STATUS\n")
    resp=sendCommand(CANWII_SOH + CMD_CIPSTATUS + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed get status\n")
        return False


    #set the esp in server mode
    print ("Putting SERVER MODE\n")
    resp=sendCommand(CANWII_SOH + CMD_CIPSERVER + "=1,30" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set server mode\n")
        sendCommand(CANWII_SOH + CMD_CWQAP + CANWII_EOH)
        return False


     #set timeout
    print ("Setting the timeout\n")
    resp=sendCommand(CANWII_SOH + CMD_CIPSTO + "=60" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set timeout\n")
        sendCommand(CANWII_SOH + CMD_CWQAP + CANWII_EOH)
        return False


    return True

def setupApWifiServer():
    #set mode
    resp=sendCommand(CANWII_SOH + CMD_CWMODE + "=" + "3" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set the mode\n")
        return False

    # #set multiple connections
    # resp=sendCommand(CANWII_SOH + CMD_CIPMUX + "=1" + CANWII_EOH)
    # if checkReceived(resp)!=0:
    #     print ("Failed to set multiple connections\n")
    #     return False

    #check connection to ap
    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "=2,0" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set dhcp\n")
        return False

    #connect to ap
    resp=sendCommand(CANWII_SOH + CMD_CWSAP + "=" + "\"esp\",\"123\",1,0" + CANWII_EOH,10)
    if checkReceived(resp)!=0:
        print ("Failed to set ap\n")
        return False

    return True

#*<SOH>
        #     <CMD_MERG><=>
        #         <MERG_CMDID>
        #         <CWMODE>
        #         <CWDHCP_P1>
        #         <CWDHCP_P2>
        #         <ssid>,<passwd>,
        #         <CWSAP_p3> channel
        #         <CWSAP_p4> wpa type
        #         <CIPMUX>
        #         <CIPSERVER_p1>
        #         <CIPSERVER_p2>
        # <EOH>

def setupMergServer():
     #         <MERG_CMDID>30
        #         <CWMODE>3
        #         <CWDHCP_P1>2
        #         <CWDHCP_P2>0
        #         <ssid>,<passwd>,esp,123
        #         <CWSAP_p3> channel 1
        #         <CWSAP_p4> wpa type 0
        #         <CIPMUX> 1
        #         <CIPSERVER_p1> 1
        #         <CIPSERVER_p2> 30

    #resp=sendCommand(CANWII_SOH + CMD_MERG + "=" + "320esp,123,1011" + "\x30" + CANWII_EOH,2)

#    /*<SOH>
#            <CMD_MERG_CONFIG_APT><=>
#                <ssid>,<passwd>,
#                <CWSAP_p3> channel
#                <CWSAP_p4> wpa type
#                <PORT>
#        <EOH>
#        */
    resp=sendCommand(CANWII_SOH + CMD_MERG_AP + "=" + "merg,123,500" + CANWII_EOH,2)

    if checkReceived(resp)!=0:
        print ("Failed to set merg mode\n")
        return False
    
    #resp=sendCommand(CANWII_SOH + CMD_CIFSR + CANWII_EOH)
    #if checkReceived(resp)!=0:
    #    print ("Failed to get IP\n")
    #    return False

    return True

def connectToServer(host,port):
    #open connection
    try:
        print("host:",host," port:",port,end='\n')

        resp=sendCommand(CANWII_SOH + CMD_CIPMUX + "=1" + CANWII_EOH)
        if checkReceived(resp)!=0:
            print ("Failed to set the mode mux\n")
            return False

        resp=sendCommand(CANWII_SOH + CMD_CIPSTART + "=1," + "\"TCP\",\"" + host + "\"," + port +  CANWII_EOH)
        if checkReceived(resp)!=0:
            print ("Failed to connect to the server\n")
            return False
    except Exception as e:
        print("Failed to connect to server.", e)
        return False
    return True

def sendSomeData():
    print("Send test data\n")
    data="hello\n"
    resp=sendCommand(CANWII_SOH + CMD_CIPSEND + "=1," + str(len(data)) + CANWII_EOH)
    print(resp,end='\n')

    for i in range(1,10):
        resp=sendCommand(data,0.8)
        resp=sendCommand(CANWII_SOH + CMD_CIPSEND + "=1," + str(len(data)) + CANWII_EOH)
        if checkReceived(resp)!=2:
            print ("Failed to send data ",i,end='\n')

    sendCommand("quit")

def readServerData():
    if ser.isOpen():
        try:
            while True:
                response = ser.readline()
                bindata=response
                if len(response)>0:
                    print("data ascii: " + response.decode(stringcoding,'ignore'),end='\n')
                    if options.binary:
                        print("data binary: " + bindata,end='\n')
                    if (response.decode(stringcoding,'ignore').find("quit")>=0):
                        return True
        except Exception as e:
                print("error open serial port: " + e)
                return False
        return False

def reopenSerial():
    for i in range (1,10):
        try:
            ser.open()
            time.sleep(2)
            return
        except Exception as e:
            print("error open serial port: " + e)
    exit()

def testComands():
    for k,v in CMDS.items():
        print("Testing ",k,end='\n')
        sendCommand(CANWII_SOH + v + CANWII_TEST+ CANWII_EOH)

def testDHCP():

    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "=0,0" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set dhcp station mode\n")
        return False

    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "?" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to check dhcp mode\n")
        return False


    time.sleep(2);
    print("DHCP STATION MODE DISABLE",end='\n')
    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "=0,1" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to unset dhcp station mode\n")
        return False

    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "?" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to check dhcp mode\n")
        return False


    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "=1,0" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set dhcp station mode\n")
        return False

    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "?" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to check dhcp mode\n")
        return False


    time.sleep(2);

    print("DHCP STATION MODE DISABLE",end='\n')
    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "=1,1" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to unset dhcp softap mode\n")
        return False

    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "?" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to check dhcp mode\n")
        return False

    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "=2,0" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to set both mode\n")
        return False

    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "?" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to check dhcp mode\n")
        return False

    time.sleep(2);
    print("DHCP STATION MODE DISABLE",end='\n')
    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "=2,1" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to unset dhcp both\n")
        return False
    resp=sendCommand(CANWII_SOH + CMD_CWDHCP + "?" + CANWII_EOH)
    if checkReceived(resp)!=0:
        print ("Failed to check dhcp mode\n")
        return False

ser = serial.Serial()
ser.port = options.port
ser.baudrate = 115200
ser.bytesize = serial.EIGHTBITS  #number of bits per bytes
ser.parity = serial.PARITY_NONE  #set parity check: no parity
ser.stopbits = serial.STOPBITS_ONE  #number of stop bits
#ser.timeout = None          #block read
ser.timeout = 1  #non-block read
ser.xonxoff = False  #disable software flow control
ser.rtscts = False  #disable hardware (RTS/CTS) flow control
ser.dsrdtr = False  #disable hardware (DSR/DTR) flow control
ser.writeTimeout = 2  #timeout for write
reopenSerial()

if ser.isOpen():
    try:

        if options.listen:
            while True:
                if ser.isOpen():
                    response=ser.read(255)
                    if len(response) >0 :
                        print("read data ascii: " , response.decode(stringcoding,'ignore'),end='\n')
                        if options.binary:
                            print("read data binary: " , response,end='\n')
                else:
                    print ("Reopening Serial Port.\n")
                    reopenSerial()
        if options.server:
            print("Acting as server\n")
            while True:
                if ser.isOpen():
                    response=ser.read(255)
                    if len(response) >0 :
                        print("read data ascii: " , response.decode(stringcoding,'ignore'),end='\n')
                        if options.binary:
                            print("read data binary: " , response,end='\n')

                        pkconn= CANWII_SOH + CANWII_ERR + "220" + CANWII_EOH
                        #print("pkconn: " , pkconn,end='\n')
                        if pkconn in response.decode(stringcoding,'ignore') :
                            print("client connected. sending version")
                            resp=sendCommand(CANWII_SOH + CMD_CIPSEND +"=0" + "VN2.0\n*5\n" + CANWII_EOH)
                        if "NEngine" in response.decode(stringcoding,'ignore'):
                            resp=sendCommand(CANWII_SOH + CMD_CIPSEND +"=0" + "*5\n*+\n" + CANWII_EOH)

                else:
                    print ("Reopening Serial Port.")
                    reopenSerial()
        if options.test:
            testDHCP();
        else:

            print ("Sending AT")
            resp=sendCommand(CANWII_SOH + CMD_AT + CANWII_EOH)
            print ("AT received")
            if checkReceived(resp)!=0:
                print ("No OK found\n")

            print ("Sending Version")
            resp=sendCommand(CANWII_SOH + CMD_VERSION + CANWII_EOH)
            print ("VERSION received")
            if checkReceived(resp)!=0:
                print ("No OK found\n")
           # resp=sendCommand(CANWII_SOH + CMD_CWLAP + CANWII_EOH)
           # if checkReceived(resp)!=0:
           #     print ("No OK found\n")

            #if setupWifiClient():
            #     #sendCommand("quit")
            #     if connectToServer("192.168.1.119","9999"):
            #         sendSomeData()

            #if setupWifiServer():
            #   readServerData()
            #if setupApServer():
            if setupMergServer():
                #ser.close()
                #time.sleep(5);
                #reopenSerial()
                print ("Set AP OK\n")

                if readServerData()==False:
                    sendCommand(CANWII_SOH + CMD_CIPCLOSE + "=5" + CANWII_EOH)
                    exit()


            #print("Putting ESP in AP mode\n")
            #if setupApWifiServer():
                # print("Reseting\n")
                # resetEsp()
                # print("Closing serial comm and slepping\n")
                # ser.close()
                # time.sleep(10)
                # print("Try to open serial comm\n")
                # ser.open()
                # print("Try to open serial comm\n")
                # if (ser.isOpen()):
                #     print("Waiting for connections\n")
                # else:
                #     print("Failed to reopen serial comm\n")

             #   if setupApServer():
             #      readServerData()

            #quit connection


    except Exception as e:
        print("general error : ", e)
        ser.close()

    finally:
        ser.close()
        exit()

