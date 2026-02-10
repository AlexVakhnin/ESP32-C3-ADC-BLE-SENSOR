
import asyncio
from bleak import BleakClient, BleakError

async def main():
    ble_address = "34:B7:DA:F8:4C:B2"
    MODEL_NBR_UUID = "d8182a40-7316-4cbf-9c6e-be507a76d775"
    data_to_send = bytearray(b"atv")
    data_shutdown = bytearray(b"shutdown")
    try:
        async with BleakClient(ble_address, adapter = "hci0") as client:
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
            print("Ubat =", n)
            if n < 9 or n > 15:
                print("Error Voltage..")
            elif n > 12:
                print("Ubat > 12V OK!")
            else:
                print("Ubat < 12V Force shutdown..")
                await client.write_gatt_char(MODEL_NBR_UUID, data_shutdown, response=True)
                print(f"Data '{data_shutdown.decode()}' written to {MODEL_NBR_UUID}.")
                rdata = await client.read_gatt_char(MODEL_NBR_UUID)
                nstr = rdata.decode()[0:12] #-> to utf8 and cut (DO POWER OFF)
                print(f"Data term: '{nstr}'")

    except BleakError as e:
        print(f"Bleak Error: {e}")
    except asyncio.TimeoutError:
        print("Timeout Error...")
        

asyncio.run(main())

###
