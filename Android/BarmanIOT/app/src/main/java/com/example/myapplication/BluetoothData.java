package com.example.myapplication;

import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class BluetoothData {

    private static com.example.myapplication.BluetoothData obj;
    private BluetoothSocket btSocket;
    private InputStream in;
    private OutputStream out;
    private static final String TAG = "Datos BT";

    public static com.example.myapplication.BluetoothData getInstance() {
        if(obj == null) {
            obj = new com.example.myapplication.BluetoothData();
        }
        return obj;
    }

    public void setSocket(BluetoothSocket socket) {
        this.btSocket = socket;
    }

    public BluetoothSocket getSocket() {
        return btSocket;
    }

    public void setIn(InputStream in) {
        this.in = in;
    }

    public InputStream getIn() {
        return this.in;
    }

    public void setOut(OutputStream out) {
        this.out = out;
    }

    public OutputStream getOut() {
        return this.out;
    }

    public void cerrarConexiones() {
        try {
            btSocket.close();
            in.close();
            out.close();
        } catch (IOException e) {
            Log.e(TAG, "Error al cerrar sockets o streams");
            Log.e(TAG, e.getMessage());
        }
    }
}
