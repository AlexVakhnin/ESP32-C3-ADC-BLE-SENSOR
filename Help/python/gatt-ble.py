
import asyncio
from bleak import BleakClient, BleakError

async def main():
    ble_address = "34:B7:DA:F8:4C:B2"
    MODEL_NBR_UUID = "d8182a40-7316-4cbf-9c6e-be507a76d775"
    data_to_send = bytearray(b"atv")
    try:    
        async with BleakClient(ble_address, adapter = "hci0") as client:
            print(f"Connected: {client.is_connected}")
            await client.write_gatt_char(MODEL_NBR_UUID, data_to_send, response=True)
            print(f"Data '{data_to_send}' written to {MODEL_NBR_UUID}.")
            
            rdata = await client.read_gatt_char(MODEL_NBR_UUID)
            print(f"Data term: {rdata.decode()}")
    except BleakError as e:
        print(f"Bleak Error: {e}")
    except asyncio.TimeoutError:
        print("Timeout Error...")
        

asyncio.run(main())

###