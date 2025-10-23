import time
from pyrf24 import RF24, RF24_PA_MAX, RF24_1MBPS, RF24_250KBPS
import RPi.GPIO as GPIO
from datetime import datetime
buf = ['a'*32]*6945
data = ['a'*32]*6945
i = 0
def millis():
    return round(time.time() * 1000)

if __name__ == "__main__":
    # The orginal command is below
    # radio = RF24(22, 17) #CE, CSN
    radio = RF24(25,0)
    pipes = [0xF0F0F0F0E1, 0xF0F0F0F0D2]

    radio.begin()

    radio.setPALevel(RF24_PA_MAX)
    radio.setDataRate(RF24_250KBPS)
    radio.setChannel(124)

    radio.setAutoAck(True)
    radio.enableDynamicPayloads()
    radio.setRetries(50, 15)


    radio.openWritingPipe(pipes[1])
    radio.openReadingPipe(1, pipes[0])

    buffer = buf

    radio.stopListening()

    # Trigger Ultrasonic Sensor
   # if not radio.write(buffer):
   #     print("No acknowledgment of transmission!")
   #     pass
    
    radio.startListening()

     # print(waiting_time)
    while radio.available():
        if i < 6945:
            test = radio.read(32)
            #print(f"{datetime.now().strftime('%m/%d/%Y %H:%M')}, {str(buffer, 'utf-8')}")
            i = i+1
            print(test)

    #encoding = 'utf-8'
    #test.decode(encoding)
    #print(test)
    started_waiting_at = 0

    
    
    while not radio.available():
        waiting_time = millis() - started_waiting_at
        print("Not Available")
        exit()
        #if millis() - started_waiting_at > 1000:
        #    print(f"{datetime.now().strftime('%m/%d/%Y %H:%M')}, No response received, timeout!")
        #    print(data)
        #    exit()

   
print("End")
