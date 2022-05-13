import sys
import time
import telepot
import os
import RPi.GPIO as GPIO
import datetime
import re #regular expression: specifies a set of strings that matches it
import requests # to request messages from Telegram
from time import sleep
from telepot.loop import MessageLoop
from subprocess import call
import picamera
from picamera import PiCamera
import time
from rpi_lcd import LCD
import serial
import time
import subprocess
from subprocess import call # Functions being imported 

##===============================Control Lock=====================================
GPIO.setwarnings(False) #help keep the code active and prevent warning when restarted
  
GPIO.setmode(GPIO.BCM) #to use Raspberry Pi board pin numbers
GPIO.setup(18, GPIO.OUT) #set up GPIO pin 18 output channel

def on(pin): #turn relay on, outputs max voltage 3.3v
   GPIO.output(18,1)
   return

def off(pin): #turn relay off, outputs 0v
  GPIO.output(18,0)
  return

##==============================Telegram Bot========================================
base_url = "https://api.telegram.org/bot2106671482:AAF_2uJW5Jqc7TzdyHDfo_aX7buo5md-i5k/sendMessage" #token

def replace_line(file_name, line_num, text): #replace a line in a file
  lines = open(file_name, 'r').readlines()
  lines[line_num] = text
  out = open(file_name, 'w')
  out.writelines(lines)
  out.close()

def handle(msg): 
  global chat_id
  chat_id = msg['chat']['id']
  command = msg['text'] #text from Telegram
  
  date = datetime.datetime.now() #return the current local date and time
  dti = date.isoformat() #iosformat returns the date value of a datetime
  print( "%s" %dti)
  dt = str(dti)
  print("\nMessage received from " + str(chat_id) + ': %s' % command)

  if command == 'On':
     print("Unlocked!");
     bot.sendMessage(chat_id, text = "Unlocked")
     bot.sendMessage(chat_id, on(18)) 
  elif command == 'Off':
     print("Locked!");
     bot.sendMessage(chat_id, text = "Locked")
     bot.sendMessage(chat_id, off(18))
  elif command == 'Photo':
     send_photo() #capture a photo
     bot.sendMessage(chat_id, text = "Photo has been sent " + dt)  
  elif command == 'Video':
     send_video() #capture a video
     bot.sendMessage(chat_id, text = "Video has been sent " + dt)   
  else:
      print("Command is not valid!");
      bot.sendMessage(chat_id, text = "Invalid input. Please enter again!\nOn - Unlock Door\nOff - Lock Door\nPhoto - Sending Photo\nVideo - Sending Video" )

def send_photo():
    print("Capturing photo…")
    camera = picamera.PiCamera()
    camera.capture('./photo.jpg') #capture photo with Raspberry Pi Camera and store it using photo.jpg
    camera.close() #must close the camera before taking another photo/video
    print("Sending photo to " + str(chat_id))
    bot.sendPhoto(chat_id, photo = open('./photo.jpg', 'rb')) #send the photo to Telegram
    
def send_video():
    chat_id = 2093204891
    camera = picamera.PiCamera() #to open camera
    camera.resolution = (640, 480)
    camera.framerate = 15
    
    filename = "./video_" + (time.strftime("%y%b%d_%H%M%S"))
    camera.start_recording(filename + ".h264")
    sleep(8) #video has a length of 5 secs
    camera.stop_recording()
    camera.close() #must close the camera before taking another photo/video
    command = "MP4Box -add " + filename + '.h264' + " " + filename + '.mp4'
    print(command)
    call([command], shell=True)
    bot.sendVideo(chat_id, video=open(filename + '.mp4', 'rb'), supports_streaming=True)   

bot = telepot.Bot('2106671482:AAF_2uJW5Jqc7TzdyHDfo_aX7buo5md-i5k') #Telegram bot token to access HTTP API
bot.message_loop(handle) #to handle messages from Telepot
print("Telegram bot is ready")

#==========================Pin pad and LCD======================================
# This callback registers the key that was pressed if no other key is currently pressed
def keypadCallback(channel):
    global keypadPressed
    if keypadPressed == -1:
        keypadPressed = channel     

# Sets all lines to a specific state. This is a helper for detecting when the user releases a button
def setAllLines(state):
    GPIO.output(L1, state)
    GPIO.output(L2, state)
    GPIO.output(L3, state)
    GPIO.output(L4, state)
    
def readInput():
    global input
    pressed = False
    
    GPIO.output(L1, GPIO.HIGH)
    if (GPIO.input(C1) == 1): #(If 1 is chosen)
        lcd.clear()
        print("Pin Pad Chosen")
        lcd.text("Pin Pad chosen", 1)
        time.sleep(2.0)
        passwordAttempt()
        pressed = True
    GPIO.output(L1, GPIO.LOW)
    
    GPIO.output(L1, GPIO.HIGH)
    if (GPIO.input(C2) == 1): #(If 2 is chosen)
        lcd.clear()
        print("Voice Chosen")
        lcd.text("Voice Chosen", 1)
        time.sleep(2.0)
        lcd.text("Speak Phrase", 1)
        voiceAttempt()
        pressed = True
    GPIO.output(L1, GPIO.LOW)
    
    GPIO.output(L1, GPIO.HIGH)
    if (GPIO.input(C3) == 1): #(If 3 is chosen)
        lcd.clear()
        print("Face Chosen")
        lcd.text("Face Chosen", 1)
        time.sleep(2.0)
        faceAttempt()
        pressed = True
    GPIO.output(L1, GPIO.LOW)
    
    if pressed:
        input = ""
    return pressed

def checkSpecialKeys():
    chat_id = 2093204891
    global input
    global maxAttempt
    global Exit
    pressed = False

    GPIO.output(L3, GPIO.HIGH)
    if (GPIO.input(C4) == 1): #(If C is pressed)
        print("Input reset")
        lcd.text(message, 1)
        time.sleep(2.0)
        pressed = True    
    GPIO.output(L3, GPIO.LOW)

    GPIO.output(L2, GPIO.HIGH)
    if (GPIO.input(C4) == 1): #If B is pressed
        Exit = 1
    GPIO.output(L2, GPIO.LOW)
    
    GPIO.output(L1, GPIO.HIGH)
    if (not pressed and GPIO.input(C4) == 1):  #(A is enter)
        if input == secretCode:
            lcd.text("Password Correct",1)
            time.sleep(2.0)
            maxAttempt = 5
            bot.sendMessage(chat_id, 'Correct password! Access allowed')#send message automatically to Telegram    
        else:
            converted_output = str(2-maxAttempt)
            print("Incorrect password!")
            lcd.text("Incorrect Password!",1)
            lcd.text("Attempts left:",2)
            lcd.text (converted_output,3)
            maxAttempt += 1
            input = ""
            bot.sendMessage(chat_id, 'Incorrect password!') #send message automatically to Telegram    
        pressed = True
    GPIO.output(L3, GPIO.LOW)

def readLine(line, characters):
    global input
    global message
    message = ""
    
    # We have to send a pulse on each line to detect button presses
    GPIO.output(line, GPIO.HIGH)
    if(GPIO.input(C1) == 1):
        input = input + characters[0]
    if(GPIO.input(C2) == 1):
        input = input + characters[1]
    if(GPIO.input(C3) == 1):
        input = input + characters[2]
    if(GPIO.input(C4) == 1):
        input = input + characters[3]
    GPIO.output(line, GPIO.LOW)
#---------------------------------------------------
def success():
    global maxAttempt
    global Exit
    global passwordOut
    passwordOut = ""
    maxAttempt = 0
    print("Success")
    lcd.text("Successful! Door will unlock for 5 seconds",1)
    on(18)
    sleep(5) #unlock for 5 secs
    off(18)
    lcd.text("Verification method?",1)
    lcd.text("1. Pin Pad",2)
    lcd.text("2. Voice",3)
    lcd.text("3. Face Recognition",4)
        
def failure():
    global maxAttempt
    global Exit
    global passwordOut
    passwordOut = ""
    maxAttempt = 0
    chat_id = 2093204891

    print("Failure")
    lcd.text("Access Denied" ,1)
    lcd.text("Notification Sent", 2)
    lcd.text("To Owner", 3)
    
    date = datetime.datetime.now() #return the current local date and time
    dti = date.isoformat() #iosformat returns the date value of a datetime
    print("%s" %dti)
    dt = str(dti)
    
    #send messages and video to Telegram
    bot.sendMessage(chat_id, 'Below is the recording video: ' + dt)
    send_video()
    
    #go back to the main menu
    lcd.text("Verification method?",1)
    lcd.text("1. Pin Pad",2)
    lcd.text("2. Voice",3)
    lcd.text("3. Face Recognition",4)
    
#--------------------------------------------
def passwordAttempt():
    chat_id = 2093204891
    
    global lcd
    lcd = LCD()
    
    global L1
    global L2
    global L3
    global L4
    
    global C1
    global C2
    global C3
    global C4
    
    global keypadPressed
    global secretCode
    global input

    # GPIO pins are connected to the keypad
    L1 = 25
    L2 = 8
    L3 = 7
    L4 = 1

    C1 = 12
    C2 = 16
    C3 = 20
    C4 = 21
    # Initialize the GPIO pins
    keypadPressed = -1
    secretCode = "1234"
    input = ""

    # Setup GPIO
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)

    global maxAttempt
    global passwordOut
    global Exit
    passwordOut = ""
    Exit = 0
    maxAttempt
    
    #lcd at initialization
    converted_output = str(3-maxAttempt)
    lcd.text("Attempts Left:",2)
    lcd.text(converted_output, 3)
     
    while True:
        # If a button was previously pressed,
        # check, whether the user has released it yet
        if keypadPressed != -1:
            setAllLines(GPIO.HIGH)
            if GPIO.input(keypadPressed) == 0:
                keypadPressed = -1
            else:
                time.sleep(0.1)
        elif(Exit == 1):
            lcd.clear() 
            print("Exited pin pad authentication!")
            lcd.text("Exited verification",1)
            time.sleep(2.0)
            lcd.text("Verification method?",1)
            lcd.text("1. Pin Pad",2)
            lcd.text("2. Voice Verification",3)
            lcd.text("3. Face recognition",4)
            break
        elif (maxAttempt < 3):
            if (not checkSpecialKeys()):
                readLine(L1, ["1","2","3","A"])
                readLine(L2, ["4","5","6","B"])
                readLine(L3, ["7","8","9","C"])
                readLine(L4, ["*","0","#","D"])
                time.sleep(0.1)
            else:
                time.sleep(0.1)
        # if code is correct        
        elif (maxAttempt == 5):
            print("Password Accepted")
            lcd.text("Password Correct",1)     
            success()
            break
        #if code is input wrong 3 times
        else:
            bot.sendMessage(chat_id, 'Password is entered WRONG 3 times! Access denied')
            failure()
            break 

##========================================Voice Verification========================================
def voiceAttempt():
    chat_id = 2093204891
    
    if __name__ == '__main__':
        ser = serial.Serial('/dev/ttyACM0',115200, timeout=1)
        ser.flush()
        start_time = time.time()
        while True:
            if (time.time() - start_time < 6): #during 5 seconds
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8').rstrip()
                    if (line == 'UTK Keyword'):
                        print(line)
                        bot.sendMessage(chat_id, 'Voice has been verified! Access allowed')#send message to Telegram
                        success()
                        break
            else: #after 5 seconds
                print("Time out")
                bot.sendMessage(chat_id, 'Voice verification failed! Access denied')
                failure()
                break
            
##=================================Facial Recognition======================================== 
def faceAttempt():
    chat_id = 2093204891

    # Sets bash command to be called
    # -----------------------------------------------------
    process = "./facial_Bash_Commands.sh;"
    log = "./log_Check.sh;"

    # Executes bash bommand
    face_log = call(process, shell=True)
    # Clears first two lines of log and removes unknown users 
    edit_Log = call(log, shell=True)

    # Short delay for file to update 
    time.sleep(0.3)
    # -----------------------------------------------------

    # Checking log file fore recognized user
    # -----------------------------------------------------
    # rt - recognitiion time 
    face_Recognized = 'rt'

    # Opens face recognition log file 
    detection_Log = open("/home/pi/facial_detection", "r")

    # Reads face recognition log file contents
    read_Log = detection_Log.read()

    # Checks  face recognition log file for recognized user 
    if face_Recognized in read_Log:
        bot.sendMessage(chat_id, 'Face has been recognized! Access allowed')#send message to Telegram
        success()
    else:
        bot.sendMessage(chat_id, 'Face recognition failed! Access denied')
        failure()

    # Clears face recognition log file contents
    detection_Log = open("/home/pi/facial_detection", "w")

    # Closes face recognition log file 
    detection_Log.close()
          
#===================================Pin Pad ======================================= 
while(1):
    lcd = LCD()

    lcd.text("Verification method?",1)
    lcd.text("1. Pin Pad",2)
    lcd.text("2. Voice",3)
    lcd.text("3. Face Recognition",4)

    # GPIO pins are connected to the keypad
    L1 = 25 #line 1
    L2 = 8
    L3 = 7
    L4 = 1
    
    C1 = 12 #column 1
    C2 = 16
    C3 = 20
    C4 = 21

    keypadPressed = -1
    secretCode = "1234"
    input = ""
    
    # Setup GPIO
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(L1, GPIO.OUT)
    GPIO.setup(L2, GPIO.OUT)
    GPIO.setup(L3, GPIO.OUT)
    GPIO.setup(L4, GPIO.OUT)

    # Use the internal pull-down resistors
    GPIO.setup(C1, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(C2, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(C3, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(C4, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.add_event_detect(C1, GPIO.RISING, callback=keypadCallback)
    GPIO.add_event_detect(C2, GPIO.RISING, callback=keypadCallback)
    GPIO.add_event_detect(C3, GPIO.RISING, callback=keypadCallback)
    GPIO.add_event_detect(C4, GPIO.RISING, callback=keypadCallback)
    
    try:
        global maxAttempt
        global passwordOut
        global Exit
        passwordOut = ""
        maxAttempt = 0
        Exit = 0
        while True:
            # If a button was previously pressed,
            # check, whether the user has released it yet
            if keypadPressed != -1:
                setAllLines(GPIO.HIGH)
                if GPIO.input(keypadPressed) == 0:
                    keypadPressed = -1
                else:
                    time.sleep(0.1)
            else: 
                if (not readInput()):
                    readLine(L1, ["1","2","3","A"])
                    readLine(L2, ["4","5","6","B"])
                    readLine(L3, ["7","8","9","C"])
                    readLine(L4, ["*","0","#","D"])
                    time.sleep(0.1)             
                else:
                    time.sleep(0.1)                  
    except KeyboardInterrupt:
        print("\nApplication stopped!")
            
