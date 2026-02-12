#
#  Usage with crontab:
#  * * * * * /usr/bin/python /root/gatt-ble.py > /dev/null
#
#  For Project: https://github.com/AlexVakhnin/ESP32-C3-ADC-BLE-SENSOR
#
import sys
import subprocess
import asyncio
import logging
#import os
from bleak import BleakClient, BleakError
from datetime import datetime

timestamp = datetime.now().strftime('%Y-%m-%d')
log_filename = f'py-ups-{timestamp}.log'
full_log_path = "/home/orangepi/"+log_filename

logging.basicConfig(level=logging.INFO, filename=full_log_path,
                 format="%(asctime)s %(levelname)s %(message)s")

async def main():
    ble_address = "34:B7:DA:F8:4C:B2"
    MODEL_NBR_UUID = "d8182a40-7316-4cbf-9c6e-be507a76d775"
    data_to_send = bytearray(b"atv")
    data_shutdown = bytearray(b"shutdown")
    v_min = 11.88
    command =  ["shutdown", "now"]
    try:
        async with BleakClient(ble_address, adapter = "hci0") as client: #UB500 as hci0 work fine! 
            print(f"Connected: {client.is_connected}")
            await client.write_gatt_char(MODEL_NBR_UUID, data_to_send, response=True)
            print(f"Data '{data_to_send.decode()}' written to {MODEL_NBR_UUID}.")

            rdata = await client.read_gatt_char(MODEL_NBR_UUID)
            nstr = rdata.decode() # to utf8
            ind = nstr.find(".") #find position of "."
            nstr = nstr[0:ind+4] #trim line feed characters (12.123)
            print(f"Data term: '{nstr}'")
            n = float(nstr)
            #n = 11
            logging.info(f"Ubat={n:.2f}") #-->Logging
            print("Ubat =", n)
            if n < 9 or n > 15:
                print("Error Voltage..")
            elif n > v_min:
                print(f"Ubat > {v_min} OK!")
            else:
                print(f"Ubat < {v_min} Force shutdown..")
                await client.write_gatt_char(MODEL_NBR_UUID, data_shutdown, response=True)
                print(f"Data '{data_shutdown.decode()}' written to {MODEL_NBR_UUID}.")
                rdata = await client.read_gatt_char(MODEL_NBR_UUID)
                nstr = rdata.decode()[0:12] #-> to utf8 and cut (DO POWER OFF)
                logging.info(f"Ubat<{v_min} '{nstr}'") #-->Logging
                print(f"Data term: '{nstr}'")
                if nstr.find("POWER") != -1:
                    print("Start Shutdown for Linux..")
                    #subprocess.run(command, check=True) #Linux shutdown(bash) Need uncomment !

    except BleakError as e:
        logging.error(f"Bleak Error: {e}") #-->Logging
        print(f"Bleak Error: {e}")
    except asyncio.TimeoutError:
        logging.error("Timeout Error...") #-->Logging
        print("Timeout Error...")

asyncio.run(main())

###
