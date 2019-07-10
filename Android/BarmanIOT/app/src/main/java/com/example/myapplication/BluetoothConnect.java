package com.example.myapplication;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.UUID;

public class BluetoothConnect extends Thread {

    private BluetoothSocket socket;
    private static final String TAG = "BT Connect";
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private String mac;
    private BluetoothDevice device;
    private BluetoothAdapter bt;
    private InputStream in;
    private OutputStream out;

    public BluetoothConnect(String mac, BluetoothDevice device, BluetoothAdapter bt) {
        this.mac = mac;
        this.device = device;
        this.bt = bt;
    }

    @Override
    public void run() {
        try {
            device = bt.getRemoteDevice(mac);
            try {
                socket = device.createInsecureRfcommSocketToServiceRecord(BTMODULEUUID);
                Method m = device.getClass().getMethod("createRfcommSocket", new Class[] {int.class});
                socket = (BluetoothSocket) m.invoke(device, 1);
                in = socket.getInputStream();
                out = socket.getOutputStream();

                BluetoothData btDatos = BluetoothData.getInstance();
                btDatos.setSocket(socket);
                btDatos.setIn(in);
                btDatos.setOut(out);
            } catch (Exception e) {
                Log.e(TAG, String.valueOf(e.getMessage()));
            }
            socket.connect();
        } catch (IOException e) {
            Log.e(TAG, String.valueOf(e.getMessage()));
        }
    }

    public BluetoothSocket getSocket() {
        return socket;
    }
}
