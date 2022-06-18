#!/usr/bin/env python3
import UNIXstreams4 as unx
import sys
import time

def main(SocketPath):
    print("TCP Forward Control V1.0")
    CON = unx.clientCon(SocketPath)
    print("Connection Success")

    while True:
        COMMAND = input("> ").lower()
        if (COMMAND == '?') | (COMMAND == 'help'):
            print("""
            help - this menu
            ? - this menu
            exit - disconnect
            setip - sets the target ip
            setport - sets the target port
            listcon - lists connections by index
            kill - kills the server
            """)
        elif COMMAND == 'exit':
            CON.senddat(b'\x00')
            print(CON.getstdat())
            break

        elif COMMAND == 'kill':
            CON.senddat(b'\x03')
            print(CON.getstdat())
            break

        elif COMMAND == 'setip':
            CON.senddat(b'\x01')
            print(CON.getstdat())
            CON.sendstdat(input('IP: ') + '\x00')
            print(CON.getstdat())

        elif COMMAND == 'setport':
            CON.senddat(b'\x02')
            print(CON.getstdat())
            CON.sendstdat(input('PORT: ') + '\x00')
            print(CON.getstdat())

        elif COMMAND == 'listcon':
            CON.senddat(b'\x04')
            Result = CON.getstdat()
            time.sleep(0.1)
            CON.senddat(b'OK')
            
            print(Result,"Connections Currently Open")
            
            for i in range(int(Result)):
                print(i,"Address:",CON.getstdat())
                time.sleep(0.1)
                CON.senddat(b'OK')

                

    CON.close()


if __name__ == '__main__':
    main(sys.argv[1])
